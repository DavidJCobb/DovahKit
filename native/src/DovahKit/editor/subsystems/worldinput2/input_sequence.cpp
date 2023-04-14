#include "./input_sequence.h"
#include <cassert>
#include "helpers/unreachable.h"
#include "./devices/abstract_device_handler.h"
#include "./defaults.h"
#include "./device_button_claim.h"

namespace dovahkit::subsystems::worldinput2 {
   #pragma region input_sequence::group
   input_sequence::group::~group() {
      for (auto* child : this->children)
         delete child;
      this->children.clear();
   }

   input_sequence::group_update_result input_sequence::group::update(timestamp_t current_time, timestamp_t last_advancement_time, devices::abstract_device_handler& device) {
      if (this->type == group_type::single_control) {
         const auto bs = device.get_state_of(this->button);
         //
         if (bs.was_released_this_frame()) {
            return group_update_result{
               .status = frame_status::released,
            };
         } else if (bs.is_down()) {
            return group_update_result{
               .down_at    = bs.down_when,
               .down_count = 1,
               .status    = frame_status::down,
            };
         }
         return group_update_result{
            .status = frame_status::inactive,
         };
      }

      if (this->type == group_type::concurrent_ordered) {
         auto   previous_timestamp = zero_timestamp;
         size_t count_down   = 0;
         bool   any_inactive = false;
         bool   any_released = false;
         size_t i;
         for (i = 0; i < this->children.size(); ++i) {
            auto* item   = this->children[i];
            auto  result = item->update(current_time, last_advancement_time, device);

            if (result.status != frame_status::released) {
               if (result.down_at < previous_timestamp) {
                  any_inactive = true;
                  break;
               }
            }
            
            if (result.status == frame_status::inactive) {
               any_inactive = true;
               break;
            } else if (result.status == frame_status::released) {
               any_released = true;
            } else if (result.status == frame_status::down) {
               previous_timestamp = result.down_at;
            }

            count_down += result.down_count;
         }
         if (any_inactive) {
            assert(i < this->children.size());
            for (i = 0; i < this->children.size(); ++i)
               this->children[i]->_clear_all_progress();
            return group_update_result{
               .down_count = count_down,
               .status     = frame_status::inactive,
            };
         }
         if (any_released) {
            return group_update_result{
               .down_count = count_down,
               .status     = frame_status::released,
            };
         }
         return group_update_result{
            .down_at    = previous_timestamp,
            .down_count = count_down,
            .status     = frame_status::down,
         };
      }

      if (this->type == group_type::concurrent_unordered) {
         auto   most_recently_down = zero_timestamp;
         size_t count_down   = 0;
         bool   any_inactive = false;
         bool   any_released = false;
         for (auto* item : this->children) {
            auto result = item->update(current_time, last_advancement_time, device);
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
                  .down_count = count_down,
                  .status     = frame_status::released,
               };
            }
            return group_update_result{
               .down_at    = most_recently_down,
               .down_count = count_down,
               .status     = frame_status::down,
            };
         }
         return group_update_result{
            .down_at    = most_recently_down,
            .down_count = count_down,
            .status     = frame_status::inactive,
         };
      }

      if (this->type == group_type::separated_ordered) {
         auto* current_item = this->children[this->state.current_item_index];
         if (this->state.current_item_index > 0) {
            auto elapsed = elapsed_time(last_advancement_time, current_time);
            if (elapsed >= dovahkit::subsystems::worldinput2::defaults::key_sequence_expire_time) {
               this->state.current_item_index = 0;
               return group_update_result{
                  .status = frame_status::inactive,
               };
            }
         }
         auto result = current_item->update(current_time, last_advancement_time, device);
         switch (result.status) {
            case frame_status::down:
               if (this->state.current_item_index == this->children.size() - 1) {
                  return result;
               }
               break;
            case frame_status::released:
               ++this->state.current_item_index;
               if (this->state.current_item_index == this->children.size()) {
                  this->state.current_item_index = 0;
                  return group_update_result{
                     .status = frame_status::released,
                  };
               }
               break;
         }
         return group_update_result{
            .down_at    = result.down_at,
            .down_count = result.down_count,
            .status     = frame_status::inactive,
         };
      }

      cobb::unreachable();
   }

   void input_sequence::group::run_interruption_check(interruption_check& check) const {
      /*//
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
      //*/
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
      this->state.current_item_index = 0;
      //
      for (auto* child : this->children)
         child->_clear_all_progress();
   }
   #pragma endregion

   #pragma region input_sequence
   void input_sequence::update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check& interruption_check) {
      if (!this->root)
         return;

      /*//
      if (this->run_interruption_check(interruption_check)) {
         this->clear_all_progress();
         return;
      }
      //*/

      auto prior  = this->state.frame_status;
      auto result = this->root->update(current_time, this->state.last_advancement, device);
      if (this->state.frame_status != result.status) {
         this->state.frame_status_changed = true;
         this->state.frame_status         = result.status;
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
                  if (claim.specificity > specificity) {
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
                  }
               }
            }
            break;
         default:
            cobb::unreachable();
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

   void input_sequence::clear_all_progress() {
      this->state.frame_status         = frame_status::inactive;
      this->state.frame_status_changed = false;
      this->state.went_down_at         = zero_timestamp;
      if (auto* g = this->root)
         g->_clear_all_progress();
   }

   namespace {
      size_t _group_concurrent_input_count(const input_sequence::group& self) {
         if (self.type == input_sequence::group_type::single_control) {
            if (self.button.is_modifier_key())
               return 0;
            return 1;
         }
         if (self.type == input_sequence::group_type::separated_ordered) {
            size_t count = 0;
            for (const auto* item : self.children) {
               auto cc = _group_concurrent_input_count(*item);
               if (cc > count)
                  count = cc;
            }
            return count;
         }
         size_t count = 0;
         for (const auto* item : self.children) {
            count += _group_concurrent_input_count(*item);
         }
         return count;
      }
   }
   bool input_sequence::is_probably_keyboard_impossible() const {
      //
      // Most keyboards can only register a limited number of simultaneously 
      // pressed keys; this count is called the "N-key rollover" for a given 
      // value of N. Typically, the limit is 2. Some USB keyboards can raise 
      // the limit to 6; for PS/2 connectors, there's no upper bound.
      // 
      // It's due to how keyboards are wired. Modifier keys generally don't 
      // contribute to the limit (else several Windows accelerator keys just 
      // wouldn't be possible), and sometimes the limit only applies to keys 
      // that are near each other; but the typical limit for non-modifier 
      // keys is 2.
      // 
      // As such, this function returns true if a given input sequence may 
      // not be possible to actually enter on a typical keyboard, due to 
      // requiring more than 2 non-modifier keys to be down concurrently.
      //
      if (!this->root)
         return false;
      size_t concurrent_input_count = _group_concurrent_input_count(*this->root);
      return (concurrent_input_count > 2);
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

   size_t input_sequence::specificity() const {
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
   /*static*/ input_sequence input_sequence::debug_from_string(const std::string& str, bool gamepad) {
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

            if (gamepad) {
               //
               // Gamepad
               //
               auto n = QString(name.c_str()).toLower();
               if (n == "a") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::a };
               } else if (n == "b") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::b };
               } else if (n == "x") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::x };
               } else if (n == "y") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::y };
               } else if (n == "lb") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::lb };
               } else if (n == "rb") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::rb };
               } else if (n == "ls") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::ls };
               } else if (n == "rs") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::rs };
               } else if (n == "start") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::start };
               } else if (n == "back") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::back };
               } else if (n == "d-pad up") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::d_pad_up };
               } else if (n == "d-pad left") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::d_pad_left };
               } else if (n == "d-pad right") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::d_pad_right };
               } else if (n == "d-pad down") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::d_pad_down };
               } else if (n == "lt") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::lt };
               } else if (n == "rt") {
                  child->button = inputs::button{ .gamepad = inputs::xinput_button::rt };
               } else {
                  qDebug("input_sequence::debug_from_string: unrecognized gamepad button name: %s", qUtf8Printable(n));
               }
            } else {
               //
               // Keyboard and mouse
               //
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
                  } else if (n == "lmb") {
                     child->button = inputs::button{ .key = cobb::qt::key::from_windows_vk(1) };
                  } else if (n == "rmb") {
                     child->button = inputs::button{ .key = cobb::qt::key::from_windows_vk(2) };
                  } else {
                     qDebug("input_sequence::debug_from_string: unrecognized key name: %s", qUtf8Printable(n));
                  }
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
