#include "./idle_node.h"
#include <cassert>
#include "helpers/vectors/move_item_to_index.h"
#include "./passkeys/initial_build.h"
#include "./passkeys/post_build_edit.h"
#include "./warnings/idle_has_multiple_next_siblings.h"
#include "../idles.h"
#include "./action_node.h"
#include "./graph_node.h"
#include "./loose_idle_list_node.h"

#include "../../files/file_load_order.h"
#include "../../forms/IdleAnimation.h"
#include "../../form_stub.h"

namespace dovah::datastores::impl::idles {
   idle_node::idle_node(datastore_type& d, form_stub& stub) : node(d), stub(stub) {
   }

   std::string idle_node::canonical_graph_path() const noexcept {
      auto loaded = this->stub.load().ptr_cast<loaded_forms::IdleAnimation>();
      if (!loaded)
         return {};
      return loaded->get_behavior_graph_path(false);
   }
   bool idle_node::is_defined_in_non_active_file() const noexcept {
      return !this->stub.get_owning_load_order().is_defined_in_active_file(this->stub);
   }
   bool idle_node::is_forced_loose() const noexcept {
      auto loaded = this->stub.load().ptr_cast<loaded_forms::IdleAnimation>();
      if (!loaded)
         return false;
      return !!(loaded->data.flags & loaded_forms::IdleAnimation::flag::is_forced_loose);
   }
   bool idle_node::is_queued_for_processing_before(const idle_node& that) const noexcept {
      return this->first_seen_anam < that.first_seen_anam;
   }

   const graph_node* idle_node::containing_graph() const noexcept {
      node* parent = this->canonical_parent;
      while (parent) {
         if (auto* casted = dynamic_cast<action_node*>(parent))
            return casted->graph;
         if (auto* casted = dynamic_cast<loose_idle_list_node*>(parent))
            return casted->graph;
         if (auto* casted = dynamic_cast<idle_node*>(parent)) {
            parent = casted->canonical_parent;
            continue;
         }
         assert(!dynamic_cast<graph_node*>(parent) && "Idle nodes cannot legally be children of graph nodes!");
         assert(false && "Idle node's parent type is unhandled here!");
      }
      return nullptr;
   }
   graph_node* idle_node::containing_graph() noexcept {
      return const_cast<graph_node*>(std::as_const(*this).containing_graph());
   }

   bool idle_node::contains(const idle_node& other) const noexcept {
      auto* idle = dynamic_cast<idle_node*>(other.canonical_parent);
      if (!idle)
         return false;
      do {
         if (idle == this)
            return true;
         idle = dynamic_cast<idle_node*>(idle->canonical_parent);
      } while (idle);
      return false;
   }
   size_t idle_node::index_of_child(const idle_node& other) const noexcept {
      for (size_t i = 0; i < this->child_idles.size(); ++i)
         if (this->child_idles[i] == &other)
            return i;
      return index_of_none;
   }

   bool idle_node::is_active_candidate_for(const action_node& action) const noexcept {
      auto& list = this->is_candidate_for.active;
      for (auto& item : list)
         if (item.action == &action)
            return true;
      return false;
   }
   bool idle_node::is_winning_root_of_action_in_own_graph() const noexcept {
      const auto path = this->canonical_graph_path();
      //
      auto _check = [this, &path](auto& list) {
         for (auto& item : list) {
            auto* action = item.action;
            assert(action != nullptr);
            assert(action->graph != nullptr);
            if (action->winning_root != this)
               continue;
            if (action->graph->path != path)
               continue;
            return true;
         }
         return false;
      };
      if (_check(this->is_candidate_for.masters))
         return true;
      if (_check(this->is_candidate_for.active))
         return true;
      return false;
   }

   // initial build:
   idle_node::internal_sort_state& idle_node::_get_sort_state(passkeys::initial_build) noexcept {
      return this->_sort_state;
   }
   void idle_node::_track_loaded_candidacy(passkeys::initial_build, action_node& action, const action_root_candidacy& cnd, bool is_master) {
      candidacy v = { cnd, &action };

      auto& list      = is_master ? this->is_candidate_for.masters : this->is_candidate_for.active;
      auto  insert_at = std::upper_bound(list.begin(), list.end(), v);
      bool  is_at_end = insert_at == list.end();
      list.insert(insert_at, v);

      if (is_at_end) {
         if (!is_master || this->is_candidate_for.active.empty()) {
            this->canonical_parent = &action;
         }
      }
   }
   void idle_node::_insert_sorted_child(passkeys::initial_build, idle_node& subject) {
      //
      // Insert the idle after its desired previous sibling, if said sibling 
      // is non-null and is already in our child list.
      //
      {
         idle_node* desired_prev = subject._sort_state.previous_idle;
         if (desired_prev) {
            auto i = this->index_of_child(*desired_prev);
            if (i != index_of_none)
               this->child_idles.insert(this->child_idles.begin() + i + 1, &subject);
            else
               this->child_idles.push_back(&subject);
         } else {
            this->child_idles.insert(this->child_idles.begin(), &subject);
         }
         subject.canonical_parent = this;
      }
      //
      // This idle may potentially be the desired previous sibling of an idle 
      // that was inserted earlier, so crawl the list and reorder the desired 
      // next sibling(s) as appropriate.
      //
      idle_node* current_node = &subject;
      idle_node* next_node    = nullptr;
      do {
         size_t current_index = index_of_none;
         size_t next_index    = index_of_none;
         for (size_t i = 0; i < this->child_idles.size(); ++i) {
            idle_node* candidate = this->child_idles[i];
            if (candidate == current_node) {
               current_index = i;
               continue;
            }
            if (candidate->_sort_state.previous_idle == current_node) {
               if (next_node) {
                  auto& dst = this->datastore.warnings.emplace_back();
                  dst = new warnings::idle_has_multiple_next_siblings(*current_node);
               } else {
                  next_node  = candidate;
                  next_index = i;
               }
            }
         }
         if (next_index == index_of_none) // no next idle?
            break;
         if (next_index == current_index + 1) // next idle is already where it should be?
            break;
         assert(next_node != nullptr);
         cobb::vectors::move_item_after_index(
            this->child_idles,
            next_index,
            current_index
         );
         //
         // Move on to next.
         //
         current_node = next_node;
         next_node    = nullptr;
      } while (current_node);
   }

   // post-build:
   void idle_node::_sever_active_candidacies_for(passkeys::post_build_edit, action_node& action) {
      auto& list        = this->is_candidate_for.active;
      bool  any_severed = false;
      for (auto& item : list) {
         if (item.action == &action) {
            item.action = nullptr;
            any_severed = true;
         }
      }
      if (any_severed) {
         std::erase_if(list, [](auto& item) { return item.action == nullptr; });
      }
   }
   void idle_node::_track_new_active_candidacy(passkeys::post_build_edit, action_node& action) {
      assert(this->is_candidate_for.active.empty());
      this->is_candidate_for.active.push_back({
         {{
            .offsets = {
               .of_record    = std::numeric_limits<size_t>::max(),
               .of_subrecord = std::numeric_limits<size_t>::max(),
            },
         }},
         &action
      });
   }
   void idle_node::_update_form_hierarchy_data(passkeys::post_build_edit) {
      auto loaded = this->stub.load().ptr_cast<loaded_forms::IdleAnimation>();
      if (!loaded)
         return;

      std::string_view graph_path;
      if (auto* graph = this->containing_graph())
         graph_path = graph->path;

      if (auto* parent_action = dynamic_cast<action_node*>(this->canonical_parent)) {
         assert(!this->is_candidate_for.active.empty() && "The datastore should've updated this first!");
         loaded->make_action_root(graph_path, parent_action->stub);
      } else if (auto* parent_idle = dynamic_cast<idle_node*>(this->canonical_parent)) {
         assert(this->is_candidate_for.active.empty() && "The datastore should've updated this first!");
         form_stub* previous = nullptr;
         {
            auto i = parent_idle->index_of_child(*this);
            assert(i != index_of_none);
            if (i > 0)
               previous = &parent_idle->child_idles[i - 1]->stub;
         }

         loaded->make_child(graph_path, parent_idle->stub, previous);
      } else if (auto* parent_loose = dynamic_cast<loose_idle_list_node*>(this->canonical_parent)) {
         assert(this->is_candidate_for.active.empty() && "The datastore should've updated this first!");
         loaded->make_loose(graph_path);
      } else if (!this->canonical_parent) {
         loaded->make_loose();
      } else {
         assert(false && "Unhandled type!");
      }
   }
}