#include "./input_sequence.h"
#include <cassert>
#include <stdexcept>
#include "helpers/unreachable.h"
#include "./devices/abstract_device_handler.h"
#include "./config.h"
#include "./device_button_claim.h"
#include "./raycast_result.h"

namespace dovahkit::subsystems::worldinput {
   #pragma region input_sequence::range_requirement
   bool input_sequence::range_requirement::is_satisfied(const devices::abstract_device_handler& device) const {
      if (this->control == range_input_control::none)
         return true;

      range_control_state state = device.get_range_control_state(this->control, this->axes);

      if (state == range_control_state::unavailable)
         return false;

      return state != range_control_state::zeroed;
   }
   #pragma endregion

   #pragma region input_sequence::group
   input_sequence::group_update_result input_sequence::group::update(
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
      
      if (this->children.empty()) {
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
         for (i = 0; i < this->children.size(); ++i) {
            auto* item   = this->children[i];
            auto  result = item->update(params, previous_timestamp);

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
            for (i = i + 1; i < this->children.size(); ++i) {
               //
               // We don't clear the progress of the child that flagged as inactive, as that 
               // child may be e.g. a partially completed separate-and-ordered group, and in 
               // that case it's only inactive because it's not complete; we want to refrain 
               // from wiping its progress so that it *can* be completed.
               //
               this->children[i]->_clear_all_progress();
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
         for (auto* item : this->children) {
            auto result = item->update(params);

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
            this->_clear_all_progress();
            return group_update_result{
               .status = frame_status::inactive,
            };
         }
         //
         auto* current_item = this->children[this->state.current_item_index];
         auto  result       = current_item->update(params);
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
               if (this->state.current_item_index == this->children.size() - 1) {
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
               if (this->state.current_item_index == this->children.size()) {
                  this->_clear_all_progress();
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
                  if (elapsed >= dovahkit::subsystems::worldinput::config::key_sequence_expire_time()) {
                     this->_clear_all_progress();
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

   bool input_sequence::group::_is_separate_ordered_group_complete(interruption_check& check) const {
      if (this->type == input_sequence::group_type::single_control) {
         for (const auto& item : check.buttons)
            if (item.button == this->button)
               return true;
         return false;
      }
      if (this->type == input_sequence::group_type::separated_ordered) {
         if (this->state.current_item_index < this->children.size() - 1)
            return false;

         assert(this->state.current_item_index < this->children.size());
         return this->children.back()->_is_separate_ordered_group_complete(check);
      }
      for (const auto* child : this->children) {
         if (!child->_is_separate_ordered_group_complete(check))
            return false;
      }
      return true;
   }
   bool input_sequence::group::run_interruption_check(interruption_check& check) const {
      assert(this->type == group_type::separated_ordered);

      if (this->state.current_item_index == 0) {
         return false;
      }
      if (this->state.current_item_index == this->children.size() - 1) {
         //
         // We're on our last item. Is it already down? If so, then our separate and 
         // ordered group has been entered in full, so it can't be interrupted. (We 
         // need this check so that keys belonging to the group's next-sibling(s) don't 
         // count as retroactively "interrupting" it.)
         //
         if (this->children[this->state.current_item_index]->_is_separate_ordered_group_complete(check)) {
            return false;
         }
      }
      assert(this->state.current_item_index < this->children.size());

      for (size_t i = check.start_at; i < check.buttons.size(); ++i) {
         check.buttons[i].matched = false;
      }

      auto traverse = [this, &check](const group& current) {
         auto recurse = [&](const group& current, auto& recurse) -> void {
            if (current.type == group_type::single_control) {
               for (size_t i = check.start_at; i < check.buttons.size(); ++i) {
                  auto& item = check.buttons[i];
                  if (item.button == current.button) {
                     item.matched = true;
                     break;
                  }
               }
               return;
            }
            if (current.type == group_type::separated_ordered) {
               assert(current.state.current_item_index < current.children.size());
               
               auto* next = current.children[current.state.current_item_index];
               recurse(*next, recurse);
               return;
            }
            for (const auto* item : current.children) {
               recurse(*item, recurse);
            }
         };
         recurse(current, recurse);
      };
      traverse(*(this->children[this->state.current_item_index]));

      for (size_t i = check.start_at; i < check.buttons.size(); ++i) {
         if (!check.buttons[i].matched) {
            return true;
         }
      }
      return false;
   }

   const input_sequence::group* input_sequence::group::final_group() const {
      const group* out;
      const group* parent;
      this->find_final_group(out, parent);
      return out;
   }
   void input_sequence::group::find_final_group(const group*& out, const group*& out_parent) const {
      out        = nullptr;
      out_parent = nullptr;
      switch (this->type) {
         case group_type::single_control:
         case group_type::concurrent_unordered:
            out = this;
            break;
         case group_type::concurrent_ordered:
         case group_type::separated_ordered:
            if (this->children.empty())
               break;
            {
               auto* last = this->children.back();
               last->find_final_group(out, out_parent);
               if (out == last)
                  out_parent = this;
            }
            break;
         default:
            cobb::unreachable();
      }
   }

   bool input_sequence::group::operator==(const group& other) const {
      if (this->type != other.type)
         return false;

      if (this->type == group_type::single_control) {
         return (this->button == other.button);
      }

      size_t size = this->children.size();
      if (size != other.children.size()) {
         return false;
      }

      if (this->type == group_type::concurrent_unordered) {
         for (const auto* a : this->children) {
            assert(a != nullptr);
            bool found = false;
            for (const auto* b : other.children) {
               assert(b != nullptr);
               if (*a == *b) {
                  found = true;
                  break;
               }
            }
            if (!found)
               return false;
         }
      } else {
         for (size_t i = 0; i < size; ++i) {
            assert(this->children[i] != nullptr);
            assert(other.children[i] != nullptr);
            if (*this->children[i] != *other.children[i])
               return false;
         }
      }
      return true;
   }

   void input_sequence::group::normalize(bool recursively) {
      //
      // This function strips out duplicate and empty ISGs, but does not do anything 
      // about impossible ISGs.
      // 
      // TODO: Dupes/redundancies across hierarchy are not yet accounted for; for example, 
      // [A + [A + B]] is equivalent to [A + B] but won't be normalized as such.
      //
      if (this->type == group_type::single_control) {
         return;
      }

      auto& list = this->children;
      const auto size = list.size();
      if (size == 0) {
         //
         // This branch should only be reachable for the root ISG of an input sequence, 
         // presuming that ISG is not a single control yet is still empty and non-null.
         //
         return;
      }

      if (this->type == group_type::concurrent_ordered) {
         //
         // In a concurrent-and-ordered group, two consecutive and identical children 
         // act the same as a single child. For example, [A + A] becomes fully down 
         // after a single A press, because A is concurrent with itself, and these 
         // groups only require that each child be pressed concurrently with or after 
         // its previous sibling.
         // 
         // Identical but non-consecutive children would result in a group that is 
         // impossible to input. For example, [A + B + A] requires that A be pressed 
         // both before and after B is pressed, without ever being released.
         // 
         // Additionally, we remove empty ISGs if they are not single buttons.
         //
         std::vector<group*> to_remove;
         if (list[0]->can_have_children() && list[0]->children.empty()) {
            to_remove.push_back(list[0]);
         }
         for (size_t i = 1; i < size; ++i) {
            auto* a = list[i - 1];
            auto* b = list[i];
            if (*a == *b) {
               to_remove.push_back(b);
            } else {
               if (b->can_have_children() && b->children.empty()) {
                  to_remove.push_back(b);
               }
            }
         }
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [&to_remove](const auto* item) -> bool {
                  for (const auto* unwanted : to_remove)
                     if (unwanted == item)
                        return true;
                  return false;
               }
            ),
            list.end()
         );
         for (auto* unwanted : to_remove) {
            delete unwanted;
         }
      } else if (this->type == group_type::concurrent_unordered) {
         //
         // In a concurrent-and-unordered group, identical children anywhere in the 
         // list act the same as a single child. For example, (A + B + A) has the 
         // same behavior as (A + B).
         // 
         // Additionally, we remove empty ISGs if they are not single buttons.
         //
         std::vector<bool> to_remove;
         to_remove.resize(size);
         //
         for (size_t i = 0; i < size - 1; ++i) {
            auto* a = list[i];
            for (size_t j = i + 1; j < size; ++j) {
               auto* b = list[j];
               if (*a == *b) {
                  to_remove[j] = true;
               }
            }
            if (a->can_have_children() && a->children.empty()) {
               to_remove[i] = true;
            }
         }
         if (size > 1) {
            auto* b = list.back();
            if (b->can_have_children() && b->children.empty()) {
               to_remove.back() = true;
            }
         }
         //
         // Below, we delete duplicate groups *and* shift the list. It's basically 
         // the same approach taken by the erase-remove idiom, except 
         //
         size_t displaced = 0;
         for (size_t i = 1; i < size; ++i) {
            if (to_remove[i]) {
               delete list[i];
               list[i] = nullptr;
               ++displaced;
            } else if (displaced > 0) {
               list[i - displaced] = list[i];
            }
         }
         if (displaced > 0)
            list.resize(size - displaced);
      } else if (this->type == group_type::separated_ordered) {
         //
         // We remove any empty ISGs inside of this one.
         //
         bool any_deleted = false;
         for (size_t i = 0; i < size; ++i) {
            auto* item = list[i];
            if (item->can_have_children() && item->children.empty()) {
               delete item;
               list[i] = nullptr;
               any_deleted = true;
            }
         }
         if (any_deleted) {
            list.erase(
               std::remove(
                  list.begin(),
                  list.end(),
                  nullptr
               ),
               list.end()
            );
         }
      }
      //
      // At this point, `size` is potentially inaccurate, so we can't use it 
      // to iterate over `list`.
      //
      if (recursively) {
         for (auto* child : list)
            child->normalize(true);
      }
   }

   input_sequence::group* input_sequence::group::_clone() const {
      auto* out = new group;

      out->type = this->type;
      if (this->type == group_type::single_control)
         out->button = this->button;
      else {
         auto size = this->children.size();
         out->children.resize(size);
         for (size_t i = 0; i < size; ++i)
            out->children[i] = this->children[i]->_clone();
      }

      return out;
   }

   void input_sequence::group::_clear_all_progress() {
      this->state.current_item_index = 0;
      //
      for (auto* child : this->children)
         child->_clear_all_progress();
   }
   #pragma endregion

   #pragma region input_sequence
   void input_sequence::update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check& interruption_check) {
      if (!this->root || !this->has_any_buttons()) {
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

      if (this->state.raycast_success_flag) {
         auto* g = this->raycast.associated_button;
         assert(g);
         assert(g->type == group_type::single_control);
         auto state = device.get_state_of(g->button);
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
      auto result = this->root->update({
         .current_time          = current_time,
         .device                = device,
         .interruption_check    = interruption_check,
         .last_advancement_time = this->state.last_advancement,
         .raycast = {
            .already_passed    = this->state.raycast_success_flag,
            .associated_button = this->raycast.associated_button,
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

   void input_sequence::clear_all_progress() {
      this->state.frame_status         = frame_status::inactive;
      this->state.frame_status_changed = false;
      this->state.went_down_at         = zero_timestamp;
      this->state.raycast_success_flag = false;
      if (auto* g = this->root)
         g->_clear_all_progress();
   }

   const input_sequence::group* input_sequence::final_group() const {
      if (!this->root)
         return nullptr;
      return this->root->final_group();
   }

   input_sequence input_sequence::clone() const {
      //
      // In the future, I want to use flat storage for input sequences and their groups. When I do, 
      // cloning will become significantly easier. We won't need member functions on the groups, 
      // nor recursion; it will be enough to just copy the entire flat range, sans run-time state.
      //
      input_sequence out;
      if (this->root)
         out.root = this->root->_clone();
      out.range = this->range;

      out.raycast.requirement = this->raycast.requirement;
      if (this->raycast.associated_button) {
         auto recurse = [this, &out](const group* current_src, group* current_dst, auto& recurse) -> group* {
            if (current_src == this->raycast.associated_button) {
               return current_dst;
            }
            if (current_src->can_have_children()) {
               size_t size = current_src->children.size();
               for (size_t i = 0; i < size; ++i) {
                  auto* child_src = current_src->children[i];
                  auto* child_dst = current_dst->children[i];
                  if (auto* found = recurse(child_src, child_dst, recurse))
                     return found;
               }
            }
            return nullptr;
         };
         out.raycast.associated_button = recurse(this->root, out.root, recurse);
      }

      return out;
   }

   input_sequence& input_sequence::operator=(const input_sequence& other) {
      if (this == &other)
         return *this;
      if (this->root)
         delete this->root;
      if (other.root)
         this->root = other.root->_clone();
      else
         this->root = nullptr;

      this->range = other.range;

      this->raycast.requirement = other.raycast.requirement;
      if (other.raycast.associated_button) {
         auto recurse = [this, &other](const group* current_src, group* current_dst, auto& recurse) -> group* {
            if (current_src == other.raycast.associated_button) {
               return current_dst;
            }
            if (current_src->can_have_children()) {
               size_t size = current_src->children.size();
               for (size_t i = 0; i < size; ++i) {
                  auto* child_src = current_src->children[i];
                  auto* child_dst = current_dst->children[i];
                  if (auto* found = recurse(child_src, child_dst, recurse))
                     return found;
               }
            }
            return nullptr;
         };
         this->raycast.associated_button = recurse(other.root, this->root, recurse);
      } else {
         this->raycast.associated_button = nullptr;
      }

      this->state = other.state;

      return *this;
   }

   input_sequence input_sequence::operator<<(const input_sequence& nested) const {
      return (this->clone() <<= nested);
   }
   input_sequence& input_sequence::operator<<=(const input_sequence& nested) {
      if (this->has_range_requirement() && nested.has_range_requirement()) {
         throw std::logic_error(
            "An input sequence can only have one directional requirement; "
            "merging here would result in two. Ideally, only the 'leaf' "
            "sequence should have the requirement."
         );
      }
      if (nested.has_range_requirement()) {
         this->range = nested.range;
      }

      if (!nested.root)
         //
         // Can occur with empty sequences, or with range-input-only "sequences."
         //
         return *this;

      group* last_terminal;
      group* parent_of_last;
      this->root->find_final_group(last_terminal, parent_of_last);
      if (!last_terminal)
         return *this;

      group* wrap = nullptr;
      switch (last_terminal->type) {
         case group_type::single_control:
         case group_type::concurrent_unordered:
            if (parent_of_last && parent_of_last->is_ordered()) {
               //
               // The parent input sequence must be entered before the child input sequence 
               // can be entered; we join them using an ordered group. If the parent input 
               // sequence already ends with an ordered group, then we can append to it 
               // instead of creating a new (redundant) one.
               //
               if (nested.root->type == parent_of_last->type) {
                  //
                  // Example cases:
                  // 
                  //    [A + <B + C> + D] <<= [E + F]
                  //       == [A + (B + C) + D + E + F]
                  //       != [A + (B + C) + D + [E + F]]
                  // 
                  //    <A + B> <<= <C + D>
                  //       == <A + B + C + D>
                  //       != <A + B + <C + D>>
                  //
                  for (auto* child : nested.root->children) {
                     parent_of_last->children.push_back(child->_clone());
                  }
               } else {
                  //
                  // Example cases:
                  // 
                  //    [A + <B + C> + D] <<= <E + F>
                  //       == [A + <B + C> + D + <E + F>]
                  // 
                  //    <A + B> <<= [C + D]
                  //       == <A + B + [C + D]>
                  //
                  parent_of_last->children.push_back(nested.root->_clone());
               }
               return *this;
            } else {
               //
               // The parent input sequence does not end in an ordered group. As such, we 
               // must take its last terminal input (whatever that may be) and wrap that 
               // terminal input in a new ordered group.
               //
               wrap = new group;
               wrap->type = group_type::concurrent_ordered;
               wrap->children.push_back(last_terminal);
               wrap->children.push_back(nested.root->_clone());
            }
            break;
         case group_type::concurrent_ordered:
         case group_type::separated_ordered:
         default:
            cobb::unreachable();
      }
      if (wrap) {
         if (parent_of_last) {
            size_t i;
            for (i = 0; i < parent_of_last->children.size(); ++i) {
               if (parent_of_last->children[i] == last_terminal) {
                  break;
               }
            }
            assert(i < parent_of_last->children.size());
            parent_of_last->children[i] = wrap;
         } else {
            assert(last_terminal == this->root);
            this->root = wrap;
         }
      }

      return *this;
   }

   void input_sequence::normalize() {
      if (this->root) {
         this->root->normalize(true);
         if (this->root->can_have_children() && this->root->children.empty()) {
            delete this->root;
            this->root = nullptr;
         }
      }
   }

   void input_sequence::assert_validity() const {
      if (auto* isg = this->raycast.associated_button) {
         assert(isg->type == group_type::single_control);
      }
      if (this->root) {
         auto per_group = []<typename Self>(this Self&& recurse, group& current) -> void {
            switch (current.type) {
               case group_type::single_control:
                  assert(current.children.empty());
                  break;
               default:
                  for (auto* child : current.children) {
                     assert(child != nullptr);
                     recurse(*child);
                  }
            }
         };
         per_group(*this->root);
      }
   }
   #pragma endregion
}
