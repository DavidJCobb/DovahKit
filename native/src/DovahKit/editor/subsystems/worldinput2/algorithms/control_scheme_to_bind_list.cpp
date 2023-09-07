#include "./control_scheme_to_bind_list.h"
#include <cassert>
#include "helpers/unreachable.h"
#include "../control_scheme/all_node_headers.h"
#include "../control_scheme.h"

#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldinput::algorithms {
   namespace {
      input_sequence _absolute_input_sequence(const control_scheme::node& target) {
         const auto& target_sequence = [&target]() {
            if (auto* casted = target.as<control_scheme_action>())
               return casted->data.input_sequence;
            if (auto* casted = target.as<control_scheme_modifier>())
               return casted->data.input_sequence;
            cobb::unreachable();
         }();
         for (auto* ancestor = target.parent(); ancestor; ancestor = ancestor->parent()) {
            //
            // Recursion and using operator<<= means that we'll clone just the outermost 
            // input sequence, and then modify it with the descendant sequences. Instead, 
            // we could do something like this:
            // 
            //  - At the start of this function, clone our input sequence and store it in 
            //    a variable, `absolute`.
            // 
            //  - Traverse up our ancestors without recursion.
            // 
            //  - When we encounter an input-node ancestor, overwrite absolute with the 
            //    expression `casted->input_sequence.clone() << absolute`.
            // 
            // But that would result in multiple clones, which -- until we implement flat 
            // ISG storage -- will mean tons of redundant heap allocations and frees.
            //
            if (auto* casted = ancestor->as<control_scheme_action>()) {
               return _absolute_input_sequence(*casted) <<= target_sequence;
            } else if (auto* casted = ancestor->as<control_scheme_modifier>()) {
               return _absolute_input_sequence(*casted) <<= target_sequence;
            }
         }
         return target_sequence.clone();

      }
   }

   extern bind_list control_scheme_to_bind_list(
      const control_scheme& src
   ) {
      auto out = bind_list(src.device_type);

      std::vector<control_scheme_condition> conditions;

      auto flatten_node = [&out, &conditions](this auto&& recurse, const control_scheme::node& current) -> void {
         if (auto* bt = current.as<control_scheme_action>()) {
            const auto& data = bt->data;
            auto& item = out.items.emplace_back();
            item.name               = data.name;
            if (!conditions.empty()) {
               item.conditions = conditions.back();
            }
            item.input_sequence     = _absolute_input_sequence(*bt);
            item.button_press_type  = data.button_press_type;
            item.bound_tool.tool    = data.tool.id;
            if (data.tool.options != nullptr && data.tool.id != worldedit::tools::id_of_none) {
               item.bound_tool.options = ((worldedit::tools::options_union*)data.tool.options)->clone();
            }

            // Action nodes are leaf nodes, so don't check for children to process.
            return;
         }

         if (!current.children.size())
            return;

         bool added_conditions = false;
         if (auto* casted = current.as<control_scheme_condition_node>()) {
            const auto& condition_info = casted->data.data;
            if (condition_info.impossible()) {
               return;
            }

            added_conditions = true;
            if (conditions.empty()) {
               conditions.push_back(condition_info);
            } else {
               auto& prior = conditions.back();
               conditions.push_back(prior & condition_info);

               if (conditions.back().impossible()) {
                  //
                  // The currently active conditions (from an ancestor of `current`) are incompatible with 
                  // the conditions on `current`; it is explicitly impossible for both sets of conditions 
                  // to be true at the same time. This means that none of `current`'s child or descendant 
                  // binds can ever trigger, so don't even bother processing them.
                  //
                  conditions.pop_back();
                  return;
               }
            }
         }

         for (const auto* child : current.children) {
            recurse(*child);
         }

         if (added_conditions) {
            conditions.pop_back();
         }
      };
      for (auto* tln : src.top_level_nodes)
         flatten_node(*tln);

      return out;
   }
}