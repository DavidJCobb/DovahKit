#include "./story_manager.h"
#include <cassert>
#include <memory>
#include "helpers/vectors/move_item_to_index.h"
#include "helpers/vectors/move_item_within.h"
#include "../data/hardcoded_form_ids.h"
#include "../form_stubs/helpers/winning_record_loads_after.h"
#include "./story_manager/branch_node.h"
#include "./story_manager/leaf_node.h"
#include "./story_manager/node.h"
#include "./story_manager/warning.h"
#include "./story_manager/passkeys/initial_build.h"
#include "./story_manager/passkeys/post_build_edit.h"
#include "../files/file_load_order.h"
#include "../forms/StoryManagerBranchNode.h"
#include "../forms/StoryManagerEventNode.h"
#include "../forms/StoryManagerQuestNode.h"
#include "../form_stub.h"

namespace {
   using node        = dovah::datastores::story_manager::node;
   using branch_node = dovah::datastores::story_manager::branch_node;
   using leaf_node   = dovah::datastores::story_manager::leaf_node;

   // As of this writing, DovahKit's backend is designed to sever references to 
   // deleted forms whenever possible. This means that when we delete an idle, 
   // the PNAM and SNAM subrecords will be cleared.
   static constexpr const bool backend_severs_references_to_deleted_forms = true;
}

namespace dovah::datastores {
   story_manager::story_manager() {
   }
   story_manager::~story_manager() {
      this->_clear();
   }

   void story_manager::_clear() {
      this->root = nullptr;

      for (auto& pair : this->nodes_by_stub)
         delete pair.second;
      this->nodes_by_stub.clear();

      for (auto* warning : this->warnings)
         delete warning;
      this->warnings.clear();
   }

   /*static*/ bool story_manager::_is_form_type_relevant(form_type ft) {
      switch (ft) {
         case form_type::story_branch_node:
         case form_type::story_event_node:
         case form_type::story_quest_node:
            return true;
      }
      return false;
   }
   /*static*/ bool story_manager::_is_branch_node_type(form_type ft) {
      switch (ft) {
         case form_type::story_branch_node:
         case form_type::story_event_node:
            return true;
      }
      return false;
   }

   void story_manager::build(file_load_order& lo) {
      this->reset();
      //
      // Pre-create all nodes.
      //
      for (auto ft : std::array{
         form_type::story_branch_node,
         form_type::story_event_node,
         form_type::story_quest_node,
      }) {
         bool is_branch = _is_branch_node_type(ft);
         lo.for_each_form_of_type(ft, [this, is_branch](form_stub* stub) {
            std::unique_ptr<node> node_ptr;
            if (is_branch) {
               node_ptr = std::make_unique<branch_node>(*this, *stub);
            } else {
               node_ptr = std::make_unique<leaf_node>(*this, *stub);
            }
            auto& node     = *node_ptr;
            this->nodes_by_stub[stub] = &node;
            node_ptr.release();
            if (stub->formID == hardcoded_form_ids::Root) {
               assert(is_branch);
               this->root = static_cast<branch_node*>(&node);
            }
            return false;
         });
      }
      //
      // Build the parent/child hierarchy for the camera path nodes.
      //
      {
         std::vector<node*> sorted;
         for (auto& pair : this->nodes_by_stub)
            sorted.push_back(pair.second);
         std::sort(
            sorted.begin(),
            sorted.end(),
            [&lo](node* x, node* y) {
               return form_stub_helpers::winning_record_loads_after(x->stub, y->stub);
            }
         );
         for(auto* current_node : sorted) {
            auto& bs     = current_node->_get_build_state({});
            auto  loaded = current_node->stub.load();
            if (auto* casted = dynamic_cast<loaded_forms::mixins::StoryManagerNode*>(&*loaded)) {
               form_stub* parent_stub   = casted->parent.get_form_stub();
               form_stub* previous_stub = casted->previous_sibling.get_form_stub();

               bool previous_is_intentionally_null = !casted->has_previous_sibling;
               bs.previous_intentionally_null = previous_is_intentionally_null;
               bs.previous_ended_up_null      = false;

               branch_node* parent_node   = nullptr;
               node*        previous_node = nullptr;
               if (parent_stub && _is_branch_node_type(parent_stub->form_type)) {
                  auto* pn = this->node_by_stub(*parent_stub);
                  if (pn) {
                     parent_node = dynamic_cast<branch_node*>(pn);
                     assert(parent_node != nullptr && "If the parent form's type is 'branch,' how is the parent node's type anything else?!");
                  }
               }
               if (previous_stub) {
                  if (_is_form_type_relevant(previous_stub->form_type))
                     previous_node = this->node_by_stub(*previous_stub);
                  else
                     previous_stub = nullptr;
               }
               bs.previous_ended_up_null = previous_node == nullptr;

               bool unordered = false;
               if (loaded->stub.form_type == form_type::story_event_node) {
                  if (this->root) {
                     parent_node = this->root;
                     unordered   = true;
                  }
               }
               if (unordered) {
                  //
                  // TODO: Only append an event-node to the root if the event-node has a 
                  // valid event ID. Otherwise, we'll need to put it somewhere else.
                  //
                  assert(this->root != nullptr);
                  this->root->_append_during_load({}, *current_node);
                  //
                  // The CK would here also modify the previous-sibling relationships 
                  // to match the insertion order. We're... not going to do that. No 
                  // need.
                  //
               } else {
                  if (previous_node) {
                     parent_node->_insert_during_load({}, *current_node, *previous_node);
                  } else if (previous_is_intentionally_null) {
                     parent_node->_insert_during_load({}, *current_node, 0);
                  } else {
                     parent_node->_append_during_load({}, *current_node);
                  }
               }
            }
         }
      }
   }
   void story_manager::normalize_for_editing() {
      //
      // If a story manager node specifies a previous sibling that isn't intentionally 
      // null, but is a form of the wrong type OR a non-existent form ID, then the node 
      // is appended to its parent. Otherwise, the node is prepended.
      // 
      // Suppose we load a node whose winning override is in the active file. When we 
      // load the node, we'll detect that it has no (valid) previous sibling...
      // 
      //  - ...and if the previous-sibling is an existent form of the wrong type, then 
      //    when we re-save the node, nothing changes.
      // 
      //  - ...but if the previous-sibling is a non-existent form, then when we save 
      //    the node, we'll save its previous-sibling as intentionally none. This will 
      //    result in a hierarchy inconsistent from what we `build()` above.
      // 
      // So if your goal is to use the datastore for an editor and not merely to view 
      // the story manager hierarchy, then call this function after `build()`. It'll 
      // find active-file story manager nodes with invalid previous siblings, and fix 
      // them.
      //
      for (auto& pair : this->nodes_by_stub) {
         auto& node = *pair.second;
         auto& bs   = node._get_build_state({});
         if (bs.previous_intentionally_null || !bs.previous_ended_up_null)
            continue;
         if (!node.parent)
            continue;
         _update_form_data(node);
         bs = {};
      }
   }
   void story_manager::reset() {
      this->_clear();
   }

   #pragma region Handlers for events occurring outside the datastore
      void story_manager::on_before_form_fully_deleted(form_stub& stub) {
         assert(stub.get_owning_load_order().is_defined_in_active_file(stub) && "Only call this function for stubs that are being wholly deleted, not just flagged!");
         if (!_is_form_type_relevant(stub.form_type))
            return;
         auto it = this->nodes_by_stub.find(&stub);
         if (it == this->nodes_by_stub.end())
            return;
         auto* subject = it->second;
         if (!subject)
            return;
         if (branch_node* parent = subject->parent) {
            if (auto& cb = this->callbacks.node_deleted.before)
               cb(*subject);
            //
            size_t i = parent->index_of_child(*subject);
            assert(i != branch_node::index_of_none);
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

      void story_manager::on_form_created(form_stub& stub) {
         if (!_is_form_type_relevant(stub.form_type))
            return;
         auto loaded = stub.load();
         if (!loaded)
            return;
         auto* loaded_mixin = dynamic_cast<loaded_forms::mixins::StoryManagerNode*>(&*loaded);
         assert(loaded_mixin != nullptr);

         node*& subject = this->nodes_by_stub[&stub];
         if (!subject) {
            try {
               if (_is_branch_node_type(stub.form_type)) {
                  subject = new branch_node(*this, stub);
               } else {
                  subject = new leaf_node(*this, stub);
               }
            } catch (...) {
               this->nodes_by_stub.erase(&stub);
               throw;
            }
         }

         form_stub*   parent_stub   = loaded_mixin->parent.get_form_stub();
         form_stub*   previous_stub = loaded_mixin->previous_sibling.get_form_stub();
         bool previous_is_intentionally_null = previous_stub == nullptr;
         branch_node* parent_node   = nullptr;
         node*        previous_node = previous_stub ? this->node_by_stub(*previous_stub) : nullptr;
         if (stub.form_type == form_type::story_event_node) {
            parent_node = this->root;
         } else {
            if (parent_stub && _is_branch_node_type(parent_stub->form_type)) {
               parent_node = (branch_node*)this->node_by_stub(*parent_stub);
            }
            if (!parent_node) {
               parent_node = this->root;
            }
         }
         
         if (!previous_node || previous_node->parent != parent_node) {
            previous_node = nullptr;
            if (!parent_node->children.empty())
               previous_node = parent_node->children.back();
         }
         this->move_node(*subject, *parent_node, previous_node);
      }
   #pragma endregion


   const node* story_manager::node_by_stub(const form_stub& stub) const noexcept {
      if (!_is_form_type_relevant(stub.form_type))
         return nullptr;
      auto it = this->nodes_by_stub.find((form_stub*)&stub); // std::unordered_map::find chokes on const-pointer keys if the key type is non-const
      if (it == this->nodes_by_stub.end())
         return nullptr;
      return it->second;
   }
   node* story_manager::node_by_stub(const form_stub& stub) noexcept {
      return const_cast<node*>(std::as_const(*this).node_by_stub(stub));
   }

   bool story_manager::is_node_movement_legal(const node& subject, const branch_node& dst_parent, const node* dst_previous) const {
      // Moving an idle inside itself or one of its descendants is illegal.
      if (&subject == &dst_parent)
         return false;
      if (auto* as_branch = dynamic_cast<const branch_node*>(&subject))
         if (as_branch->is_ancestor_of(dst_parent))
            return false;

      // Prevent inconsistent moves (e.g. "move into A, after B" if B is not a child of A)
      if (dst_previous)
         if (dst_parent.index_of_child(*dst_previous) == branch_node::index_of_none)
            return false;

      return true;
   }

   #pragma region Helpers for editing operations
      void story_manager::_update_form_data(node& subject) {
         if (auto& cb = this->callbacks.form_data_modified.before)
            cb(subject.stub);
         subject._update_form_hierarchy_data({});
         if (auto& cb = this->callbacks.form_data_modified.after)
            cb(subject.stub);
      }
   #pragma endregion

   void story_manager::_delete_single_node(node& subject) {
      assert(!!this->handlers.delete_form);

      auto& subject_form_stub = subject.stub;
      auto  subject_form_id   = subject.stub.formID;

      const bool defined_in_master = !subject_form_stub.get_owning_load_order().is_defined_in_active_file(subject_form_stub);

      if (defined_in_master) {
         //
         // The form was originally defined outside of the active file 
         // and so cannot be wholly deleted. The most we can do is flag 
         // it and its descendants as "deleted."
         //
         if constexpr (backend_severs_references_to_deleted_forms) {
            //
            // The form will be severed from all parent nodes. We'll 
            // move it to the root node for now, but we should investigate 
            // either: making it so we can more precisely control what 
            // uses are severed by deletion; or have a way to represent 
            // orphans.
            //
            if (auto* dst = this->root) {
               if (dst->children.empty()) {
                  this->move_node(subject, *this->root, nullptr);
               } else {
                  this->move_node(subject, *this->root, dst->children.back());
               }
            }
         }
         this->handlers.delete_form(subject_form_stub); // flag the form as "deleted"
         return;
      }

      if (auto& cb = this->callbacks.node_deleted.before)
         cb(subject);
      //
      if (branch_node* parent = subject.parent) {
         node* former_next_sibling = nullptr;
         size_t i = parent->index_of_child(subject);
         assert(i != branch_node::index_of_none);
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
      this->handlers.delete_form(subject_form_stub);
      delete& subject;
      //
      if (auto& cb = this->callbacks.node_deleted.after)
         cb(subject_form_id);
   }

   void story_manager::delete_node(node& subject) {
      assert(!!this->handlers.delete_form);

      if (auto* casted = dynamic_cast<branch_node*>(&subject)) {
         auto children = casted->children; // intentional copy
         for (node* child : children)
            this->delete_node(*child);
      }

      this->_delete_single_node(subject);
   }

   void story_manager::move_node(node& subject, branch_node& dst_parent, node* dst_previous) {
      // Skip redundant operations.
      if (subject.parent == &dst_parent) {
         if (dst_previous) {
            auto i = dst_parent.index_of_child(subject);
            assert(i != branch_node::index_of_none);
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
      branch_node* moved_from = subject.parent;
      if (moved_from) {
         size_t from = moved_from->index_of_child(subject);
         auto&  list = moved_from->children;
         if (from + 1 < list.size())
            former_next_sibling = list[from + 1];
         list.erase(list.begin() + from);
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
   void story_manager::move_node_within_parent(node& subject, int by) {
      branch_node* parent = subject.parent;
      assert(parent != nullptr);
      const size_t from = parent->index_of_child(subject);
      const size_t size = parent->children.size();
      assert(from != branch_node::index_of_none);

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