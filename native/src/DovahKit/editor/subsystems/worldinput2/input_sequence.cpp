#include "./input_sequence.h"
#include <cassert>
#include "./devices/abstract_device_handler.h"
#include "./defaults.h"

namespace dovahkit::subsystems::worldinput2 {
   #pragma region input_sequence::group
   input_sequence::group_update_result input_sequence::group::update(timestamp_t current_time, devices::abstract_device_handler& device) {
      this->state.frame_status_changed = false;

      const auto prior_fs = this->state.frame_status;

      if (this->type == group_type::single_control) {
         const auto bs = device.get_state_of(this->button);
         //
         if (bs.was_released_this_frame()) {
            this->state.frame_status         = frame_status::released;
            this->state.frame_status_changed = true;
         } else if (bs.is_down()) {
            this->state.frame_status         = frame_status::down;
            this->state.frame_status_changed = bs.was_pressed_this_frame();
         } else {
            this->state.frame_status         = frame_status::inactive;
            this->state.frame_status_changed = (prior_fs != frame_status::inactive);
         }
         return group_update_result::not_interrupted;
      }

      // If a top-level group's frame status becomes released, then that should cause the 
      // containing input sequence to flag as released -- and to reset the states on all of
      // its contained groups. If a nested group's frame status becomes released, then that 
      // should be handled by its parent group as either interrupting an incomplete input 
      // sequence, or as releasing a currently-down input sequence; this should cause the 
      // containing groups, all the way up to the top-level one, to either become released 
      // or flag as interrupted, causing the containing input sequence to reset them all.
      //
      // In short: the frame status should never remain released across frames; it should 
      // always end up being reset to inactive on the same frame that it appeared.
      assert(this->state.frame_status != frame_status::released);

      if (this->state.frame_status == frame_status::down) {
         //
         // Detect when a currently-down group is released.
         //
         auto terminals = this->terminal_items();
         for (auto* item : terminals) {

            // An input sequence group can only be down if, on the last frame, all of its 
            // terminal items were also down.
            assert(item->state.frame_status == frame_status::down);

            auto result = item->update(current_time, device);
            assert(result != group_update_result::interrupted);
            if (item->state.frame_status == frame_status::released) {
               this->state.frame_status         = frame_status::released;
               this->state.frame_status_changed = true;
               return group_update_result::not_interrupted;
            }
            assert(item->state.frame_status == frame_status::down);
         }
         assert(this->state.frame_status == frame_status::down);
      }

      if (this->is_ordered()) {
         auto* current_item = this->children[this->state.current_item_index];
         static_assert(false, "TODO: Check if interrupted by keys not part of the current bind (maybe only for separate-and-ordered?");
         if (this->type == group_type::concurrent_ordered) {
            //
            // Ensure no currently-down items have been released.
            //
            for (size_t i = 0; i < this->state.current_item_index; ++i) {
               auto* item = this->children[i];
               assert(item->state.frame_status == frame_status::down);

               auto result = item->update(current_time, device);
               assert(result != group_update_result::interrupted);
               //
               // A result of interrupted should be impossible here: once an input sequence 
               // group is down, the update algorithm only cares about whether it's released, 
               // and so should not allow it to become interrupted.

               if (item->state.frame_status == frame_status::released) {
                  this->state.frame_status         = frame_status::inactive;
                  this->state.frame_status_changed = true;
                  return group_update_result::interrupted;
               }
               assert(item->state.frame_status == frame_status::down);
            }
         } else {
            assert(this->type == group_type::separated_ordered);
            //
            // For separated-and-ordered groups, there should be a limit on how much time can 
            // elapse between keypresses. Take too long to input the next keypress, and we 
            // should consider that as the sequence being interrupted.
            //
            if (this->state.current_item_index > 0) {
               auto elapsed = elapsed_time(this->state.last_advancement, current_time);
               if (elapsed >= dovahkit::subsystems::worldinput2::defaults::key_sequence_expire_time) {
                  this->state.frame_status         = frame_status::inactive;
                  this->state.frame_status_changed = true;
                  return group_update_result::interrupted;
               }
            }
         }

         auto result = current_item->update(current_time, device);
         if (result == group_update_result::interrupted) {
            this->state.frame_status         = frame_status::inactive;
            this->state.frame_status_changed = true;
            return result;
         }

         switch (current_item->state.frame_status) {
            case frame_status::down:
               if (current_item->state.frame_status_changed) {
                  if (this->type == group_type::concurrent_ordered) {
                     ++this->state.current_item_index;
                     if (this->state.current_item_index == this->children.size()) {
                        this->state.frame_status         = frame_status::down;
                        this->state.frame_status_changed = true;
                        return group_update_result::not_interrupted;
                     }
                  }
               }
               break;
            case frame_status::released:
               if (this->type == group_type::separated_ordered) {
                  ++this->state.current_item_index;
                  this->state.last_advancement = current_time;
               }
               if (this->state.current_item_index == this->children.size()) {
                  this->state.frame_status         = frame_status::released;
                  this->state.frame_status_changed = true;
                  return group_update_result::not_interrupted;
               }
               break;
         }

         return group_update_result::not_interrupted;
      }

      assert(this->type == group_type::concurrent_unordered);
      {
         bool any_released = false;
         bool any_down     = false;
         bool any_down_now = false; // true if any went down on this frame specifically
         bool any_inactive = false;
         for (auto* item : this->children) {
            auto result = item->update(current_time, device);
            if (result == group_update_result::interrupted) {
               this->state.frame_status         = frame_status::inactive;
               this->state.frame_status_changed = true;
               return result;
            }
            switch (item->state.frame_status) {
               case frame_status::released:
                  any_released = true;
                  break;
               case frame_status::down:
                  any_down = true;
                  if (item->state.frame_status_changed)
                     any_down_now = true;
                  break;
               case frame_status::inactive:
                  any_inactive = true;
                  break;
            }
            if (any_released && any_inactive) {
               //
               // This happens if a concurrent-and-ordered group had some keys released before 
               // all of them went down, interrupting the bind.
               //
               this->state.frame_status = frame_status::inactive;
               assert(this->state.frame_status_changed == false);
               return group_update_result::interrupted;
            }
         }
         if (any_inactive == false) {
            assert(any_down == true || any_released == true); // All items are either down or released.
            if (any_released) {
               this->state.frame_status         = frame_status::released;
               this->state.frame_status_changed = true;
            } else {
               //
               // If `any_down_now` is true, then -- by virtue of the logic for all of this -- 
               // the group's frame status will have been a value other than `down` before we 
               // set it to `down` here. We could potentially omit the `any_down_now` bool if 
               // we instead check whether the prior frame status value was `down`, which could 
               // be done in a setter for `frame_status` if we implement one.
               //
               this->state.frame_status         = frame_status::down;
               this->state.frame_status_changed = any_down_now;
            }
         }
         return group_update_result::not_interrupted;
      }
      this->state.frame_status = frame_status::inactive;
      return group_update_result::not_interrupted;
   }

   bool input_sequence::group::all_contents_inactive() const {
      for (auto* item : this->children) {
         if (item->state.frame_status != frame_status::inactive)
            return false;
         if (!item->all_contents_inactive())
            return false;
      }
      return true;
   }

   void input_sequence::group::_clear_all_progress() {
      this->state.frame_status         = frame_status::inactive;
      this->state.frame_status_changed = false;
      this->state.current_item_index   = 0;
      this->state.last_advancement     = zero_timestamp;
      //
      for (auto* child : this->children)
         child->_clear_all_progress();
   }
   #pragma endregion

   #pragma region input_sequence
   void input_sequence::update(timestamp_t current_time, devices::abstract_device_handler& device) {
      if (!this->root)
         return;

      auto result = this->root->update(current_time, device);
      if (result == group_update_result::interrupted) {
         this->clear_all_progress();
         return;
      }
      this->state.frame_status = this->root->state.frame_status;
      if (this->state.frame_status == frame_status::released) {
         this->clear_all_progress();
         if (this->root->already_consumed() == false) {
            this->state.frame_status = frame_status::released;
            for (const auto& button : this->root->terminal_inputs()) {
               device.consume(button);
            }
         }
      }
   }

   bool input_sequence::all_contents_inactive() const {
      if (!this->root)
         return true;
      if (this->root->state.frame_status != frame_status::inactive)
         return false;
      return this->root->all_contents_inactive();
   }
   void input_sequence::clear_all_progress() {
      this->state.frame_status         = frame_status::inactive;
      this->state.frame_status_changed = false;
      if (auto* g = this->root)
         g->_clear_all_progress();
   }
   #pragma endregion
}
