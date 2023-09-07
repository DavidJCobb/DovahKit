#include "./input_sequence_flat.h"
#include "./devices/abstract_device_handler.h"
#include "./defaults.h"
#include "./device_button_claim.h"

namespace dovahkit::subsystems::worldinput2 {
   #pragma region input_sequence_flat::group
   input_sequence_flat::group_update_result input_sequence_flat::group::update(
      const group_update_params params,
      timestamp_t previous_sibling_time
   ) {
      const auto current_time          = params.current_time;
      auto&      device                = params.device;
      auto&      interruption_check    = params.interruption_check;
      const auto last_advancement_time = params.last_advancement_time;

      if (this->type == group_type::single_control) {
         const auto bs = device.get_state_of(this->button);
         //
         if (bs.was_released_this_frame()) {
            return group_update_result{
               .down_at    = bs.down_when,
               .down_count = 0,
               .status     = frame_status::released,
            };
         } else if (bs.is_down()) {

            if (this == params.raycast.associated_button) {
               const auto& requirement = params.raycast.requirement;
               //
               if (bs.was_pressed_this_frame()) {
                  const auto& res = device.get_raycast_result(current_time, this->button);
                  if (!requirement.is_satisfied_by(res)) {
                     return group_update_result{
                        .status  = frame_status::inactive,
                        .raycast = raycast_status::failed,
                     };
                  }
                  return group_update_result{
                     .down_at    = bs.down_when,
                     .down_count = 1,
                     .status     = frame_status::down,
                     //
                     .raycast = raycast_status::passed,
                  };
               } else if (params.raycast.already_passed) {
                  const bool per_frame = requirement.timing == raycast_requirement::timing_type::per_frame;
                  const bool no_change = requirement.fail_if_target_changes;
                  if (per_frame || no_change) {
                     const auto& res_p = device.get_per_frame_raycast_result();
                     if (per_frame) {
                        if (!requirement.is_satisfied_by(res_p)) {
                           return group_update_result{
                              .status  = frame_status::inactive,
                              .raycast = raycast_status::failed,
                           };
                        }
                     }
                     if (no_change) {
                        const auto& res_b = device.get_raycast_result(current_time, this->button);
                        if (!res_b.same_target_as(res_p)) {
                           return group_update_result{
                              .status  = frame_status::inactive,
                              .raycast = raycast_status::failed,
                           };
                        }
                     }
                  }
               }
            }
            return group_update_result{
               .down_at    = bs.down_when,
               .down_count = 1,
               .status     = frame_status::down,
               //
               .raycast = raycast_status::unaffected,
            };
         }
         return group_update_result{
            .status = frame_status::inactive,
         };
      }
      
      if (this->children().empty()) {
         return group_update_result{
            .down_at    = zero_timestamp,
            .down_count = 0,
            .status     = frame_status::down,
         };
      }

      if (this->type == group_type::concurrent_ordered) {
         raycast_status raycast_satisfied = raycast_status::unaffected;
         auto   previous_timestamp = previous_sibling_time;
         size_t count_down   = 0;
         bool   any_inactive = false;
         bool   any_released = false;
         size_t i;
         for (i = 0; i < this->children().size(); ++i) {
            auto& item   = this->children()[i];
            auto  result = item.update(params, previous_timestamp);

            if (result.raycast != raycast_status::unaffected)
               raycast_satisfied = result.raycast;

            if (result.status != frame_status::released) {
               if (result.down_at < previous_timestamp) {
                  any_inactive = true;
                  break;
               }
            }
            
            if (result.status == frame_status::inactive) {
               any_inactive = true;
               //
               if (result.down_at > previous_timestamp) // ignore `result.down_at` if it's a zero timestamp
                  previous_timestamp = result.down_at;
               count_down += result.down_count;
               //
               break;
            } else if (result.status == frame_status::released) {
               if (result.down_at != zero_timestamp && result.down_at < previous_timestamp) {
                  any_inactive = true;
                  break;
               }
               any_released       = true;
               previous_timestamp = result.down_at;
            } else if (result.status == frame_status::down) {
               previous_timestamp = result.down_at;
            }

            count_down += result.down_count;
         }
         if (any_inactive) {
            for (i = i + 1; i < this->children().size(); ++i) {
               //
               // We don't clear the progress of the child that flagged as inactive, as that 
               // child may be e.g. a partially completed separate-and-ordered group, and in 
               // that case it's only inactive because it's not complete; we want to refrain 
               // from wiping its progress so that it *can* be completed.
               //
               this->children()[i].clear_all_progress();
            }
            return group_update_result{
               .down_at    = previous_timestamp,
               .down_count = count_down,
               .status     = frame_status::inactive,
               .raycast    = raycast_satisfied,
            };
         }
         if (any_released) {
            return group_update_result{
               .down_at    = previous_timestamp,
               .down_count = count_down,
               .status     = frame_status::released,
               .raycast    = raycast_satisfied,
            };
         }
         return group_update_result{
            .down_at    = previous_timestamp,
            .down_count = count_down,
            .status     = frame_status::down,
            .raycast    = raycast_satisfied,
         };
      }

      if (this->type == group_type::concurrent_unordered) {
         raycast_status raycast_satisfied = raycast_status::unaffected;
         auto   most_recently_down = zero_timestamp;
         size_t count_down   = 0;
         bool   any_inactive = false;
         bool   any_released = false;
         for (auto& item : this->children()) {
            auto result = item.update(params);

            if (result.raycast != raycast_status::unaffected)
               raycast_satisfied = result.raycast;

            count_down += result.down_count;
            if (result.down_at > most_recently_down)
               most_recently_down = result.down_at;
            switch (result.status) {
               case frame_status::inactive:
                  any_inactive = true;
                  break;
               case frame_status::released:
                  any_released = true;
                  break;
            }
         }
         if (!any_inactive) {
            if (any_released) {
               return group_update_result{
                  .down_at    = most_recently_down,
                  .down_count = count_down,
                  .status     = frame_status::released,
                  .raycast    = raycast_satisfied,
               };
            }
            return group_update_result{
               .down_at    = most_recently_down,
               .down_count = count_down,
               .status     = frame_status::down,
               .raycast    = raycast_satisfied,
            };
         }
         return group_update_result{
            .down_at    = most_recently_down,
            .down_count = count_down,
            .status     = frame_status::inactive,
            .raycast    = raycast_satisfied,
         };
      }

      if (this->type == group_type::separated_ordered) {
         if (this->run_interruption_check(interruption_check)) {
            this->clear_all_progress();
            return group_update_result{
               .status = frame_status::inactive,
            };
         }
         //
         auto& current_item = this->children()[this->state.current_item_index];
         auto  result       = current_item.update(params);
         if (this->state.current_item_index == 0) {
            if (result.down_at < previous_sibling_time) {
               return group_update_result{
                  .down_at    = zero_timestamp,
                  .down_count = 0,
                  .status     = frame_status::inactive,
               };
            }
         }
         switch (result.status) {
            case frame_status::down:
               if (this->state.current_item_index == this->children().size() - 1) {
                  return group_update_result{
                     .down_at    = current_time,
                     .down_count = result.down_count,
                     .status     = frame_status::down,
                     .raycast    = result.raycast,
                  };
               }
               return group_update_result{
                  .down_at    = current_time,
                  .down_count = result.down_count,
                  .status     = frame_status::inactive,
                  .raycast    = result.raycast,
               };
               break;
            case frame_status::released:
               ++this->state.current_item_index;
               if (this->state.current_item_index == this->children().size()) {
                  this->clear_all_progress();
                  return group_update_result{
                     .down_at = result.down_at,
                     .status  = frame_status::released,
                     .raycast = result.raycast,
                  };
               } else {
                  return group_update_result{
                     .down_at    = current_time,
                     .down_count = result.down_count,
                     .status     = frame_status::inactive,
                     .raycast    = result.raycast,
                  };
               }
               break;
            case frame_status::inactive:
               //
               // Enforce the maximum time to progress the sequence.
               //
               if (this->state.current_item_index > 0) {
                  auto time = last_advancement_time;
                  if (result.down_at > time)
                     time = result.down_at;
                  auto elapsed = elapsed_time(time, current_time);
                  if (elapsed >= dovahkit::subsystems::worldinput2::defaults::key_sequence_expire_time) {
                     this->clear_all_progress();
                     return group_update_result{
                        .status = frame_status::inactive,
                     };
                  }
               } else {
                  return group_update_result{
                     .down_at    = result.down_at,
                     .down_count = result.down_count,
                     .status     = frame_status::inactive,
                     .raycast    = result.raycast,
                  };
               }
               break;
         }
         return group_update_result{
            .down_at    = result.down_at > last_advancement_time ? result.down_at : last_advancement_time,
            .down_count = result.down_count,
            .status     = frame_status::inactive,
            .raycast    = result.raycast,
         };
      }

      cobb::unreachable();
   }
   #pragma endregion

   #pragma region input_sequence_flat
   void input_sequence_flat::update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check& interruption_check) {
      auto* root = this->root();

      if (!root || !this->has_any_buttons()) {
         if (this->has_range_requirement()) {
            //
            // By making this exception, we allow the user to bind actions directly to 
            // range constraints, e.g. binding "Turn Camera" to an Xbox controller's 
            // right stick without the need for any buttons to be pressed. Of course, 
            // this only actually works if the button press type used for the bind is 
            // Hold.
            //
            this->state.frame_status         = frame_status::down;
            this->state.frame_status_changed = false;
         }
         return;
      }

      const group* raycast_associated_button = nullptr;
      if (this->raycast.associated_button != index_of_none) {
         assert(this->raycast.associated_button < this->contents.size());
         raycast_associated_button = &(this->contents[this->raycast.associated_button]);
      }

      if (this->state.raycast_success_flag) {
         assert(raycast_associated_button);
         assert(raycast_associated_button->type == group_type::single_control);
         auto state = device.get_state_of(raycast_associated_button->button);
         if (!state.is_down() && !(state.flags & device_button_state::flag::down_state_changed_on_this_frame)) {
            this->state.raycast_success_flag = false;
         }
      }

      {
         interruption_check.start_at = 0;
         //
         bool found_first_button = false;
         for (size_t i = 0; i < interruption_check.buttons.size(); ++i) {
            auto& item = interruption_check.buttons[i];
            if (item.down_at > this->state.last_advancement) {
               found_first_button = true;
               interruption_check.start_at = i;
               break;
            }
         }
         if (!found_first_button) {
            //
            // If *all* of the buttons were down before our last advancement time, 
            // then we want to skip all of them.
            //
            interruption_check.start_at = interruption_check.buttons.size();
         }
      }

      auto prior  = this->state.frame_status;
      auto result = root->update({
         .current_time          = current_time,
         .device                = device,
         .interruption_check    = interruption_check,
         .last_advancement_time = this->state.last_advancement,
         .raycast = {
            .already_passed    = this->state.raycast_success_flag,
            .associated_button = raycast_associated_button,
            .requirement       = this->raycast.requirement,
         }
      });

      if (this->has_raycast_requirement()) {
         if (result.raycast == raycast_status::passed)
            this->state.raycast_success_flag = true;
         else if (result.raycast == raycast_status::failed) {
            this->state.raycast_success_flag = false;
            if (result.status == frame_status::released) {
               this->clear_all_progress();
            }
            result.status = frame_status::inactive;
         } else if (result.raycast == raycast_status::unaffected) {
            if (!this->state.raycast_success_flag) {
               result.status = frame_status::inactive;
            }
         }
      }

      if (this->state.frame_status != result.status) {
         this->state.frame_status_changed = true;
         this->state.frame_status         = result.status;
      } else {
         this->state.frame_status_changed = false;
      }
      switch (result.status) {
         case frame_status::inactive:
            this->state.last_advancement = result.down_at;
            break;
         case frame_status::down:
            this->state.last_advancement = result.down_at;
            if (this->state.frame_status_changed) {
               this->state.went_down_at = result.down_at;
            }
            break;
         case frame_status::released:
            {
               auto down_at = this->state.went_down_at;

               auto specificity = this->specificity();
               auto terminals   = this->terminal_inputs();
               bool consumed_by_more_specific_sequence = false;

               this->clear_all_progress();

               for (const auto& button : terminals) {
                  const auto& claim = device.get_existing_claim_of(button);
                  if (claim.specificity >= specificity) {
                     if (claim.when > down_at) {
                        consumed_by_more_specific_sequence = true;
                        break;
                     }
                  }
               }
               if (!consumed_by_more_specific_sequence) {
                  this->state.frame_status         = frame_status::released; // clear_all_progress reset this earlier
                  this->state.frame_status_changed = true;
                  this->state.went_down_at         = down_at;
                  for (const auto& button : terminals) {
                     auto& claim = device.get_pending_claim_of(button);
                     claim.attempt_new_claim(current_time, specificity);
                     //
                     // It's worth noting here that claims only get applied to an input 
                     // control if it's still down on the frame after the claim is made, 
                     // so the terminal inputs that were released on this frame will not 
                     // be claimed. This prevents an input sequence from blocking itself, 
                     // and that fact in turn allows you to re-trigger any typical button 
                     // combination by releasing and re-pressing a terminal input without 
                     // having to re-enter the whole input sequence.
                     //
                  }
               }
            }
            break;
         default:
            cobb::unreachable();
      }
   }
   #pragma endregion
}