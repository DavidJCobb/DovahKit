#include "./idles.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "./idles/_all.h"
#include "./idles/config/sort_during_initial_insertion.h"
#include "./idles/passkeys/check_is_building.h"
#include "./idles/passkeys/idle_sorting.h"
#include "../files/file_load_order.h"
#include "../form_stub.h"
#include "../form_types.h"
#include "../forms/IdleAnimation.h"
#include "./idles/warnings/cyclical_parent_relationships.h"
#include "./idles/warnings/cyclical_sibling_relationships.h"
#include "./idles/warnings/inconsistent_parentage.h"
#include "./idles/warnings/orphaned_idle.h"
#include "./idles/warnings/previous_sibling_is_not_as_expected.h"
#include "./idles/warnings/sibling_is_an_ancestor.h"
#include "./idles/warnings/siblings_have_mismatched_parents.h"

namespace {
   using loaded_idle_type = dovah::loaded_forms::IdleAnimation;
   using idle_flag = loaded_idle_type::flag;

   using namespace dovah::datastores::impl::idles::config;
   namespace warnings {
      using namespace dovah::datastores::impl::idles::warnings;
   }
}

namespace dovah::datastores {
   idles::idles() {
      this->loose.actions = new action_parent_node(*this);
      this->loose.idles   = new idle_parent_node(*this);
   }
   idles::~idles() {
      this->_clear();
   }

   void idles::build(file_load_order& lo) {
      this->reset();
      this->_is_building = true;
      //
      // Pre-create all idle nodes.
      //
      lo.for_each_form_of_type(form_type::idle, [this](dovah::form_stub* stub) -> bool {
         idle_node*& mapping = this->idles_by_stub[stub];
         mapping = new idle_node(*this, *stub);
         return false;
      });
      //
      // Build the parent/child hierarchy for the idle nodes.
      //
      for (auto& pair : this->idles_by_stub) {
         auto* stub   = pair.first;
         auto* node   = pair.second;
         auto  loaded = stub->load().ptr_cast<loaded_idle_type>();
         if (!loaded)
            continue;
         if (loaded->data.flags & idle_flag::parent) {
            this->_place_parent_idle(*node, *loaded);
         } else {
            this->_place_child_idle(*node, *loaded);
         }
      }
      std::sort(
         this->graphs.begin(),
         this->graphs.end(),
         _graph_node_sort_comparator
      );
      for (auto* graph : this->graphs) {
         if constexpr (sort_during_initial_insertion) {
            graph->sort_children();
         } else {
            graph->sort_descendants({});
         }
      }
      if constexpr (!sort_during_initial_insertion) {
         this->loose.actions->sort_descendants({});
         for (idle_node* idle : this->loose.idles->children) {
            idle->sort_descendants({});
         }
      }
      //
      // Final validation.
      //
      for (auto* graph : this->graphs) {
         for (action_node* action : graph->children)
            for (idle_node* idle : action->children)
               this->_post_placement_parentage_validation(*idle);
         for(idle_node* idle : graph->loose->children)
            this->_post_placement_parentage_validation(*idle);
      }
      //
      // The CK doesn't do this validation step for idles that are wholly orphaned and not in any 
      // graph, but we (ab)use the validation step to clean up sort-related state to avoid dangling 
      // pointers in the future, so we'll do it.
      //
      for (action_node* action : this->loose.actions->children)
         for (idle_node* idle : action->children)
            this->_post_placement_parentage_validation(*idle);
      for (idle_node* idle : this->loose.idles->children)
         this->_post_placement_parentage_validation(*idle);
      //
      this->_is_building = false;
   }
   void idles::reset() {
      if (auto& callback = this->callbacks.on_cleared.before; callback)
         (callback)();
      this->_clear();
      this->loose.actions = new action_parent_node(*this);
      this->loose.idles   = new idle_parent_node(*this);
      if (auto& callback = this->callbacks.on_cleared.after; callback)
         (callback)();
   }

   bool idles::_check_is_building(impl::idles::passkeys::check_is_building) const {
      return this->_is_building;
   }

   void idles::_clear() {
      this->idles_by_stub.clear();

      for (auto*& ptr : this->graphs) {
         if (!ptr)
            continue;
         delete ptr;
         ptr = nullptr;
      }
      this->graphs.clear();

      if (auto*& ptr = this->loose.actions) {
         delete ptr;
         ptr = nullptr;
      }
      if (auto*& ptr = this->loose.idles) {
         delete ptr;
         ptr = nullptr;
      }

      for (auto*& ptr : this->warnings) {
         if (!ptr)
            continue;
         delete ptr;
         ptr = nullptr;
      }
      this->warnings.clear();
   }

   #pragma region Initial build
      void idles::_place_parent_idle(idle_node& node, const loaded_idle_type& loaded_idle) {
         assert(loaded_idle.data.flags & idle_flag::parent);
         auto* parent = loaded_idle.parent.get_form_stub();
         const bool parent_is_action = parent && parent->form_type == form_type::action;

         idle_parent_node* parent_node = nullptr;

         auto* graph = this->get_or_create_graph_by_path(loaded_idle.corrected_behavior_graph_path());
         if (graph) {
            if (parent_is_action) {
               parent_node = graph->get_or_create_action(*parent);
            } else {
               parent_node = graph->loose;
               assert(parent_node != nullptr);
            }
         } else if (parent_is_action) {
            parent_node = this->get_or_create_loose_action(*parent);
            assert(parent_node != nullptr);
         }
         if (!parent_node) {
            parent_node = this->loose.idles;
            assert(parent_node != nullptr);
         }
         parent_node->append_child(node);
      }
      void idles::_place_child_idle(idle_node& child_idle, const loaded_idle_type& loaded_idle) {
         auto* graph = this->get_or_create_graph_by_path(loaded_idle.corrected_behavior_graph_path());

         dovah::form_stub* parent   = loaded_idle.parent.get_form_stub();
         dovah::form_stub* previous = loaded_idle.previous_sibling.get_form_stub();
         dovah::form_stub* action   = nullptr;
         if (parent && parent->form_type != form_type::idle) {
            if (parent->form_type == form_type::action)
               action = parent;
            parent = nullptr;
         }

         idle_parent_node* loose_parent_node = nullptr;

         seen_idle_set seen_ancestors;
         if (this->_has_cyclical_parentage(seen_ancestors, loaded_idle)) {
            {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::cyclical_parent_relationships(child_idle);
            }
            parent   = nullptr;
            previous = nullptr;
         }
         {
            auto problem = this->_check_siblings(seen_ancestors, loaded_idle);
            if (problem.has_value()) {
               if (*problem == sibling_problem::cyclical) {
                  auto& dst = this->warnings.emplace_back();
                  dst = new warnings::cyclical_sibling_relationships(child_idle);
               } else if (*problem == sibling_problem::ancestor) {
                  auto& dst = this->warnings.emplace_back();
                  dst = new warnings::sibling_is_an_ancestor(child_idle);
               } else if (*problem == sibling_problem::mismatched) {
                  auto& dst = this->warnings.emplace_back();
                  dst = new warnings::siblings_have_mismatched_parents(child_idle);
               }
               if (!loose_parent_node && parent) {
                  auto* g = this->graph_by_idle(*parent);
                  if (g)
                     loose_parent_node = g->loose;
               }
               if (!loose_parent_node && previous) {
                  auto* g = this->graph_by_idle(*previous);
                  if (g)
                     loose_parent_node = g->loose;
               }
               parent   = nullptr;
               previous = nullptr;
            }
         }

         {
            idle_node* parent_node   = nullptr;
            idle_node* previous_node = nullptr;
            if (parent) {
               auto it = this->idles_by_stub.find(parent);
               if (it != this->idles_by_stub.end())
                  parent_node = it->second;
            }
            if (previous) {
               auto it = this->idles_by_stub.find(previous);
               if (it != this->idles_by_stub.end())
                  previous_node = it->second;
            }
            child_idle._get_sort_state({}) = {
               .parent_idle   = parent_node,
               .previous_idle = previous_node,
            };

            if (parent_node) {
               if constexpr (sort_during_initial_insertion) {
                  parent_node->insert_sorted_child({}, child_idle, this->warnings);
               } else {
                  parent_node->append_child(child_idle);
               }
               return;
            }
         }

         if (action) {
            if (graph) {
               loose_parent_node = graph->get_or_create_action(*action);
            } else {
               loose_parent_node = this->get_or_create_loose_action(*action);
            }
            assert(loose_parent_node != nullptr);
         } else {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::orphaned_idle(child_idle);
         }
         if (!loose_parent_node) {
            loose_parent_node = this->loose.idles;
            assert(loose_parent_node != nullptr);
         }
         loose_parent_node->append_child(child_idle);
      }

      bool idles::_has_cyclical_parentage(seen_idle_set& seen, const loaded_idle_type& idle) {
         auto* current = idle.parent.get_form_stub();
         do {
            if (!current)
               break;
            if (current->form_type != dovah::form_type::idle)
               break;
            if (seen.contains(current))
               return true;
            seen.insert(current);

            auto loaded = current->load().ptr_cast<loaded_idle_type>();
            if (!loaded)
               break;
            current = loaded->parent.get_form_stub();
         } while (true);
         return false;
      }
      std::optional<idles::sibling_problem> idles::_check_siblings(const seen_idle_set& seen_ancestors, const loaded_idle_type& idle) {
         auto* parent  = idle.parent.get_form_stub();
         if (parent && parent->form_type != dovah::form_type::idle)
            parent = nullptr;

         auto* current = idle.previous_sibling.get_form_stub();
         if (!current || current->form_type != form_type::idle)
            return {};

         seen_idle_set seen_siblings;
         do {
            if (seen_ancestors.contains(current))
               return sibling_problem::ancestor;
            if (seen_siblings.contains(current))
               return sibling_problem::cyclical;
            seen_siblings.insert(current);

            auto loaded = current->load().ptr_cast<loaded_idle_type>();
            if (!loaded)
               break;

            auto* current_parent = loaded->parent.get_form_stub();
            if (current_parent && current_parent->form_type != form_type::idle)
               current_parent = nullptr;
            if (current_parent != parent)
               return sibling_problem::mismatched;

            //
            // Move on to next.
            //
            current = loaded->previous_sibling.get_form_stub();
            if (!current || current->form_type != form_type::idle)
               break;
         } while (true);
         return {};
      }

      void idles::_post_placement_parentage_validation(idle_node& idle) {
         auto* previous = idle._get_sort_state({}).previous_idle;
         auto* parent   = idle._get_sort_state({}).parent_idle;
         idle._get_sort_state({}) = {};

         if (!parent)
            return;

         size_t index_of_idle = parent->index_of_child(idle);
         if (index_of_idle == node::no_index) {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::inconsistent_parentage(idle);
         }
         if (previous) {
            idle_node* actual = nullptr;
            if (index_of_idle > 0) {
               actual = parent->children[index_of_idle - 1];
            }
            if (index_of_idle == 0 || actual != previous) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::previous_sibling_is_not_as_expected(idle, previous, actual);
            }
         }
         for (idle_node* child : idle.children) {
            this->_post_placement_parentage_validation(*child);
         }
      }
   #pragma endregion

   /*static*/ bool idles::_graph_node_sort_comparator(const graph_node* a, const graph_node* b) {
      if (!a) {
         return b == nullptr;
      } else if (!b) {
         return false;
      }
      const auto&  path_a   = a->path;
      const auto&  path_b   = b->path;
      const size_t min_size = std::min(path_a.size(), path_b.size());
      for (size_t i = 0; i < min_size; ++i) {
         auto ca = path_a[i];
         auto cb = path_b[i];
         if (ca >= 'a' && ca <= 'z')
            ca -= 0x20;
         if (cb >= 'a' && cb <= 'z')
            cb -= 0x20;
         if (ca != cb)
            return ca < cb;
      }
      return path_a.size() < path_b.size();
   }
   
   #pragma region Graph node getters
      const idles::graph_node* idles::graph_by_idle(dovah::form_stub& stub) const noexcept {
         if (stub.form_type != form_type::idle)
            return nullptr;
         auto loaded = stub.load().ptr_cast<loaded_idle_type>();
         if (!loaded)
            return nullptr;
         return this->graph_by_path(loaded->corrected_behavior_graph_path());
      }
      idles::graph_node* idles::graph_by_idle(dovah::form_stub& stub) noexcept {
         return const_cast<graph_node*>(std::as_const(*this).graph_by_idle(stub));
      }
      const idles::graph_node* idles::graph_by_path(std::string_view path) const noexcept {
         const size_t path_size = path.size();
         for (auto* graph : this->graphs) {
            auto& current_path = graph->path;
            if (current_path.size() != path_size)
               continue;
            bool matches = true;
            for (size_t i = 0; i < path_size; ++i) {
               char a = path[i];
               char b = current_path[i];
               if (a == b)
                  continue;
               if (a == '/' || a == '\\')
                  if (b == '/' || b == '\\')
                     continue;
               if (a >= 'a' && a <= 'z')
                  a -= 0x20;
               if (b >= 'a' && b <= 'z')
                  b -= 0x20;
               if (a != b) {
                  matches = false;
                  break;
               }
            }
            if (matches)
               return graph;
         }
         return nullptr;
      }
      idles::graph_node* idles::graph_by_path(std::string_view path) noexcept {
         return const_cast<graph_node*>(std::as_const(*this).graph_by_path(path));
      }
      idles::graph_node* idles::get_or_create_graph_by_path(std::string_view path) {
         auto* graph = this->graph_by_path(path);
         if (graph)
            return graph;
         if (path.empty())
            return nullptr;
         if (!this->_is_building) {
            auto* created = new graph_node(*this);
            try {
               created->path = path;
               auto it = std::upper_bound(
                  this->graphs.begin(),
                  this->graphs.end(),
                  created,
                  _graph_node_sort_comparator
               );
               if (auto& cb = this->callbacks.graphs.on_created.before)
                  cb(path, std::distance(this->graphs.begin(), it));
               this->graphs.insert(it, created);
               if (auto& cb = this->callbacks.graphs.on_created.after)
                  cb(path, *created);
            } catch (...) {
               delete created;
               throw;
            }
            return created;
         }
         auto& pointer = this->graphs.emplace_back();
         pointer = new graph_node(*this);
         pointer->path = path;
         return pointer;
      }
   #pragma endregion
   #pragma region Loose action getters
      const idles::action_node* idles::loose_action(const form_stub& stub) const noexcept {
         return this->loose.actions->action_by_stub(stub);
      }
      idles::action_node* idles::loose_action(const form_stub& stub) noexcept {
         return const_cast<action_node*>(std::as_const(*this).loose_action(stub));
      }
      idles::action_node* idles::get_or_create_loose_action(dovah::form_stub& stub) {
         return this->loose.actions->get_or_create_action(stub);
      }
   #pragma endregion

   const idles::idle_node* idles::idle_by_stub(const form_stub& stub) const noexcept {
      if (stub.form_type != form_type::idle)
         return nullptr;
      auto it = this->idles_by_stub.find((form_stub*)&stub); // can't use const pointer to look up non-const-pointer keys -_-
      if (it == this->idles_by_stub.end())
         return nullptr;
      return it->second;
   }
   idles::idle_node* idles::idle_by_stub(const form_stub& stub) noexcept {
      return const_cast<idle_node*>(std::as_const(*this).idle_by_stub(stub));
   }

   #pragma region Hierarchy helpers
      const idles::graph_node* idles::graph_by_idle(const idle_node& idle) const noexcept {
         const idle_parent_node* root = nullptr;
         {
            const idle_node* current = &idle;
            while (current->parent) {
               const idle_parent_node* parent = current->parent;
               if (auto* casted = dynamic_cast<const idle_node*>(parent)) {
                  current = casted;
               } else {
                  root = casted;
                  break;
               }
            }
         }
         if (!root) // idle isn't in the tree?
            return nullptr;
         if (root == this->loose.idles) // idle is top-level loose?
            return nullptr;
         if (auto* action = dynamic_cast<const action_node*>(root)) {
            const action_parent_node* grandparent = action->parent;
            if (!grandparent) // idle's parent action isn't in the tree?
               return nullptr;
            if (grandparent == this->loose.actions) // idle is inside of a loose action?
               return nullptr;
            if (auto* graph = dynamic_cast<const graph_node*>(grandparent))
               return graph;
         }
         //
         // May be a loose idle in a graph?
         //
         for (auto* graph : this->graphs)
            if (root == graph->loose)
               return graph;
         //
         // Unknown.
         //
         return nullptr;
      }
      idles::graph_node* idles::graph_by_idle(idle_node& idle) noexcept {
         return const_cast<graph_node*>(std::as_const(*this).graph_by_idle(idle));
      }
   #pragma endregion

   #pragma region Handlers for events occurring outside the datastore
      void idles::on_before_form_deleted(form_stub& stub) {
         switch (stub.form_type) {
            case form_type::idle:
               this->on_before_idle_deleted(stub);
               break;
            case form_type::action:
               {
                  auto _delete_if_present_in = [this, &stub](action_parent_node& parent) {
                     auto  index = parent.index_of_child(stub);
                     if (index == node::no_index)
                        return;
                     parent.destroy_child(index);
                  };
                  for (auto* graph : this->graphs) {
                     _delete_if_present_in(*graph);
                  }
                  _delete_if_present_in(*this->loose.actions);
               }
               break;
         }
      }

      void idles::on_action_modified(form_stub& stub) {
         if (stub.form_type != form_type::action)
            return;

         auto _re_sort = [this, &stub](action_parent_node& parent) {
            auto index = parent.index_of_child(stub);
            if (index == node::no_index)
               return;
            parent.re_sort_child(index);
         };

         for (auto* graph : this->graphs) {
            _re_sort(*graph);
         }
         _re_sort(*this->loose.actions);
      }

      void idles::on_idle_created(form_stub& stub) {
         if (stub.form_type != form_type::idle)
            return;
         auto loaded = stub.load().ptr_cast<loaded_idle_type>();
         if (!loaded)
            return;

         idle_node*& node = this->idles_by_stub[&stub];
         if (!node) {
            try {
               node = new idle_node(*this, stub);
            } catch (...) {
               this->idles_by_stub.erase(&stub);
               throw;
            }
         }
         if (auto* prev_stub = loaded->previous_sibling.get_form_stub()) {
            idle_node* prev_node = this->idle_by_stub(*prev_stub);
            if (prev_node) {
               this->place_idle_after(*node, *prev_node);
               return;
            }
         }
         idle_parent_node* parent_node = nullptr;
         if (auto* parent_stub = loaded->parent.get_form_stub()) {
            if (parent_stub->form_type == form_type::action) {
               if (auto* graph = this->graph_by_idle(stub)) {
                  parent_node = graph->get_or_create_action(*parent_stub);
               } else {
                  parent_node = this->get_or_create_loose_action(*parent_stub);
               }
            } else {
               parent_node = this->idle_by_stub(*parent_stub);
            }
         }
         if (!parent_node)
            parent_node = this->loose.idles;
         this->append_idle_in(*node, *parent_node);
      }
      void idles::on_before_idle_deleted(form_stub& stub) {
         if (stub.form_type != form_type::idle)
            return;
         idle_node* node = nullptr;
         {
            auto it = this->idles_by_stub.find(&stub);
            if (it == this->idles_by_stub.end())
               return;
            node = it->second;
            assert(node != nullptr);
         }
         //
         if (idle_parent_node* parent = node->parent) {
            auto i = parent->index_of_child(*node);
            parent->destroy_child(i); // this erases from `this->idles_by_stub` for us.
            node = nullptr;
         } else {
            delete node;
         }
      }
   #pragma endregion

   bool idles::place_idle_after(idle_node& idle, idle_node& desired_previous_sibling) {
      if (!desired_previous_sibling.parent)
         return false;
      idle_parent_node* parent_after = desired_previous_sibling.parent;
      size_t index_after = parent_after->index_of_child(desired_previous_sibling);
      assert(index_after != node::no_index);

      if (idle.parent) {
         size_t index_prior = idle.parent->index_of_child(idle);
         assert(index_prior != node::no_index);
         bool same_parent = idle.parent == parent_after;

         if (same_parent && index_prior == index_after - 1)
            return true;
      }
      parent_after->insert_child_after(idle, index_after);
      return true;
   }
   bool idles::append_idle_in(idle_node& idle, idle_parent_node& desired_parent) {
      desired_parent.append_child(idle);
      return true;
   }
}