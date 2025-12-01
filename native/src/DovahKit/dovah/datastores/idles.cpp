#include "./idles.h"
#include <cassert>
#include <memory>
#include "helpers/vectors/move_item_within.h"
#include "./idles/action_node.h"
#include "./idles/action_root_candidacy.h"
#include "./idles/graph_node.h"
#include "./idles/idle_node.h"
#include "./idles/loose_idle_list_node.h"
#include "./idles/passkeys/initial_build.h"
#include "./idles/passkeys/post_build_edit.h"
#include "./idles/warning.h"
#include "./idles/warnings/action_root_is_forced_loose.h"
#include "./idles/warnings/child_idle_is_an_action_root_candidate.h"
#include "./idles/warnings/child_of_idle_is_forced_loose.h"
#include "./idles/warnings/cyclical_parent_relationships.h"
#include "./idles/warnings/cyclical_sibling_relationships.h"
#include "./idles/warnings/idle_has_candidacies_for_multiple_actions.h"
#include "./idles/warnings/inconsistent_parentage_on_idle.h"
#include "./idles/warnings/loose_idle_is_not_flagged.h"
#include "./idles/warnings/no_loose_idle_list_for_idle.h"
#include "./idles/warnings/orphaned_idle.h"
#include "./idles/warnings/previous_sibling_is_not_as_expected.h"
#include "./idles/warnings/sibling_is_an_ancestor.h"
#include "./idles/warnings/siblings_have_mismatched_parents.h"
#include "../files/file_load_order.h"
#include "../forms/IdleAnimation.h"
#include "../form_stub.h"

namespace {
   using node                 = dovah::datastores::idles::node;
   using action_node          = dovah::datastores::idles::action_node;
   using graph_node           = dovah::datastores::idles::graph_node;
   using idle_node            = dovah::datastores::idles::idle_node;
   using loose_idle_list_node = dovah::datastores::idles::loose_idle_list_node;

   using action_root_candidacy = dovah::datastores::impl::idles::action_root_candidacy;

   namespace warnings {
      using namespace dovah::datastores::impl::idles::warnings;
   }

   // As of this writing, DovahKit's backend is designed to sever references to 
   // deleted forms whenever possible. This means that when we delete an idle, 
   // the active-file ANAM will be cleared out, making the idle loose. (We don't 
   // clear non-active-file ANAMs; see comments in IdleAnimation for details.)
   static constexpr const bool backend_severs_references_to_deleted_forms = true;
}

namespace dovah::datastores {
   namespace {
      static bool _graph_sort_comparator(const graph_node* a, const graph_node* b) {
         const auto&  name_a = a->path;
         const auto&  name_b = b->path;
         const size_t size   = std::min(name_a.size(), name_b.size());
         for (size_t i = 0; i < size; ++i) {
            char ca = name_a[i];
            char cb = name_b[i];
            if (ca >= 'A' && ca >= 'Z')
               ca += 0x20;
            if (cb >= 'A' && cb >= 'Z')
               cb += 0x20;
            if (ca != cb)
               return ca < cb;
         }
         return name_a.size() < name_b.size();
      }
   }

   idles::idles() {
      this->loose = new loose_idle_list_node(*this);
   }
   idles::~idles() {
      this->_clear();
   }

   void idles::_clear() {
      for (auto* graph : this->graphs)
         delete graph;
      this->graphs.clear();

      delete this->loose;
      this->loose = nullptr;

      for (auto& pair : this->idles_by_stub)
         delete pair.second;
      this->idles_by_stub.clear();

      for (auto* warning : this->warnings)
         delete warning;
      this->warnings.clear();
   }

   void idles::build(file_load_order& lo) {
      this->reset();
      //
      // Pre-create all idle nodes.
      //
      lo.for_each_form_of_type(form_type::idle, [this](form_stub* stub) {
         auto  node_ptr = std::make_unique<idle_node>(*this, *stub);
         auto& node     = *node_ptr;
         this->idles_by_stub[stub] = &node;
         node_ptr.release();

         auto loaded = node.stub.load().ptr_cast<loaded_idle_data>();
         if (loaded)
            this->_place_action_root(node, *loaded);

         return false;
      });
      //
      // Build the parent/child hierarchy for the idle nodes.
      //
      for (auto& pair : this->idles_by_stub) {
         auto& node   = *pair.second;
         auto  loaded = node.stub.load().ptr_cast<loaded_idle_data>();
         if (loaded) {
            if (loaded->data.flags & loaded_idle_data::flag::is_forced_loose) {
               this->_place_forced_loose_idle(node, *loaded);
            } else {
               this->_place_child_idle(node, *loaded);
            }
         }
      }
      //
      std::sort(
         this->graphs.begin(),
         this->graphs.end(),
         &_graph_sort_comparator
      );
      for (auto* graph : this->graphs) {
         graph->re_sort_all_actions({});
      }
      //
      for (auto& pair : this->idles_by_stub)
         this->_post_placement_parentage_validation(*pair.second);
   }
   void idles::reset() {
      this->_clear();
      this->loose = new loose_idle_list_node(*this);
   }
         
   #pragma region Handlers for events occurring outside the datastore
      void idles::on_before_form_fully_deleted(form_stub& stub) {
         assert(stub.get_owning_load_order().is_defined_in_active_file(stub) && "Only call this function for stubs that are being wholly deleted, not just flagged!");
         switch (stub.form_type) {
            case form_type::idle:
               {
                  auto it = this->idles_by_stub.find(&stub);
                  if (it == this->idles_by_stub.end())
                     return;
                  auto* node = it->second;
                  if (!node)
                     return;
                  assert(node->is_candidate_for.masters.empty() && "How is an idle defined in the active file becoming a candidate by virtue of a non-active file?!");
                  if (auto* parent = node->canonical_parent) {
                     if (auto& cb = this->callbacks.idle_deleted.before)
                        cb(*node);

                     this->_destroy_non_canonical_active_root_candidacies(*node);
                     auto* former_next_sibling = this->_take_idle_from_canonical_parent(*parent, *node);
                     if (auto* parent_action = dynamic_cast<action_node*>(parent)) {
                        node->canonical_parent = nullptr;
                        this->_update_canonical_parent_action_after_root_taken(*parent_action, *node);
                        assert(parent_action->winning_root != node && "The to-be-deleted idle should no longer be the winning root of an action!");
                        assert(!parent_action->candidates_include(*node) && "The to-be-deleted idle should no longer be a candidate of an action!");
                     }
                     this->idles_by_stub.erase(it);
                     delete node;
                     if (former_next_sibling)
                        this->_update_form_data(*former_next_sibling);

                     if (auto& cb = this->callbacks.idle_deleted.after)
                        cb(stub.formID);
                  } else {
                     this->idles_by_stub.erase(it);
                     delete node;
                  }
               }
               break;
            case form_type::action:
               for (auto* graph : this->graphs) {
                  size_t i = graph->index_of_action(stub);
                  if (i == node::index_of_none)
                     continue;

                  action_node* action = graph->actions[i];

                  // If the action has a winning root, and is the canonical parent of that 
                  // root (i.e. the root either isn't in multiple places, or it is, but the 
                  // action is the "primary" place), then move the winning root to loose. 
                  // This can cause another idle to slide into place and become a new winning 
                  // root, so we use a loop.
                  assert(action->candidacies.masters.empty() && "How does an action defined in the active file have candidates from non-active files?!");
                  if (action->winning_root) {
                     auto* loose = graph->loose;
                     while (idle_node* winner = action->winning_root) {
                        if (winner->canonical_parent != action)
                           break;
                        this->move_idle(*winner, *loose, nullptr);
                     }
                  }

                  if (auto& cb = this->callbacks.action_deleted.before)
                     cb(*action);

                  for (auto& cnd : action->candidacies.active) {
                     auto* idle = cnd.idle;
                     assert(idle != nullptr);
                     idle->_sever_active_candidacies_for({}, *action);
                     this->_update_form_data(*idle);
                  }
                  action->candidacies.active.clear();

                  graph->actions.erase(graph->actions.begin() + i);
                  delete action;

                  if (auto& cb = this->callbacks.action_deleted.after)
                     cb();
               }
               break;
         }
      }

      void idles::on_action_modified(form_stub& stub) {
         if (stub.form_type != form_type::action)
            return;
         for (auto* graph : this->graphs) {
            auto i = graph->index_of_action(stub);
            if (i != node::index_of_none)
               graph->re_sort_action({}, i);
         }
      }
      void idles::on_idle_editor_id_potentially_changed(form_stub& stub) {
         if (stub.form_type != form_type::idle)
            return;
         auto it = this->idles_by_stub.find(&stub);
         if (it == this->idles_by_stub.end())
            return;

         idle_node* idle = it->second;
         if (auto* loose = dynamic_cast<loose_idle_list_node*>(idle->canonical_parent)) {
            loose->re_sort_idle({}, loose->index_of_child(*idle));
         }
      }

      void idles::on_idle_created(form_stub& stub) {
         if (stub.form_type != form_type::idle)
            return;
         auto loaded = stub.load().ptr_cast<loaded_idle_data>();
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
         form_stub* parent_stub = loaded->get_hierarchy_parent();
         form_stub* prev_stub   = loaded->get_hierarchy_previous_sibling();
         idle_node* parent_idle = parent_stub ? this->idle_by_stub(*parent_stub) : nullptr;
         idle_node* prev_idle   = prev_stub ? this->idle_by_stub(*prev_stub) : nullptr;
         if (prev_idle) {
            if (auto* casted = dynamic_cast<idle_node*>(prev_idle->canonical_parent)) {
               parent_idle = casted;
            }
            if (parent_idle) {
               this->move_idle(*node, *parent_idle, prev_idle);
               return;
            }
         }
         if (parent_idle) {
            this->move_idle(*node, *parent_idle, nullptr);
            return;
         }
         if (parent_stub && parent_stub->form_type == form_type::action) {
            auto* graph = this->_get_or_create_graph(loaded->get_behavior_graph_path(false));
            if (graph) {
               auto* action = graph->get_action(*parent_stub);
               if (!action) {
                  action = new action_node(*this, *parent_stub);
                  auto i = graph->prospective_index_of(*action);
                  if (auto& cb = this->callbacks.action_inserted.before)
                     cb(*graph, *action, i);
                  graph->actions.insert(graph->actions.begin() + i, action);
                  action->graph = graph;
                  if (auto& cb = this->callbacks.action_inserted.after)
                     cb(*action);
               }
               assert(action != nullptr);
               this->move_idle(*node, *action, nullptr);
               return;
            }
         }
         this->move_idle(*node, *this->loose, nullptr);
         return;
      }
   #pragma endregion

   graph_node* idles::_get_or_create_graph(std::string_view path) {
      auto* graph = this->graph_by_path(path);
      if (graph)
         return graph;
      auto& dst = this->graphs.emplace_back();
      dst = new graph_node(*this, path);
      return dst;
   }
   action_node* idles::_get_or_create_action(std::string_view graph_path, form_stub* action) {
      if (!action || action->form_type != form_type::action)
         return nullptr;
      auto* graph = this->_get_or_create_graph(graph_path);
      if (!graph)
         return nullptr;
      return graph->get_or_create_action(*action);
   }
   void idles::_place_action_root(idle_node& idle, loaded_idle_data& loaded) {
      action_node* first_action   = nullptr;
      bool         actions_differ = false;
      //
      auto _on_action = [&first_action, &actions_differ](action_node& action) {
         if (actions_differ)
            return;
         if (first_action)
            actions_differ |= &action != first_action;
         else
            first_action = &action;
      };

      auto& candidacies = loaded.get_all_action_root_candidicacies();
      for (const auto& item : candidacies.masters) {
         auto* action = this->_get_or_create_action(item.behavior_graph_path, item.action.get_form_stub());
         if (!action)
            continue;
         _on_action(*action);

         auto candidacy = action_root_candidacy{
            .source_file = item.anam_subrecord.source_file,
            .offsets = {
               .of_record    = item.anam_subrecord.offsets.of_record,
               .of_subrecord = item.anam_subrecord.offsets.of_subrecord,
            },
         };
         action->_track_candidate({}, candidacy, idle, true);
         idle._track_loaded_candidacy({}, *action, candidacy, true);
      }
      for (const auto& item : candidacies.active) {
         auto* action = this->_get_or_create_action(item.behavior_graph_path, item.action.get_form_stub());
         if (!action)
            continue;
         _on_action(*action);

         auto candidacy = action_root_candidacy{
            .source_file = item.anam_subrecord.source_file,
            .offsets = {
               .of_record    = item.anam_subrecord.offsets.of_record,
               .of_subrecord = item.anam_subrecord.offsets.of_subrecord,
            },
         };
         action->_track_candidate({}, candidacy, idle, false);
         idle._track_loaded_candidacy({}, *action, candidacy, false);
      }

      if (actions_differ) {
         auto& dst = this->warnings.emplace_back();
         dst = new warnings::idle_has_candidacies_for_multiple_actions(idle);
      }
   }
   void idles::_place_forced_loose_idle(idle_node& idle, loaded_idle_data& loaded) {
      auto* graph = this->_get_or_create_graph(idle.canonical_graph_path());
      auto* loose = this->loose;
      if (graph) {
         loose = graph->loose;
      } else {
         auto& dst = this->warnings.emplace_back();
         dst = new warnings::no_loose_idle_list_for_idle(idle);
      }
      loose->child_idles.push_back(&idle);
      idle.canonical_parent = loose;

      if (idle.is_candidate_for.masters.size() || idle.is_candidate_for.active.size()) {
         auto& dst = this->warnings.emplace_back();
         dst = new warnings::action_root_is_forced_loose(idle);
      }
      if (auto* stub = loaded.get_hierarchy_parent()) {
         if (stub->form_type == form_type::idle) {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::child_of_idle_is_forced_loose(idle);
         }
      }
   }
   /*static*/ bool idles::_idle_has_cyclical_parentage(std::set<form_stub*>& seen, loaded_idle_data& loaded) {
      auto* current = loaded.get_hierarchy_parent();
      do {
         if (!current)
            break;
         if (current->form_type != dovah::form_type::idle)
            break;
         if (seen.contains(current))
            return true;
         seen.insert(current);

         auto loaded = current->load().ptr_cast<loaded_idle_data>();
         if (!loaded)
            break;
         current = loaded->get_hierarchy_parent();
      } while (true);
      return false;
   }
   /*static*/ idles::sibling_problem idles::_idle_has_bad_siblinghood(const std::set<form_stub*>& seen_ancestors, loaded_idle_data& loaded) {
      auto* parent  = loaded.get_hierarchy_parent();
      if (parent && parent->form_type != dovah::form_type::idle)
         parent = nullptr;

      auto* current = loaded.get_hierarchy_previous_sibling();
      if (!current || current->form_type != form_type::idle)
         return {};

      std::set<form_stub*> seen_siblings;
      do {
         if (seen_ancestors.contains(current))
            return sibling_problem::ancestor;
         if (seen_siblings.contains(current))
            return sibling_problem::cyclical;
         seen_siblings.insert(current);

         auto loaded = current->load().ptr_cast<loaded_idle_data>();
         if (!loaded)
            break;

         auto* current_parent = loaded->get_hierarchy_parent();
         if (current_parent && current_parent->form_type != form_type::idle)
            current_parent = nullptr;
         if (current_parent != parent)
            return sibling_problem::mismatched;

         //
         // Move on to next.
         //
         current = loaded->get_hierarchy_previous_sibling();
         if (!current || current->form_type != form_type::idle)
            break;
      } while (true);
      return sibling_problem::none;
   }
   void idles::_place_child_idle(idle_node& idle, loaded_idle_data& loaded) {
      auto* graph = this->_get_or_create_graph(loaded.get_behavior_graph_path(false));

      form_stub* parent   = loaded.get_hierarchy_parent();
      form_stub* previous = loaded.get_hierarchy_previous_sibling();
      bool is_action_root_in_own_graph = idle.is_winning_root_of_action_in_own_graph();
      if (parent && parent->form_type != form_type::idle) {
         parent = nullptr;
      }

      loose_idle_list_node* loose_parent_node = nullptr;

      std::set<form_stub*> seen_ancestors;
      if (this->_idle_has_cyclical_parentage(seen_ancestors, loaded)) {
         {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::cyclical_parent_relationships(idle);
         }
         parent   = nullptr;
         previous = nullptr;
      } else {
         auto problem = this->_idle_has_bad_siblinghood(seen_ancestors, loaded);
         if (problem != sibling_problem::none) {
            if (problem == sibling_problem::cyclical) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::cyclical_sibling_relationships(idle);
            } else if (problem == sibling_problem::ancestor) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::sibling_is_an_ancestor(idle);
            } else if (problem == sibling_problem::mismatched) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::siblings_have_mismatched_parents(idle);
            }
            if (graph) {
               loose_parent_node = graph->loose;
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
      //
      // A minor note about the corrections made to the idle pointers 
      // above. These are the same corrections that the game makes, but 
      // this case, we don't *retain* the corrections. That is: the game 
      // directly modifies the loaded `TESIdleForm` objects in memory to 
      // have the corrected pointers, whereas we don't.
      //
      // The only effect this should have is that when we build the trees, 
      // we can't early-out as quickly. If for example there's a cyclical 
      // sibling relationship between A < B < C < D < A, then:
      //
      //  - The game nulls out A's previous-sibling and parent; then when 
      //    it checks B, B has a different parent from A and fails right 
      //    off rip. This cascades such that C and D both fail quickly as 
      //    well.
      //
      //  - ...whereas DovahKit has to detect the full cycle every time 
      //    it processes any of the siblings in that cycle.
      //
      // We still produce the same tree as the CK once we're done, and 
      // doing it this way is simpler. As a potential optimization, we 
      // could split building from two steps into three: create all idle 
      // nodes; then [new step] set all idle nodes' sort states blindly 
      // based on what their ANAM wants; then, in this function, we'd use 
      // (and modify) the sort states alone, allowing us to early-out 
      // exactly as the CK does. But having that extra step would be slower 
      // to no benefit when dealing with well-formed idle trees.
      //

      // Find parent-node and previous-node given parent-idle-form and 
      // previous-idle-form.
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
         idle._get_sort_state({}) = {
            .parent_idle   = parent_node,
            .previous_idle = previous_node,
         };

         if (parent_node) {
            if (idle.is_candidate_for.masters.size() || idle.is_candidate_for.active.size()) {
               auto& dst = this->warnings.emplace_back();
               dst = new warnings::child_idle_is_an_action_root_candidate(idle);
               // "Idle %1 has been placed as both an action root and a child idle, and so may end up in multiple places at once."
            }
            parent_node->_insert_sorted_child({}, idle);
            return;
         }
      }

      if (is_action_root_in_own_graph) {
         //
         // The idle can't be loose if it's a winning action root in its 
         // containing graph. It will have already been tied to the action 
         // via the `_place_action_root` function.
         //
         return;
      }
      if (loose_parent_node) {
         // Same warning as the CK, but with more precise wording.
         auto& dst = this->warnings.emplace_back();
         dst = new warnings::loose_idle_is_not_flagged(idle);
         // "Idle %1 has ended up loose, but wasn't originally flagged as loose. Is this intentional?"
      } else {
         if (graph) {
            loose_parent_node = graph->loose;

            // Same warning as the CK, but with more precise wording.
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::loose_idle_is_not_flagged(idle);
            // "Idle %1 has ended up loose, but wasn't originally flagged as loose. Is this intentional?"
         } else {
            loose_parent_node = this->loose;

            auto& dst = this->warnings.emplace_back();
            dst = new warnings::orphaned_idle(idle);
         }
      }
      loose_parent_node->child_idles.push_back(&idle); // TODO: alphabetically sorted insertion
      idle.canonical_parent = loose_parent_node;
   }
   void idles::_post_placement_parentage_validation(idle_node& idle) {
      auto* previous = idle._get_sort_state({}).previous_idle;
      auto* parent   = idle._get_sort_state({}).parent_idle;
      idle._get_sort_state({}) = {};

      if (!parent) {
         assert(!dynamic_cast<idle_node*>(idle.canonical_parent));
      }

      if (!parent)
         return;
      auto& siblings = parent->child_idles;
      
      size_t i = parent->index_of_child(idle);
      if (i == node::index_of_none) {
         auto& dst = this->warnings.emplace_back();
         dst = new warnings::inconsistent_parentage_on_idle(idle);
      } else if (previous) {
         idle_node* actual = nullptr;
         if (i > 0)
            actual = siblings[i - 1];
         if (i == 0 || actual != previous) {
            auto& dst = this->warnings.emplace_back();
            dst = new warnings::previous_sibling_is_not_as_expected(idle, previous, actual);
         }
      }
   }

   const graph_node* idles::graph_by_path(std::string_view path) const noexcept {
      for (auto* graph : this->graphs)
         if (graph->path_equals(path))
            return graph;
      return nullptr;
   }
   graph_node* idles::graph_by_path(std::string_view path) noexcept {
      return const_cast<graph_node*>(std::as_const(*this).graph_by_path(path));
   }
   const graph_node* idles::graph_by_idle(idle_node& idle) const noexcept {
      return this->graph_by_path(idle.canonical_graph_path());
   }
   graph_node* idles::graph_by_idle(idle_node& idle) noexcept {
      return const_cast<graph_node*>(std::as_const(*this).graph_by_idle(idle));
   }
   const graph_node* idles::graph_by_idle(form_stub& stub) const noexcept {
      if (stub.form_type != form_type::idle)
         return nullptr;
      {
         auto it = this->idles_by_stub.find(&stub);
         if (it != this->idles_by_stub.end()) {
            idle_node* idle = it->second;
            if (idle->canonical_parent)
               return idle->containing_graph();
         }
      }
      auto loaded = stub.load().ptr_cast<loaded_idle_data>();
      if (!loaded)
         return nullptr;
      return this->graph_by_path(loaded->get_behavior_graph_path(false));
   }
   graph_node* idles::graph_by_idle(form_stub& stub) noexcept {
      return const_cast<graph_node*>(std::as_const(*this).graph_by_idle(stub));
   }

   graph_node* idles::get_or_create_graph(std::string_view path) {
      auto* graph = this->graph_by_path(path);
      if (graph)
         return graph;

      auto graph_ptr = std::make_unique<graph_node>(*this, path);
      graph = graph_ptr.get();

      auto it = std::upper_bound(this->graphs.begin(), this->graphs.end(), graph, &_graph_sort_comparator);

      if (auto& cb = this->callbacks.graph_inserted.before)
         cb(*graph, std::distance(this->graphs.begin(), it));

      this->graphs.insert(it, graph);
      graph_ptr.release();

      if (auto& cb = this->callbacks.graph_inserted.after)
         cb(*graph);

      return graph;
   }

   const idle_node* idles::idle_by_stub(const form_stub& stub) const {
      if (stub.form_type != form_type::idle)
         return nullptr;
      auto it = this->idles_by_stub.find((form_stub*)&stub); // std::unordered_map::find chokes on const-pointer keys if the key type is non-const
      if (it == this->idles_by_stub.end())
         return nullptr;
      return it->second;
   }
   idle_node* idles::idle_by_stub(const form_stub& stub) {
      return const_cast<idle_node*>(std::as_const(*this).idle_by_stub(stub));
   }

   // This only checks whether a given movement would produce a result which is 
   // representable given the file format and tree-building algorithm. This is 
   // not intended to prevent moves that are merely bad ideas (e.g. moves that 
   // would cause the tree to be degenerate in a way that: Bethesda doesn't 
   // guard against, and that therefore has to be representable by our code). 
   // For those, see `is_idle_movement_a_really_bad_idea`.
   bool idles::is_idle_movement_legal(const idle_node& subject, const node& dst_parent, const idle_node* dst_previous) const {
      if (dynamic_cast<const graph_node*>(&dst_parent))
         return false;

      // Moving an idle inside itself or one of its descendants is illegal.
      if (&subject == &dst_parent)
         return false;
      if (auto* casted = dynamic_cast<const idle_node*>(&dst_parent))
         if (subject.contains(*casted))
            return false;

      // Prevent inconsistent moves (e.g. "move into A, after B" if B is not a child of A)
      if (dst_previous)
         if (auto* casted = dynamic_cast<const idle_node*>(&dst_parent))
            if (casted->index_of_child(*dst_previous) == node::index_of_none)
               return false;

      return true;
   }

   bool idles::is_idle_movement_a_really_bad_idea(const idle_node& subject, const node& dst_parent, const idle_node* dst_previous) const {
      // Moving action roots is a bad idea.
      if (dynamic_cast<const action_node*>(subject.canonical_parent))
         return true;

      // Displacing action roots is a bad idea.
      if (auto* dst_action = dynamic_cast<const action_node*>(&dst_parent))
         if (dst_action->winning_root)
            return true;

      // Bethesda doesn't intend for loose idles to have children, so moving 
      // an idle that has children into LOOSE is a bad idea.
      if (subject.child_idles.size())
         if (dynamic_cast<const loose_idle_list_node*>(&dst_parent))
            return true;

      // Similarly, moving an idle to be the child of a loose idle is also a 
      // bad idea, though we'll allow it if the latter already has children.
      if (auto* dst_idle = dynamic_cast<const idle_node*>(&dst_parent))
         if (dst_idle->child_idles.empty())
            if (dynamic_cast<const loose_idle_list_node*>(dst_idle->canonical_parent))
               return true;

      return false;
   }

   bool idles::is_idle_deletion_legal(const idle_node&) const {
      return true;
   }
   bool idles::is_idle_deletion_a_really_bad_idea(const idle_node& subject) const {
      // Deleting an action-root defined by a non-active file is a bad idea, 
      // since it won't necessarily stop being the root for that action.
      if (!subject.is_candidate_for.masters.empty())
         return true;

      return false;
   }

   #pragma region Helpers for editing operations
      void idles::_on_runner_up_became_root(action_node& action) {
         idle_node* idle = action.winning_root;
         assert(idle != nullptr);

         if (idle->canonical_parent == &action)
            return;
         if (idle->canonical_parent == action.graph->loose) {
            auto* loose = action.graph->loose;
            if (idle->is_forced_loose()) {
               if (auto& cb = this->callbacks.idle_becoming_multiply_present_in)
                  cb(*idle, action);
            } else {
               if (auto& cb = this->callbacks.idle_moved.before)
                  cb(*idle, action, 0);
               loose->child_idles.erase(loose->child_idles.begin() + loose->index_of_child(*idle));
               idle->canonical_parent = &action;
               if (auto& cb = this->callbacks.idle_moved.after)
                  cb(*idle);
            }
         } else {
            if (auto& cb = this->callbacks.idle_becoming_multiply_present_in)
               cb(*idle, action);
         }
      }

      // This should be invoked for an idle before it is moved or deleted. If the 
      // idle is an active-file candidate for any action roots besides its canonical 
      // parent, then [by definition the idle is in multiple places, and] this severs 
      // those candidacies and emits callbacks for the idle no longer being multiply 
      // present in those locations.
      //
      // The canonical parent is skipped here, and should be handled by the caller 
      // as appropriate for the given operation (move versus delete).
      void idles::_destroy_non_canonical_active_root_candidacies(idle_node& subject) {
         auto&  list = subject.is_candidate_for.active;
         size_t size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto* action = list[i].action;
            if (action == subject.canonical_parent)
               continue;
            bool was_root = action->winning_root == &subject;
            action->_untrack_active_file_candidate({}, subject);
            if (was_root) {
               action->_recalc_winning_root({});
               if (action->winning_root != &subject) {
                  if (auto& cb = this->callbacks.idle_no_longer_multiply_present_in)
                     cb(subject, *action);
               }
               if (action->winning_root) {
                  this->_on_runner_up_became_root(*action);
               }
            }
            list.erase(list.begin() + i);
            --i;
            --size;
         }
      }

      // To be invoked as part of move- or delete-idle operations. Returns the 
      // idle's former next-sibling. The caller must have already severed the 
      // idle's active-file action root candidacies, and is responsible for 
      // telling the idle to where it has been relocated.
      idle_node* idles::_take_idle_from_canonical_parent(node& take_from, idle_node& idle) {
         idle_node* former_next_sibling = nullptr;
         if (auto* from_action = dynamic_cast<action_node*>(&take_from)) {
            from_action->_untrack_active_file_candidate({}, idle);
            assert(idle.is_candidate_for.active.size() <= 1);
            idle.is_candidate_for.active.clear();
         } else {
            assert(idle.is_candidate_for.active.size() == 0);
            if (auto* from_idle = dynamic_cast<idle_node*>(&take_from)) {
               auto i = from_idle->index_of_child(idle);
               assert(i != node::index_of_none);
               if (i + 1 < from_idle->child_idles.size())
                  former_next_sibling = from_idle->child_idles[i + 1];
               from_idle->child_idles.erase(from_idle->child_idles.begin() + i);
            } else if (auto* from_loose = dynamic_cast<loose_idle_list_node*>(&take_from)) {
               auto i = from_loose->index_of_child(idle);
               assert(i != node::index_of_none);
               from_loose->child_idles.erase(from_loose->child_idles.begin() + i);
            }
         }
         return former_next_sibling;
      }

      void idles::_update_canonical_parent_action_after_root_taken(action_node& taken_from, idle_node& taken_idle) {
         taken_from._recalc_winning_root({});
         if (&taken_idle == taken_from.winning_root) {
            //
            // If `taken_idle` is still the winning root of the action we just 
            // moved it from, then it must now be present in multiple places, with 
            // its canonical parent being somewhere else.
            //
            if (auto& cb = this->callbacks.idle_becoming_multiply_present_in)
               cb(taken_idle, taken_from);
         } else if (taken_from.winning_root) {
            this->_on_runner_up_became_root(taken_from);
         }
      }

      void idles::_update_form_data(idle_node& idle) {
         if (auto& cb = this->callbacks.form_data_modified.before)
            cb(idle.stub);
         idle._update_form_hierarchy_data({});
         if (auto& cb = this->callbacks.form_data_modified.after)
            cb(idle.stub);
      }
   #pragma endregion

   void idles::_delete_single_idle(idle_node& subject) {
      assert(!!this->handlers.delete_idle);

      const bool defined_in_master = subject.is_defined_in_non_active_file();

      auto& subject_form_stub = subject.stub;
      auto  subject_form_id = subject.stub.formID;

      if constexpr (backend_severs_references_to_deleted_forms) {
         auto* graph = subject.containing_graph();
         auto* loose = graph ? graph->loose : this->loose;
         this->move_idle(subject, *loose, nullptr);
         assert(subject.canonical_parent == loose);
         if (!defined_in_master) {
            if (auto& cb = this->callbacks.idle_deleted.before)
               cb(subject);
            this->_take_idle_from_canonical_parent(*loose, subject);
            subject.canonical_parent = nullptr;
            this->idles_by_stub.erase(&subject_form_stub);
            this->handlers.delete_idle(subject_form_stub);
            delete &subject;
            if (auto& cb = this->callbacks.idle_deleted.after)
               cb(subject_form_id);
         }
      } else {
         if (defined_in_master) {
            //
            // The IDLE form was originally defined outside of the active file 
            // and so cannot be wholly deleted. The most we can do is move it 
            // to LOOSE and flag it and its descendants as "deleted."
            //
            auto* graph = subject.containing_graph();
            auto* loose = graph ? graph->loose : this->loose;
            this->move_idle(subject, *loose, nullptr);
            //
            this->handlers.delete_idle(subject_form_stub); // flag the form as "deleted"
            return;
         }

         this->_destroy_non_canonical_active_root_candidacies(subject);

         auto& form = subject.stub;
         if (auto& cb = this->callbacks.idle_deleted.before)
            cb(subject);

         auto* moved_from = subject.canonical_parent;
         auto* former_next_sibling = this->_take_idle_from_canonical_parent(*moved_from, subject);
         subject.canonical_parent = nullptr;
         if (former_next_sibling)
            this->_update_form_data(*former_next_sibling);
         this->_update_form_data(subject);

         this->idles_by_stub.erase(&subject_form_stub);
         this->handlers.delete_idle(subject_form_stub);

         if (auto& cb = this->callbacks.idle_deleted.after)
            cb(subject_form_id);

         if (auto* casted = dynamic_cast<action_node*>(moved_from))
            this->_update_canonical_parent_action_after_root_taken(*casted, subject);

         assert(subject.is_candidate_for.masters.empty());
         assert(subject.is_candidate_for.active.empty());
         delete &subject;
      }
   }

   void idles::delete_idle(idle_node& subject) {
      assert(!!this->handlers.delete_idle);

      auto children = subject.child_idles;
      for (idle_node* child : children)
         this->delete_idle(*child);

      this->_delete_single_idle(subject);
   }

   void idles::move_idle(idle_node& subject, node& dst_parent, idle_node* dst_previous) {
      // Skip redundant operations.
      if (subject.canonical_parent == &dst_parent) {
         if (auto* dst_idle = dynamic_cast<idle_node*>(&dst_parent)) {
            if (dst_previous) {
               auto i = dst_idle->index_of_child(subject);
               assert(i != node::index_of_none);
               if (i > 0 && dst_idle->child_idles[i - 1] == dst_previous)
                  return;
            } else {
               if (!dst_idle->child_idles.empty() && dst_idle->child_idles[0] == &subject)
                  return;
            }
            //
            // Else movement proceeds, because the idle can still be reordered 
            // within its parent. (All other parent types are unordered.)
            //
         } else if (auto* dst_loose = dynamic_cast<loose_idle_list_node*>(&dst_parent)) {
            if (subject.is_forced_loose()) {
               //
               // If a subject happened to *end up* in LOOSE, rather than being 
               // made loose intentionally, then an explicit move to LOOSE is 
               // not strictly a no-op; make the subject intentionally loose and 
               // then exit.
               //
               this->_update_form_data(subject);
            }
            return;
         } else {
            return;
         }
      }

      // Skip impossible operations.
      if (!this->is_idle_movement_legal(subject, dst_parent, dst_previous))
         return;

      // If moving to an action, displace any action root which is already there.
      if (auto* dst_action = dynamic_cast<action_node*>(&dst_parent)) {
         //
         // We need a loop because *some* kinds of displacement may "summon" a 
         // runner-up root into the spot, and we'll then need to displace that 
         // runner-up as well.
         //
         while (dst_action->winning_root) {
            graph_node* graph     = dst_action->graph;
            idle_node*  displaced = dst_action->winning_root;
            assert(graph != nullptr);
            dst_action->winning_root = nullptr;
            if (displaced->canonical_parent == dst_action) {
               //
               // The destination is the to-be-displaced idle's canonical parent, 
               // so that idle must be moved.
               //
               auto* displaced_into = graph->loose;
               if (displaced->is_active_candidate_for(*dst_action)) {
                  //
                  // The to-be-displaced idle is placed here by the active file, 
                  // so let's do a fully-fledged move operation to make it a 
                  // loose idle within the active file. Because this runs the 
                  // full move algorithm, it may summon a runner-up.
                  //
                  this->move_idle(*displaced, *displaced_into, nullptr);
               } else {
                  //
                  // The to-be-displaced idle is placed here by a master file, 
                  // so it'll be displaced to a loose idle. The difference 
                  // between this and the contrary branch is the difference 
                  // between the idle being "made" a loose idle versus it 
                  // "ending up as" a loose idle.
                  //
                  auto i = displaced_into->prospective_index_of(*displaced);

                  if (auto& cb = this->callbacks.idle_moved.before)
                     cb(*displaced, *displaced_into, i);
                  displaced_into->child_idles.insert(displaced_into->child_idles.begin() + i, displaced);
                  displaced->canonical_parent = displaced_into;
                  if (auto& cb = this->callbacks.idle_moved.after)
                     cb(*displaced);
                  //
                  // This wasn't the full move algorithm and therefore did not 
                  // process runner-ups. We can exit.
                  //
                  break;
               }
            } else {
               //
               // The to-be-displaced idle is in multiple places at once, the 
               // destination is one of those, and the destination is not the 
               // to-be-displaced idle's canonical parent. At minimum, the 
               // to-be-displaced idle will cease to be multiply present in 
               // this action.
               //
               if (auto& cb = this->callbacks.idle_no_longer_multiply_present_in)
                  cb(*displaced, *dst_action);
               if (displaced->is_active_candidate_for(*dst_action)) {
                  //
                  // The displaced idle is placed in multiple locations by the 
                  // active file specifically, and the destination is one of 
                  // those places. The idle we're moving is also going to be 
                  // put here, and we can't predict in advance which of these 
                  // idles will "win" (i.e. which will come later in the file 
                  // that we eventually save back out).
                  //
                  // To avoid an ambiguous ordering, adjust the displaced idle 
                  // so that the active file no longer places it here.
                  //
                  this->move_idle(*displaced, *graph->loose, nullptr);
               } else {
                  //
                  // This wasn't the full move algorithm and therefore did not 
                  // process runner-ups. We can exit.
                  //
                  break;
               }
            }
         }
         assert(dst_action->winning_root == nullptr);
      }

      // Destroy the subject's active-file action root candidacies, except that 
      // pertaining to its canonical parent. (That particular candidacy will be 
      // destroyed when we move the subject, further below.)
      this->_destroy_non_canonical_active_root_candidacies(subject);

      size_t insert_at = 0;
      if (auto* dst_idle = dynamic_cast<idle_node*>(&dst_parent)) {
         if (dst_previous) {
            insert_at = dst_idle->index_of_child(*dst_previous) + 1;
         }
      } else if (auto* dst_loose = dynamic_cast<loose_idle_list_node*>(&dst_parent)) {
         insert_at = dst_loose->prospective_index_of(subject);
      }

      if (auto& cb = this->callbacks.idle_moved.before)
         cb(subject, dst_parent, insert_at);

      idle_node* former_next_sibling = nullptr;
      node*      moved_from = subject.canonical_parent;
      if (moved_from) {
         if (moved_from == &dst_parent) {
            size_t moved_from_index = insert_at + 1;
            if (auto* from_idle = dynamic_cast<idle_node*>(moved_from))
               moved_from_index = from_idle->index_of_child(subject);
            else if (auto* from_loose = dynamic_cast<loose_idle_list_node*>(moved_from))
               moved_from_index = from_loose->index_of_child(subject);
            if (insert_at >= moved_from_index)
               --insert_at;
         }
         former_next_sibling = this->_take_idle_from_canonical_parent(*moved_from, subject);
      }
      subject.canonical_parent = &dst_parent;
      if (auto* dst_action = dynamic_cast<action_node*>(&dst_parent)) {
         subject._track_new_active_candidacy({}, * dst_action);
         assert(dst_action->candidacies.active.empty());
         dst_action->_track_active_file_candidate({}, subject);
         dst_action->winning_root = &subject;
      } else if (auto* dst_loose = dynamic_cast<loose_idle_list_node*>(&dst_parent)) {
         dst_loose->child_idles.insert(dst_loose->child_idles.begin() + insert_at, &subject);
      } else if (auto* dst_idle = dynamic_cast<idle_node*>(&dst_parent)) {
         dst_idle->child_idles.insert(dst_idle->child_idles.begin() + insert_at, &subject);
         if (insert_at + 1 < dst_idle->child_idles.size()) {
            idle_node* new_next_sibling = dst_idle->child_idles[insert_at + 1];
            assert(new_next_sibling != nullptr);
            this->_update_form_data(*new_next_sibling);
         }
      } else {
         assert(false && "unhandled case");
      }
      auto graph_path_prior = subject.canonical_graph_path();
      this->_update_form_data(subject);
      auto graph_path_after = subject.canonical_graph_path();
      if (former_next_sibling)
         this->_update_form_data(*former_next_sibling);

      if (auto& cb = this->callbacks.idle_moved.after)
         cb(subject);

      // Update the action the subject was moved from (if any).
      if (auto* casted = dynamic_cast<action_node*>(moved_from))
         this->_update_canonical_parent_action_after_root_taken(*casted, subject);

      // If the subject was moved across graphs, update form data for all of its 
      // descendants.
      if (graph_path_prior != graph_path_after) {
         //
         // Moving the subject across graphs should, in general, force changes 
         // to all of its descendants to update their DNAM subrecords. I don't 
         // believe that's strictly necessary, but it seems like it'd make for 
         // the cleanest serialized data.
         //
         [this](this auto&& recurse, idle_node& parent) -> void {
            for (idle_node* idle : parent.child_idles) {
               this->_update_form_data(*idle);
               recurse(*idle);
            }
         }(subject);
      }
   }
   void idles::move_idle_within_parent(idle_node& subject, int by) {
      idle_node*   parent = dynamic_cast<idle_node*>(subject.canonical_parent);
      assert(parent != nullptr);
      const size_t from   = parent->index_of_child(subject);
      const size_t size   = parent->child_idles.size();
      assert(from != node::index_of_none);

      idle_node* previous_prior = nullptr;
      idle_node* previous_after = nullptr;
      if (from)
         previous_prior = parent->child_idles[from - 1];

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
      if (auto& cb = this->callbacks.idle_moved.before)
         cb(subject, *parent, to);

      cobb::vectors::move_item_within(parent->child_idles, from, by);

      if (previous_prior)
         this->_update_form_data(*previous_prior);
      this->_update_form_data(subject);
      {
         auto i = parent->index_of_child(subject);
         if (i < size - 1)
            previous_after = parent->child_idles[i];
      }
      if (previous_after)
         this->_update_form_data(*previous_after);

      if (auto& cb = this->callbacks.idle_moved.after)
         cb(subject);
   }
}