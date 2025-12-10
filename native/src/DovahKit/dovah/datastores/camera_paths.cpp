#include "./camera_paths.h"
#include <cassert>
#include <memory>
#include "helpers/vectors/move_item_to_index.h"
#include "helpers/vectors/move_item_within.h"
#include "./camera_paths/node.h"
#include "./camera_paths/passkeys/initial_build.h"
#include "./camera_paths/passkeys/post_build_edit.h"
#include "./camera_paths/warning.h"
#include "./camera_paths/warnings/cyclical_parent_relationships.h"
#include "./camera_paths/warnings/cyclical_sibling_relationships.h"
#include "./camera_paths/warnings/form_has_multiple_next_siblings.h"
#include "./camera_paths/warnings/inconsistent_parentage.h"
#include "./camera_paths/warnings/previous_sibling_is_not_as_expected.h"
#include "./camera_paths/warnings/sibling_is_an_ancestor.h"
#include "./camera_paths/warnings/siblings_have_mismatched_parents.h"
#include "../files/file_load_order.h"
#include "../forms/CameraPath.h"
#include "../form_stub.h"

namespace {
   using node = dovah::datastores::camera_paths::node;

   namespace warnings {
      using namespace dovah::datastores::impl::camera_paths::warnings;
   }

   // As of this writing, DovahKit's backend is designed to sever references to 
   // deleted forms whenever possible. This means that when we delete an idle, 
   // the active-file ANAM will be cleared out, making the idle loose. (We don't 
   // clear non-active-file ANAMs; see comments in IdleAnimation for details.)
   static constexpr const bool backend_severs_references_to_deleted_forms = true;
}

namespace dovah::datastores {
   camera_paths::camera_paths() {
   }
   camera_paths::~camera_paths() {
      this->_clear();
   }

   void camera_paths::_clear() {
      this->root.children.clear();

      for (auto& pair : this->nodes_by_stub)
         delete pair.second;
      this->nodes_by_stub.clear();

      for (auto* warning : this->warnings)
         delete warning;
      this->warnings.clear();
   }

   void camera_paths::build(file_load_order& lo) {
      this->reset();
      //
      // Pre-create all camera path nodes.
      //
      lo.for_each_form_of_type(relevant_form_type, [this](form_stub* stub) {
         auto  node_ptr = std::make_unique<node>(*this, *stub);
         auto& node     = *node_ptr;
         this->nodes_by_stub[stub] = &node;
         node_ptr.release();
         return false;
      });
      //
      // Build the parent/child hierarchy for the camera path nodes.
      //
      for (auto& pair : this->nodes_by_stub) {
         auto& node   = *pair.second;
         auto  loaded = node.stub.load().ptr_cast<loaded_form_data>();
         if (loaded) {
            this->_place_form(node, *loaded);
         }
      }
      for (auto& pair : this->nodes_by_stub)
         this->_post_placement_parentage_validation(*pair.second);
   }
   void camera_paths::reset() {
      this->_clear();
   }

   #pragma region Handlers for events occurring outside the datastore
      void camera_paths::on_before_form_fully_deleted(form_stub& stub) {
         assert(stub.get_owning_load_order().is_defined_in_active_file(stub) && "Only call this function for stubs that are being wholly deleted, not just flagged!");
         if (stub.form_type != relevant_form_type)
            return;
         auto it = this->nodes_by_stub.find(&stub);
         if (it == this->nodes_by_stub.end())
            return;
         auto* subject = it->second;
         if (!subject)
            return;
         if (node_parent* parent = subject->parent) {
            if (auto& cb = this->callbacks.node_deleted.before)
               cb(*subject);
            //
            size_t i = parent->index_of_child(*subject);
            assert(i != node::index_of_none);
            node* former_next_sibling = nullptr;
            if (i + 1 < parent->children.size())
               former_next_sibling = parent->children[i + 1];
            //
            parent->children.erase(parent->children.begin() + i);
            this->nodes_by_stub.erase(it);
            delete subject;
            //
            if (former_next_sibling)
               this->_update_form_data(*former_next_sibling);
            //
            if (auto& cb = this->callbacks.node_deleted.after)
               cb(stub.formID);

         } else {
            this->nodes_by_stub.erase(it);
            delete subject;
         }
      }

      void camera_paths::on_form_created(form_stub& stub) {
         if (stub.form_type != relevant_form_type)
            return;
         auto loaded = stub.load().ptr_cast<loaded_form_data>();
         if (!loaded)
            return;

         node*& subject = this->nodes_by_stub[&stub];
         if (!subject) {
            try {
               subject = new node(*this, stub);
            } catch (...) {
               this->nodes_by_stub.erase(&stub);
               throw;
            }
         }
         form_stub*   parent_stub = loaded->parent.get_form_stub();
         form_stub*   prev_stub   = loaded->previous_sibling.get_form_stub();
         node_parent* parent      = parent_stub ? this->node_by_stub(*parent_stub) : nullptr;
         node*        prev_node   = prev_stub ? this->node_by_stub(*prev_stub) : nullptr;
         if (!parent) {
            parent = &this->root;
         }
         if (prev_node) {
            if (prev_node->parent == parent) {
               this->move_camera_path(*subject, *parent, prev_node);
               return;
            }
            prev_node = nullptr;
            parent    = &this->root;
            if (!parent->children.empty())
               prev_node = parent->children.back();
         }
         this->move_camera_path(*subject, *parent, prev_node);
      }
   #pragma endregion

   /*static*/ bool camera_paths::_form_has_cyclical_parentage(std::set<form_stub*>& seen, loaded_form_data& loaded) {
      auto* current = loaded.parent.get_form_stub();
      do {
         if (!current)
            break;
         if (current->form_type != relevant_form_type)
            break;
         if (seen.contains(current))
            return true;
         seen.insert(current);

         auto loaded = current->load().ptr_cast<loaded_form_data>();
         if (!loaded)
            break;
         current = loaded->parent.get_form_stub();
      } while (true);
      return false;
   }
   /*static*/ camera_paths::sibling_problem camera_paths::_form_has_bad_siblinghood(const std::set<form_stub*>& seen_ancestors, loaded_form_data& loaded) {
      auto* parent  = loaded.parent.get_form_stub();
      if (parent && parent->form_type != relevant_form_type)
         parent = nullptr;

      auto* current = loaded.previous_sibling.get_form_stub();
      if (!current || current->form_type != relevant_form_type)
         return {};

      std::set<form_stub*> seen_siblings;
      do {
         if (seen_ancestors.contains(current))
            return sibling_problem::ancestor;
         if (seen_siblings.contains(current))
            return sibling_problem::cyclical;
         seen_siblings.insert(current);

         auto loaded = current->load().ptr_cast<loaded_form_data>();
         if (!loaded)
            break;

         auto* current_parent = loaded->parent.get_form_stub();
         if (current_parent && current_parent->form_type != relevant_form_type)
            current_parent = nullptr;
         if (current_parent != parent)
            return sibling_problem::mismatched;

         //
         // Move on to next.
         //
         current = loaded->previous_sibling.get_form_stub();
         if (!current || current->form_type != relevant_form_type)
            break;
      } while (true);
      return sibling_problem::none;
   }
   void camera_paths::_place_form(node& subject, loaded_form_data& loaded) {
      form_stub* parent_stub   = loaded.parent.get_form_stub();
      form_stub* previous_stub = loaded.previous_sibling.get_form_stub();
      if (parent_stub && parent_stub->form_type != relevant_form_type) {
         parent_stub = nullptr;
      }

      std::set<form_stub*> seen_ancestors;
      if (this->_form_has_cyclical_parentage(seen_ancestors, loaded)) {
         {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::cyclical_parent_relationships(subject);
         }
         parent_stub   = nullptr;
         previous_stub = nullptr;
      } else {
         auto problem = this->_form_has_bad_siblinghood(seen_ancestors, loaded);
         if (problem != sibling_problem::none) {
            if (problem == sibling_problem::cyclical) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::cyclical_sibling_relationships(subject);
            } else if (problem == sibling_problem::ancestor) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::sibling_is_an_ancestor(subject);
            } else if (problem == sibling_problem::mismatched) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::siblings_have_mismatched_parents(subject);
            }
            parent_stub   = nullptr;
            previous_stub = nullptr;
         }
      }

      node_parent* parent_obj    = nullptr;
      node*        previous_node = nullptr;
      if (parent_stub) {
         auto it = this->nodes_by_stub.find(parent_stub);
         if (it != this->nodes_by_stub.end())
            parent_obj = it->second;
      }
      if (!parent_obj) {
         parent_obj = &this->root;
      }
      if (previous_stub) {
         auto it = this->nodes_by_stub.find(previous_stub);
         if (it != this->nodes_by_stub.end())
            previous_node = it->second;
      }
      subject._get_sort_state({}) = {
         .parent   = parent_obj,
         .previous = previous_node,
      };
      assert(parent_obj != nullptr);
      parent_obj->_insert_sorted_child({}, subject);
   }
   void camera_paths::_post_placement_parentage_validation(node& subject) {
      node*        previous = nullptr;
      node_parent* parent   = nullptr;
      {
         auto& ss = subject._get_sort_state({});
         previous = ss.previous;
         parent   = ss.parent;
         if (ss.multiple_next_siblings) {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::form_has_multiple_next_siblings(subject);
         }
         ss = {};
      }
      if (!parent) {
         assert(subject.parent == nullptr);
      }

      if (!parent)
         return;
      auto& siblings = parent->children;
      
      size_t i = parent->index_of_child(subject);
      if (i == node::index_of_none) {
         auto& dst = this->warnings.emplace_back();
         dst = new warnings::inconsistent_parentage(subject);
      } else if (previous) {
         node* actual = nullptr;
         if (i > 0)
            actual = siblings[i - 1];
         if (i == 0 || actual != previous) {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::previous_sibling_is_not_as_expected(subject, previous, actual);
         }
      }
   }

   const node* camera_paths::node_by_stub(const form_stub& stub) const noexcept {
      if (stub.form_type != relevant_form_type)
         return nullptr;
      auto it = this->nodes_by_stub.find((form_stub*)&stub); // std::unordered_map::find chokes on const-pointer keys if the key type is non-const
      if (it == this->nodes_by_stub.end())
         return nullptr;
      return it->second;
   }
   node* camera_paths::node_by_stub(const form_stub& stub) noexcept {
      return const_cast<node*>(std::as_const(*this).node_by_stub(stub));
   }

   bool camera_paths::is_node_movement_legal(const node& subject, const node_parent& dst_parent, const node* dst_previous) const {
      // Moving an idle inside itself or one of its descendants is illegal.
      if (&subject == &dst_parent)
         return false;
      if (auto* dst_node = dynamic_cast<const node*>(&dst_parent))
         if (subject.is_ancestor_of(*dst_node))
            return false;

      // Prevent inconsistent moves (e.g. "move into A, after B" if B is not a child of A)
      if (dst_previous)
         if (dst_parent.index_of_child(*dst_previous) == node_parent::index_of_none)
            return false;

      return true;
   }

   #pragma region Helpers for editing operations
      void camera_paths::_update_form_data(node& subject) {
         if (auto& cb = this->callbacks.form_data_modified.before)
            cb(subject.stub);
         subject._update_form_hierarchy_data({});
         if (auto& cb = this->callbacks.form_data_modified.after)
            cb(subject.stub);
      }
   #pragma endregion

   void camera_paths::_delete_single_node(node& subject) {
      assert(!!this->handlers.delete_camera_path);

      const bool defined_in_master = subject.is_defined_in_non_active_file();

      auto& subject_form_stub = subject.stub;
      auto  subject_form_id = subject.stub.formID;

      if constexpr (backend_severs_references_to_deleted_forms) {
         if (defined_in_master) {
            if (subject.parent) {
               this->move_camera_path(subject, this->root, nullptr);
               assert(subject.parent == nullptr);
            }
         } else {
            if (auto& cb = this->callbacks.node_deleted.before)
               cb(subject);
            if (node_parent* parent = subject.parent) {
               node* former_next_sibling = nullptr;
               size_t i = parent->index_of_child(subject);
               assert(i != node::index_of_none);
               if (i + 1 < parent->children.size()) {
                  former_next_sibling = parent->children[i + 1];
               }
               parent->children.erase(parent->children.begin() + i);
               subject.parent = nullptr;
               if (former_next_sibling) {
                  this->_update_form_data(*former_next_sibling);
               }
            }
            this->nodes_by_stub.erase(&subject_form_stub);
            this->handlers.delete_camera_path(subject_form_stub);
            delete &subject;
            if (auto& cb = this->callbacks.node_deleted.after)
               cb(subject_form_id);
         }
      } else {
         if (defined_in_master) {
            //
            // The IDLE form was originally defined outside of the active file 
            // and so cannot be wholly deleted. The most we can do is move it 
            // to LOOSE and flag it and its descendants as "deleted."
            //
            this->move_camera_path(subject, this->root, nullptr);
            this->handlers.delete_camera_path(subject_form_stub); // flag the form as "deleted"
            return;
         }

         auto& form = subject.stub;
         if (auto& cb = this->callbacks.node_deleted.before)
            cb(subject);

         node* former_next_sibling = nullptr;
         if (node_parent* former_parent = subject.parent) {
            size_t i = former_parent->index_of_child(subject);
            assert(i != node::index_of_none);
            if (i + 1 < former_parent->children.size())
               former_next_sibling = former_parent->children[i + 1];
            former_parent->children.erase(former_parent->children.begin() + i);
            subject.parent = nullptr;
         }
         if (former_next_sibling)
            this->_update_form_data(*former_next_sibling);
         this->_update_form_data(subject);

         this->nodes_by_stub.erase(&subject_form_stub);
         this->handlers.delete_camera_path(subject_form_stub);

         if (auto& cb = this->callbacks.node_deleted.after)
            cb(subject_form_id);

         delete &subject;
      }
   }

   void camera_paths::delete_camera_path(node& subject) {
      assert(!!this->handlers.delete_camera_path);

      auto children = subject.children;
      for (node* child : children)
         this->delete_camera_path(*child);

      this->_delete_single_node(subject);
   }

   void camera_paths::move_camera_path(node& subject, node_parent& dst_parent, node* dst_previous) {
      // Skip redundant operations.
      if (subject.parent == &dst_parent) {
         if (dst_previous) {
            auto i = dst_parent.index_of_child(subject);
            assert(i != node::index_of_none);
            if (i > 0 && dst_parent.children[i - 1] == dst_previous)
               return;
         } else {
            if (!dst_parent.children.empty() && dst_parent.children[0] == &subject)
               return;
         }
      }

      // Skip impossible operations.
      if (!this->is_node_movement_legal(subject, dst_parent, dst_previous))
         return;

      size_t insert_at = 0;
      if (dst_previous) {
         insert_at = dst_parent.index_of_child(*dst_previous) + 1;
      }

      if (auto& cb = this->callbacks.node_placed.before)
         cb(subject, dst_parent, insert_at);

      node*        former_next_sibling = nullptr;
      node_parent* moved_from = subject.parent;
      if (moved_from) {
         size_t from = moved_from->index_of_child(subject);
         auto&  list = moved_from->children;
         if (from + 1 < list.size())
            former_next_sibling = list[from + 1];
         if (moved_from == &dst_parent) {
            if (insert_at >= from)
               --insert_at;
         }
      }
      {
         subject.parent = &dst_parent;
         auto& dst_list = dst_parent.children;
         dst_list.insert(dst_list.begin() + insert_at, &subject);
         if (insert_at + 1 < dst_list.size()) {
            node* new_next_sibling = dst_list[insert_at + 1];
            assert(new_next_sibling != nullptr);
            this->_update_form_data(*new_next_sibling);
         }
      }
      if (former_next_sibling)
         this->_update_form_data(*former_next_sibling);

      if (auto& cb = this->callbacks.node_placed.after)
         cb(subject);
   }
   void camera_paths::move_camera_path_within_parent(node& subject, int by) {
      node_parent* parent = subject.parent;
      assert(parent != nullptr);
      const size_t from = parent->index_of_child(subject);
      const size_t size = parent->children.size();
      assert(from != node::index_of_none);

      node* previous_prior = nullptr;
      node* previous_after = nullptr;
      if (from)
         previous_prior = parent->children[from - 1];

      if (by < 0) {
         if (from == 0)
            return;
         if (from < -by)
            by = -from;
      } else if (by == 0) {
         return;
      } else {
         if (from == size - 1)
            return;
         if (from + by >= size)
            by = size - from - 1;
      }
      size_t to = from + by;
      if (auto& cb = this->callbacks.node_placed.before)
         cb(subject, *parent, to);

      cobb::vectors::move_item_within(parent->children, from, by);

      if (previous_prior)
         this->_update_form_data(*previous_prior);
      this->_update_form_data(subject);
      {
         auto i = parent->index_of_child(subject);
         if (i < size - 1)
            previous_after = parent->children[i];
      }
      if (previous_after)
         this->_update_form_data(*previous_after);

      if (auto& cb = this->callbacks.node_placed.after)
         cb(subject);
   }
}