#include "./bind_list.h"

#include "./debugging.h"
#include "./tools/combined_tool_results.h"

#include "./algorithms/concurrent_bind_conflict_resolution.h"
#include "./algorithms/hold_blocks_press.h"
#include "./algorithms/press_preempts_hold.h"
#include "./devices/abstract_device_handler.h"
#include "./tools/combined_tool_results.h"
#include "./tools/opaque_tool_options.h"
#include "./util/is_delta_control.h"
#include "./core.h"
#include "./defaults.h"
#include "./interruption_check.h"
//
#include "editor/subsystems/worldedit/core.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "editor/subsystems/worldedit/tool_system/tool_dispatch_table.h"

namespace dovahkit::subsystems::worldinput2 {
   #pragma region bind_list_item
   bind_list_item::bind_list_item(const bind_list_item& other) {
      *this = other;
   }
   bind_list_item::bind_list_item(bind_list_item&& other) noexcept {
      *this = std::move(other);
   }

   bind_list_item::~bind_list_item() {
      if (auto*& p = this->bound_tool.options) {
         delete ((worldedit::tools::options_union*)p);
         p = nullptr;
      }
   }

   bind_list_item& bind_list_item::operator=(const bind_list_item& src) {
      this->name = src.name;
      this->conditions = src.conditions;
      //
      this->button_press_type = src.button_press_type;
      this->input_sequence    = src.input_sequence.clone();
      //
      this->bound_tool.tool   = src.bound_tool.tool;
      {
         auto*& mine = this->bound_tool.options;
         const auto* const& theirs = src.bound_tool.options;
         if (mine) {
            if (theirs) {
               *((worldedit::tools::options_union*)mine) = *((worldedit::tools::options_union*)theirs);
            } else {
               delete ((worldedit::tools::options_union*)mine);
               mine = nullptr;
            }
         } else if (theirs) {
            mine = ((worldedit::tools::options_union*)theirs)->clone();
         }
      }
      return *this;
   }
   bind_list_item& bind_list_item::operator=(bind_list_item&& src) noexcept {
      std::swap(this->name,               src.name);
      std::swap(this->conditions,         src.conditions);
      std::swap(this->button_press_type,  src.button_press_type);
      std::swap(this->input_sequence,     src.input_sequence);
      std::swap(this->bound_tool.tool,    src.bound_tool.tool);
      std::swap(this->bound_tool.options, src.bound_tool.options);
      return *this;
   }

   void bind_list_item::invoke(worldedit::tool_results_tuple& out, const tool_invocation_cause& cause) const {
      if (this->bound_tool.tool == worldedit::tools::id_of_none) {
         return;
      }
      assert(this->bound_tool.options != nullptr);

      auto& table = worldedit::tools::tool_dispatch_table[this->bound_tool.tool];
      table.invoke(cause, *this->bound_tool.options, out);
   }
   void bind_list_item::invoke_for_hold_release(worldedit::tool_results_tuple& out) const {
      if (this->bound_tool.tool == worldedit::tools::id_of_none) {
         return;
      }
      assert(this->bound_tool.options != nullptr);

      auto& table = worldedit::tools::tool_dispatch_table[this->bound_tool.tool];
      table.invoke_hold_release(*this->bound_tool.options, out);
   }
   #pragma endregion

   #pragma region bind_list
   void bind_list::update(timestamp_t now, worldedit::tool_results_tuple& press_results, worldedit::tool_results_tuple& hold_results) {
      auto& subsys = core::get(); // worldinput2
      devices::abstract_device_handler& device = subsys.device_by_type(this->device_type);

      auto current_conditions = control_scheme_condition::from_worldedit_state();

      std::vector<bind_list_item*> eligible_binds;
      std::vector<bind_list_item*> conflict_losing_binds;

      interruption_check interruption_check = device.prepare_interruption_check();

      #pragma region Find eligible nodes (and nodes that lose concurrent conflicts)
      for (auto& child : this->items) {
         bool matched = false;
         {
            auto& sequence = child.input_sequence;

            sequence.update(now, device, interruption_check);
            if (sequence.state.frame_status == input_sequence::frame_status::released) {
               switch (child.button_press_type) {
                  case button_press_type::press:
                     matched = true;
                     break;
                  case button_press_type::long_press:
                     {
                        auto down_for = elapsed_time(sequence.state.went_down_at, now);
                        if (down_for >= defaults::press_to_long_press_threshold)
                           matched = true;
                     }
                     break;
               }
            } else if (sequence.state.frame_status == input_sequence::frame_status::down) {
               switch (child.button_press_type) {
                  case button_press_type::hold:
                     matched = true;
                     break;
               }
            }

            if (!matched && child.button_press_type == button_press_type::hold) {
               child.state.press_blocked_hold = false;
            }

            if (matched && !sequence.range.is_satisfied(device))
               matched = false;
         }
         if (child.conditions.has_value()) {
            auto test = child.conditions.value() & current_conditions;
            if (test.impossible()) {
               matched = false;
            }
         }
         //
         if (!matched) {
            continue;
         }
         {
            eligible_binds.push_back(&child);
                  
            bool child_conflicted = false;
            {
               for (auto* prior : eligible_binds) {
                  bind_list_item* winning_node;
                  bool winner_is_not_blocked;
                  //
                  algorithms::concurrent_bind_conflict_resolution(
                     now,
                     child,
                     *prior,
                     winning_node,
                     winner_is_not_blocked
                  );

                  bind_list_item* losing_node = nullptr;
                  if (winning_node) {
                     losing_node = (winning_node == &child) ? prior : &child;
                     //
                     if (losing_node->button_press_type != button_press_type::hold) {
                        conflict_losing_binds.push_back(losing_node);
                     }
                     if (winning_node == &child) {
                        conflict_losing_binds.push_back(losing_node);
                        if (!winner_is_not_blocked) {
                           child_conflicted = true;
                        }
                     } else {
                        child_conflicted = true;
                     }
                  } else if (!winner_is_not_blocked) {
                     conflict_losing_binds.push_back(prior);
                     child_conflicted = true;
                  }
                  //
                  // We don't remove the conflict loser from `eligible_binds` because we want to 
                  // test all eligible binds against both each other and any previously designated 
                  // conflict losers. We'll remove the conflict losers later, after traversal and 
                  // conflict resolution are complete.
                  // 
                  // Hm... Maybe we should rename `eligible_binds` to `binds_under_consideration` 
                  // or `accumulated_binds`...
                  //
               }
            }
            //
            if (child_conflicted) {
               conflict_losing_binds.push_back(&child);
               continue;
            }
         }
      }
      #pragma endregion
      
      for (auto* conflicted : conflict_losing_binds) {
         eligible_binds.erase(
            std::remove_if(
               eligible_binds.begin(),
               eligible_binds.end(),
               [conflicted](auto* node) {
                  return node == conflicted;
               }
            ),
            eligible_binds.end()
         );
      }
      conflict_losing_binds.clear();

      #pragma region Press-preempts-Hold
      [this, now, &device, &eligible_binds](){
         struct hold_node_conflict_info {
            bind_list_item* node = nullptr;
            size_t outlasted  = 0;
            size_t delayed_by = 0;
            bool   is_advanced_past : 1 = false;
            bool   is_blocked       : 1 = false;
         };
         std::vector<hold_node_conflict_info> seen_hold_binds;
         std::vector<bind_list_item*> winning_press_binds;

         #pragma region Gather eligible Hold binds to test against, and prep state for them
         for (auto* bind : eligible_binds) {
            if (bind->button_press_type == button_press_type::hold) {
               auto& item = seen_hold_binds.emplace_back();
               item.node = bind;
               //
               if (bind->state.press_blocked_hold)
                  item.is_blocked = true;
            }
         }
         if (seen_hold_binds.empty())
            return;
         #pragma endregion

         #pragma region First pass to find all winning Press binds
         for (auto& press_bind : this->items) {
            if (press_bind.button_press_type == button_press_type::hold)
               continue;
            //
            for (auto& hold_info : seen_hold_binds) {
               auto result = algorithms::press_preempts_hold(
                  now,
                  device,
                  press_bind,
                  *hold_info.node
               );
               switch (result) {
                  using enum algorithms::press_preempt_hold_result;
                  case press_delays_hold:
                     ++hold_info.delayed_by;
                     winning_press_binds.push_back(&press_bind);
                     break;
                  case press_blocks_hold:
                     hold_info.is_blocked = true;
                     break;
                  case press_advanced_past_hold:
                     ++hold_info.delayed_by;
                     hold_info.is_advanced_past = true;
                     winning_press_binds.push_back(&press_bind);
                     break;
                  case hold_outlasted_press:
                     ++hold_info.outlasted;
                     break;
               }
            }
         }
         #pragma endregion

         #pragma region Identify Press binds that won one conflict but lost another
         {
            std::vector<bind_list_item*> retroamended_losing_presses;
            for (auto& hold_info : seen_hold_binds) {
               if (hold_info.is_blocked || hold_info.is_advanced_past)
                  continue;
               if (hold_info.outlasted == 0)
                  continue;
               hold_info.outlasted += hold_info.delayed_by;
               hold_info.delayed_by = 0;
               //
               for (auto* press_node : winning_press_binds) {
                  auto result = algorithms::press_preempts_hold(
                     now,
                     device,
                     *press_node,
                     *hold_info.node
                  );
                  switch (result) {
                     using enum algorithms::press_preempt_hold_result;
                     case press_delays_hold:
                     case hold_outlasted_press:
                        retroamended_losing_presses.push_back(press_node);
                        break;
                  }
               }
            }
            if (!retroamended_losing_presses.empty()) {
               winning_press_binds.erase(
                  std::remove_if(
                     winning_press_binds.begin(),
                     winning_press_binds.end(),
                     [&retroamended_losing_presses](const auto* node) -> bool {
                        return std::find(retroamended_losing_presses.begin(), retroamended_losing_presses.end(), node) != retroamended_losing_presses.end();
                     }
                  ),
                  winning_press_binds.end()
               );
            }
         }
         #pragma endregion

         #pragma region If a Press bind loses a conflict, then any Hold binds that lost to it retroactively "un-lose"
         for (auto& hold_info : seen_hold_binds) {
            if (hold_info.is_blocked)
               continue;
            if (hold_info.delayed_by == 0)
               continue;
            //
            bool any_loss = false;
            bool still_advanced_past = false;
            for (auto* press_node : winning_press_binds) {
               auto result = algorithms::press_preempts_hold(
                  now,
                  device,
                  *press_node,
                  *hold_info.node
               );
               switch (result) {
                  using enum algorithms::press_preempt_hold_result;
                  case press_delays_hold:
                     any_loss = true;
                     break;
                  case press_advanced_past_hold:
                     any_loss = true;
                     still_advanced_past = true;
                     break;
               }
            }
            if (!any_loss) {
               hold_info.delayed_by = 0;
               if (!still_advanced_past) {
                  hold_info.is_advanced_past = false;
               }
            }
         }
         #pragma endregion

         #pragma region All Hold binds that lost all conflicts they were in should cease to be eligible
         for (auto& hold_info : seen_hold_binds) {
            auto* node = hold_info.node;
            if (hold_info.delayed_by == 0 && !hold_info.is_blocked && !hold_info.is_advanced_past) {
               if (!node->state.press_blocked_hold)
                  continue;
            }
            eligible_binds.erase(
               std::remove_if(
                  eligible_binds.begin(),
                  eligible_binds.end(),
                  [node](auto* current) {
                     return current == node;
                  }
               ),
               eligible_binds.end()
            );
         }
         #pragma endregion
      }();
      #pragma endregion

      // Hold release.
      for (auto* hold_node : this->last_frame_active_hold_binds) {
         if (std::find(eligible_binds.begin(), eligible_binds.end(), hold_node) == eligible_binds.end()) {
            hold_node->invoke_for_hold_release(hold_results);
         }
      }

      // Conflict resolution: Holds block Presses.
      {
         //
         // We only want Hold nodes that are released to block Press and Long Press 
         // nodes (that are released, but that's implied by their being eligible). 
         // We can check whether each Hold node in this list is also eligible while 
         // we potentially disqualify the Press nodes, but it's easier to just pre-
         // filter this list instead.
         //
         auto& to_prune = this->last_frame_active_hold_binds;
         to_prune.erase(
            std::remove_if(
               to_prune.begin(),
               to_prune.end(),
               [this, &eligible_binds](const auto* node) -> bool {
                  return std::find(
                     eligible_binds.begin(),
                     eligible_binds.end(),
                     node
                  ) != eligible_binds.end();
               }
            ),
            to_prune.end()
         );
      }
      if (!this->last_frame_active_hold_binds.empty()) {
         eligible_binds.erase(
            std::remove_if(
               eligible_binds.begin(),
               eligible_binds.end(),
               [this, now](const auto* node) -> bool {
                  if (node->button_press_type == button_press_type::hold) {
                     return false;
                  }
                  for (auto* hold_node : this->last_frame_active_hold_binds) {
                     bool result = algorithms::hold_blocks_press(
                        *node,
                        *hold_node
                     );
                     if (result)
                        return true;
                  }
                  //
                  // No conflict.
                  //
                  return false;
               }
            ),
            eligible_binds.end()
         );
      }

      std::vector<bind_list_item*> this_frame_active_hold_binds;

      // Execution:
      for (auto* node : eligible_binds) {

         QPointF value    = { 0, 0 };
         bool    is_delta = false;
         bool    is_stale = false;
         if (node->input_sequence.has_range_requirement()) {
            auto& req = node->input_sequence.range;
            if (req.control != range_input_control::none) {
               is_delta = util::is_delta_control(req.control);
               value    = device.get_range_control_value(req.control, req.axes);
               if (is_delta)
                  is_stale = device.get_range_control_state(req.control, req.axes) == range_control_state::stale;
            }
         }
         if (!is_stale) {
            tool_invocation_cause cause;
            //
            cause.has_button = node->input_sequence.has_any_buttons();
            cause.has_range  = node->input_sequence.has_range_requirement();
            //
            cause.button.is_down    = node->button_press_type == button_press_type::hold;
            cause.button.down_when  = node->input_sequence.state.went_down_at;
            cause.button.press_type = node->button_press_type;
            {
               cause.button.down_state_changed_this_frame = node->input_sequence.state.frame_status_changed;
               if (node->button_press_type == button_press_type::hold) {
                  if (!cause.button.down_state_changed_this_frame) {
                     bool changed = true;
                     for (const auto* item : this->last_frame_active_hold_binds) {
                        if (item == node) {
                           changed = false;
                           break;
                        }
                     }
                     cause.button.down_state_changed_this_frame = changed;
                  }
               }
            }
            cause.range.x = value.x();
            cause.range.y = value.y();
            cause.range.is_delta = is_delta;

            if (node->input_sequence.has_raycast_requirement()) {
               cause.raycast = device.get_raycast_result(now, node->input_sequence.raycast.associated_button->button);
            }

            node->invoke((node->button_press_type == button_press_type::hold) ? hold_results : press_results, cause);
         }
         
         if (node->button_press_type == button_press_type::hold) {
            assert(node->input_sequence.state.frame_status == input_sequence::frame_status::down);
            this_frame_active_hold_binds.push_back(node);
         } else {
            assert(node->input_sequence.state.frame_status == input_sequence::frame_status::released);
            node->input_sequence.state.frame_status = input_sequence::frame_status::inactive;
         }
      }

      std::swap(this_frame_active_hold_binds, this->last_frame_active_hold_binds);
   }
   #pragma endregion
}