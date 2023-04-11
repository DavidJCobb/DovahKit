#include "./input_sequence.h"
#include <cassert>
#include "helpers/unreachable.h"
#include "./devices/abstract_device_handler.h"
#include "./defaults.h"

namespace dovahkit::subsystems::worldinput2 {
   #pragma region input_sequence::group
   input_sequence::group::~group() {
      for (auto* child : this->children)
         delete child;
      this->children.clear();
   }

   input_sequence::group_update_result input_sequence::group::update(timestamp_t current_time, timestamp_t last_advancement_time, devices::abstract_device_handler& device) {
      this->state.frame_status_changed = false;

      const auto prior_fs = this->state.frame_status;

      if (this->type == group_type::single_control) {
         const auto bs = device.get_state_of(this->button);
         //
         if (bs.was_released_this_frame()) {
            this->state.frame_status         = frame_status::released;
            this->state.frame_status_changed = true;
            return group_update_result::advancing;
         } else if (bs.is_down()) {
            this->state.frame_status         = frame_status::down;
            this->state.frame_status_changed = bs.was_pressed_this_frame();
            return group_update_result::advancing;
         } else {
            this->state.frame_status         = frame_status::inactive;
            this->state.frame_status_changed = (prior_fs != frame_status::inactive);
         }
         return group_update_result::no_change;
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
         std::vector<group*> terminals;
         if (this->is_concurrent()) {
            terminals = this->children;
         } else {
            if (!this->children.empty())
               terminals.push_back(this->children.back());
         }
         for (auto* item : terminals) {

            // An input sequence group can only be down if, on the last frame, all of its 
            // terminal items were also down.
            assert(item->state.frame_status == frame_status::down);

            auto result = item->update(current_time, last_advancement_time, device);
            assert(result != group_update_result::interrupted);
            if (item->state.frame_status == frame_status::released) {
               this->state.frame_status         = frame_status::released;
               this->state.frame_status_changed = true;
               return group_update_result::advancing;
            }
            assert(item->state.frame_status == frame_status::down);
         }
         assert(this->state.frame_status == frame_status::down);
         return group_update_result::advancing;
      }

      if (this->is_ordered()) {
         auto* current_item = this->children[this->state.current_item_index];
         if (this->type == group_type::concurrent_ordered) {
            //
            // Ensure no currently-down items have been released.
            //
            for (size_t i = 0; i < this->state.current_item_index; ++i) {
               auto* item = this->children[i];
               assert(item->state.frame_status == frame_status::down);

               auto result = item->update(current_time, last_advancement_time, device);
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
               auto elapsed = elapsed_time(last_advancement_time, current_time);
               if (elapsed >= dovahkit::subsystems::worldinput2::defaults::key_sequence_expire_time) {
                  this->state.frame_status         = frame_status::inactive;
                  this->state.frame_status_changed = true;
                  return group_update_result::interrupted;
               }
            }
         }

         auto result = current_item->update(current_time, last_advancement_time, device);
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
                     }
                  }
               }
               return group_update_result::advancing;
            case frame_status::released:
               if (this->type == group_type::separated_ordered) {
                  ++this->state.current_item_index;
               }
               if (this->state.current_item_index == this->children.size()) {
                  this->state.frame_status         = frame_status::released;
                  this->state.frame_status_changed = true;
               }
               return group_update_result::advancing;
         }

         return result;
      }

      assert(this->type == group_type::concurrent_unordered);
      {
         bool any_advancing = false;
         bool any_released  = false;
         bool any_down      = false;
         bool any_down_now  = false; // true if any went down on this frame specifically
         bool any_inactive  = false;
         for (auto* item : this->children) {
            auto result = item->update(current_time, last_advancement_time, device);
            if (result == group_update_result::interrupted) {
               this->state.frame_status         = frame_status::inactive;
               this->state.frame_status_changed = true;
               return result;
            }
            if (result == group_update_result::advancing) {
               any_advancing = true;
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
         return (any_advancing || any_down || any_released) ? group_update_result::advancing : group_update_result::no_change;
      }
      this->state.frame_status = frame_status::inactive;
      return group_update_result::no_change;
   }

   void input_sequence::group::run_interruption_check(interruption_check& check) const {
      //
      // This sub-algorithm runs recursively on some of the ISGs in an input sequence. It serves 
      // two purposes: it detects whether the user is currently in the middle of inputting a 
      // separate-and-ordered ISG (be it this group or one of its descendants); and it checks if 
      // any of the buttons (on the current device) that have gone down since the containing 
      // input sequence's last advancement time are buttons that would advance this ISG or any 
      // of its descendants.
      //
      if (this->type == group_type::single_control) {
         for (auto& item : check.buttons) {
            if (item.button == this->button) {
               item.matched = true;
               break;
            }
         }
         return;
      }
      if (this->type == group_type::concurrent_unordered) {
         const group* in_progress_separate_ordered_child = nullptr;

         for (const auto* item : this->children) {
            if (item->type != group_type::separated_ordered)
               continue;
            if (item->state.frame_status == frame_status::down)
               continue;
            if (item->state.current_item_index > 0) {
               in_progress_separate_ordered_child = item;
               break;
            }
         }

         if (!in_progress_separate_ordered_child) {
            for (const auto* item : this->children)
               if (item->type != group_type::separated_ordered)
                  item->run_interruption_check(check);
         } else {
            //
            // If a separate-and-ordered child is currently being entered, then the user is 
            // "locked in" to that child item; buttons exclusive to its siblings count as 
            // interruptions, so we shouldn't run this algorithm on those.
            //
            in_progress_separate_ordered_child->run_interruption_check(check);
         }
         return;
      }
      if (this->type == group_type::separated_ordered) {
         check.saw_separate_ordered_group = true;
      }
      if (this->state.current_item_index >= this->children.size()) {
         //
         // This can happen if this group's frame status is `down` as a result of 
         // the user fully progressing through this group's contents.
         //
         return;
      }
      auto* current_item = this->children[this->state.current_item_index];
      current_item->run_interruption_check(check);
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
   bool input_sequence::group::already_consumed(const devices::abstract_device_handler& device) const {
      std::vector<inputs::button> terminals;
      this->terminal_inputs(terminals);
      for (const auto& button : terminals) {
         if (!device.is_consumed(button))
            return false;
      }
      return true;
   }
   void input_sequence::group::terminal_inputs(std::vector<inputs::button>& append_to) const {
      if (this->type == group_type::single_control) {
         append_to.push_back(this->button);
         return;
      }
      for (const auto* item : this->children) {
         item->terminal_inputs(append_to);
      }
   }
   size_t input_sequence::group::descendant_count() const {
      if (this->type == group_type::single_control)
         return 0;
      size_t count = 0;
      for (const auto* item : this->children) {
         ++count;
         count += item->descendant_count();
      }
      return count;
   }
   size_t input_sequence::group::input_control_count() const {
      if (this->type == group_type::single_control)
         return 0;
      size_t count = 0;
      for (const auto* item : this->children) {
         if (item->type == group_type::single_control)
            ++count;
         else
            count += item->input_control_count();
      }
      return count;
   }
   bool input_sequence::group::is_or_contains_input_control(const inputs::button& button) const {
      if (this->type == group_type::single_control) {
         return this->button == button;
      }
      for (const auto* item : this->children) {
         if (item->is_or_contains_input_control(button))
            return true;
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
         if (this->button != other.button)
            return false;
      } else {
         size_t size = this->children.size();
         if (size != other.children.size())
            return false;
         for (size_t i = 0; i < size; ++i) {
            assert(this->children[i]);
            assert(other.children[i]);
            if (*this->children[i] != *other.children[i])
               return false;
         }
      }
      return true;
   }
   bool input_sequence::group::shallow_equals(const group& other) const {
      if (this->type != other.type)
         return false;
      if (this->type == group_type::single_control) {
         if (this->button != other.button)
            return false;
      } else {
         size_t size = this->children.size();
         if (size != other.children.size())
            return false;
      }
      return true;
   }
   bool input_sequence::group::is_superset_of(const group& other) const {
      if (this->type == group_type::single_control)
         return false;
      if (other.type == group_type::single_control) {
         for (auto* child : this->children)
            if (*child == other)
               return true;
      }
      if (this->type == other.type) {
         auto&  list_sup = this->children;
         auto&  list_sub = other.children;
         size_t size_sup = list_sup.size();
         size_t size_sub = list_sub.size();
         if (size_sup > size_sub) {
            //
            // It's possible that `this` is equal to `other` but with additional stuff 
            // added. What that actually means depends on whether the groups are ordered.
            //
            if (this->is_ordered()) {
               //
               // Given two ordered groups U and V and two arbitrary indices I and J: 
               // if the I-th element of U is equivalent to the (I + J)-th element in V 
               // for all possible values of I (i.e. any that are valid indices in U) 
               // and for any single value of J, then U is a subset of V.
               // 
               // In simpler terms: U is a subset of V if U's contents are equal, in 
               // data and ordering, to any subrange (with equivalent length) of V.
               //
               for (size_t i = 0; i < size_sup; ++i) {
                  bool shallow = list_sup[i]->shallow_equals(*list_sub[0]);
                  if (!shallow)
                     continue;
                  for (size_t j = 0; j < size_sub; ++j) {
                     shallow = list_sup[i + j]->shallow_equals(*list_sub[j]);
                     if (!shallow)
                        break;
                  }
                  if (!shallow)
                     continue;

                  bool deep = true;
                  for (size_t j = 0; j < size_sub; ++j) {
                     if (*list_sup[i + j] != *list_sub[j]) {
                        deep = false;
                        break;
                     }
                  }
                  if (deep)
                     return true;
               }
            } else {
               //
               // Given two unordered groups U and V: if for every element in U there 
               // is at least one equivalent element in V, then U is a subset of V.
               //
               std::vector<bool> contained;
               contained.resize(size_sub);
               //
               size_t contained_count = 0;

               for (const auto* child : this->children) {
                  for (size_t j = 0; j < size_sub; ++j) {
                     if (contained[j])
                        continue;
                     if (*child == *other.children[j]) {
                        contained[j] = true;
                        break;
                     }
                  }
                  if (++contained_count == size_sub) {
                     return true;
                  }
               }
            }
            //
         }
         //
         // If this group is not directly a superset of `other`, it may still contain 
         // a child group that is a superset of `other`, so fall through here.
         //
      }

      for (auto* child : this->children)
         if (child->is_superset_of(other))
            return true;

      return false;
   }

   void input_sequence::group::debug_stringify(std::string& out) const {
      if (this->type == group_type::single_control) {
         out += this->button.key.glyph.toStdString();
         return;
      }
      switch (this->type) {
         case group_type::concurrent_ordered:   out += '['; break;
         case group_type::concurrent_unordered: out += '('; break;
         case group_type::separated_ordered:    out += '<'; break;
      }
      for (const auto* child : this->children)
         child->debug_stringify(out);
      switch (this->type) {
         case group_type::concurrent_ordered:   out += ']'; break;
         case group_type::concurrent_unordered: out += ')'; break;
         case group_type::separated_ordered:    out += '>'; break;
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
      this->state.frame_status         = frame_status::inactive;
      this->state.frame_status_changed = false;
      this->state.current_item_index   = 0;
      //
      for (auto* child : this->children)
         child->_clear_all_progress();
   }
   #pragma endregion

   #pragma region input_sequence
   void input_sequence::update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check& interruption_check) {
      if (!this->root)
         return;

      if (this->run_interruption_check(interruption_check)) {
         this->clear_all_progress();
         return;
      }

      auto result = this->root->update(current_time, this->state.last_advancement, device);
      if (result == group_update_result::interrupted) {
         this->clear_all_progress();
         return;
      } else if (result == group_update_result::advancing) {
         this->state.last_advancement = current_time;
      }
      this->state.frame_status         = this->root->state.frame_status;
      this->state.frame_status_changed = this->root->state.frame_status_changed;
      if (this->state.frame_status == frame_status::down) {
         if (this->state.frame_status_changed) {
            this->state.went_down_at = current_time;
         }
      } else if (this->state.frame_status == frame_status::released) {
         auto down_at = this->state.went_down_at;

         this->clear_all_progress();
         if (this->root->already_consumed(device) == false) {
            this->state.frame_status         = frame_status::released;
            this->state.frame_status_changed = true;
            this->state.went_down_at = down_at;
            for (const auto& button : this->terminal_inputs()) {
               device.consume(button);
            }
         }
      }
   }

   bool input_sequence::run_interruption_check(interruption_check& check) const {
      check.saw_separate_ordered_group = false;
      check.start_at = 0;
      //
      bool found_first_button = false;
      for (size_t i = 0; i < check.buttons.size(); ++i) {
         auto& item = check.buttons[i];
         item.matched = false;
         if (!found_first_button) {
            if (item.down_at > this->state.last_advancement) {
               found_first_button = true;
               check.start_at = i;
            }
         }
      }

      if (this->root) {
         this->root->run_interruption_check(check);
      }
      if (check.saw_separate_ordered_group) {
         for (const auto& item : check.buttons)
            if (!item.matched)
               return true;
      }
      return false;
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
      this->state.went_down_at         = zero_timestamp;
      if (auto* g = this->root)
         g->_clear_all_progress();
   }

   std::vector<inputs::button> input_sequence::terminal_inputs() const {
      std::vector<inputs::button> out;
      if (this->root)
         this->root->terminal_inputs(out);
      return out;
   }

   const input_sequence::group* input_sequence::final_group() const {
      if (!this->root)
         return nullptr;
      return this->root->final_group();
   }

   bool input_sequence::is_subset_of(const input_sequence& other) const {
      if (!this->root || !other.root)
         return false;

      {  // Compare terminal inputs.
         auto term_sub = this->terminal_inputs();
         auto term_sup = other.terminal_inputs();
         if (term_sub.size() < term_sup.size()) {
            bool all = true;
            for (const auto& a : term_sub) {
               bool found = false;
               for (const auto& b : term_sup) {
                  if (a == b) {
                     found = true;
                     break;
                  }
               }
               if (!found)
                  all = false;
            }
            if (all)
               return true;
         }
      }

      // Compare full sequences:

      {
         // TODO: Once we switch to flat storage for ISG trees, we can check the total 
         //       number of ISGs (child and descendant) in a sequence by just checking 
         //       the length of the storage. This would allow us to early-out if the 
         //       `other` sequence is shorter than `this`.
      }
      if (this->root->is_superset_of(*other.root))
         return true;

      return false;
   }

   size_t input_sequence::total_group_count() const {
      if (this->root) {
         return 1 + this->root->descendant_count();
      }
      return 0;
   }
   size_t input_sequence::input_control_count() const {
      if (!this->root)
         return 0;
      if (this->root->type == group_type::single_control)
         return 1;
      return this->root->input_control_count();
   }

   input_sequence input_sequence::clone() const {
      //
      // In the future, I want to use flat storage for input sequences and their groups. When I do, 
      // cloning will become significantly easier. We won't need member functions on the groups, 
      // nor recursion; it will be enough to just copy the entire flat range, sans run-time state.
      //
      input_sequence out;
      out.root = this->root->_clone();
      return out;
   }

   input_sequence input_sequence::operator<<(const input_sequence& nested) const {
      return (this->clone() <<= nested);
   }
   input_sequence& input_sequence::operator<<=(const input_sequence& nested) {
      if (!nested.root)
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

   void input_sequence::debug_stringify(std::string& out) const {
      if (this->root)
         this->root->debug_stringify(out);
   }
   /*static*/ input_sequence input_sequence::debug_from_string(const std::string& str) {
      input_sequence out;

      std::vector<group*> nesting;

      size_t i = 0;
      for (; i < str.size(); ++i) {
         const char c = str[i];

         auto type  = group_type::single_control;
         bool close = false;

         switch (c) {
            case '[': type = group_type::concurrent_ordered;   break;
            case '(': type = group_type::concurrent_unordered; break;
            case '<': type = group_type::separated_ordered;    break;
            case ']': type = group_type::concurrent_ordered;   close = true; break;
            case ')': type = group_type::concurrent_unordered; close = true; break;
            case '>': type = group_type::separated_ordered;    close = true; break;
            case '+':
               qDebug("input_sequence::debug_from_string: unexpected + at position %d in: '%s'", i, str.c_str());
               [[fallthrough]];
            case ' ':
               continue;
         }
         if (close) {
            if (nesting.empty()) {
               qDebug("input_sequence::debug_from_string:: unexpected closing delimiter %c at position %d", c, i);
               __debugbreak();
            }
            nesting.pop_back();
            continue;
         }

         auto* child = new group;
         child->type = type;
         //
         if (nesting.empty()) {
            out.root = child;
         } else {
            auto* parent = nesting.back();
            assert(parent);
            assert(parent->type != group_type::single_control);

            parent->children.push_back(child);
         }

         if (type == group_type::single_control) {
            //
            // Advance to next ending delimiter, or to next '+'.
            //
            assert(c != ' ');
            std::string name;
            name += c;
            for (++i; i < str.size(); ++i) {
               const char d = str[i];
               if (d == '+') {
                  break;
               }
               if (d == ']' || d == ')' || d == '>') {
                  --i;
                  break;
               }
               name += d;
            }

            // trim trailing whitespace:
            size_t j;
            for (j = name.size() - 1; j > 0; --j)
               if (name[j] != ' ')
                  break;
            name.resize(j + 1);

            if (name.size() == 1) {
               child->button = inputs::button{ .key = cobb::qt::key(QChar(name[0])) };
            } else {
               auto n = QString(name.c_str()).toLower();
               if (n == "ctrl") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Control) };
               } else if (n == "alt") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Alt) };
               } else if (n == "shift") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Shift) };
               } else if (n == "prtscrn") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Print) };
               } else if (n == "caps lock") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_CapsLock) };
               } else if (n == "num lock") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_NumLock) };
               } else if (n == "scroll lock") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_ScrollLock) };
               } else if (n == "tab") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Tab) };
               } else if (n == "space") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Space) };
               } else if (n == "enter") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Enter) };
               } else if (n == "del" || n == "delete") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Delete) };
               } else if (n == "home") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_Home) };
               } else if (n == "end") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_End) };
               } else if (n == "page up") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_PageUp) };
               } else if (n == "page down") {
                  child->button = inputs::button{ .key = cobb::qt::key(Qt::Key::Key_PageDown) };
               } else {
                  qDebug("input_sequence::debug_from_string: unrecognize key name: %s", qUtf8Printable(n));
               }
            }
            continue;
         } else {
            nesting.push_back(child);
         }
      }

      return out;
   }
   #pragma endregion
}
