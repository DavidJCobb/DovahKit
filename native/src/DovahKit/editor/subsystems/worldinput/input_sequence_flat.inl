#pragma once
#include "./input_sequence_flat.h"
#include "helpers/unreachable.h"

namespace dovahkit::subsystems::worldinput2 {
   #pragma region input_sequence_flat::group
   constexpr bool input_sequence_flat::group::can_have_children() const noexcept {
      switch (this->type) {
         case group_type::single_control:
            return false;
      }
      return true;
   }
   constexpr bool input_sequence_flat::group::is_concurrent() const noexcept {
      switch (this->type) {
         case group_type::concurrent_ordered:
         case group_type::concurrent_unordered:
            return true;
      }
      return false;
   }
   constexpr bool input_sequence_flat::group::is_ordered() const noexcept {
      switch (this->type) {
         case group_type::concurrent_ordered:
         case group_type::separated_ordered:
            return true;
      }
      return false;
   }

   constexpr const input_sequence_flat::group* input_sequence_flat::group::final_group() const {
      const group* out;
      const group* parent;
      this->find_final_group(out, parent);
      return out;
   }
   constexpr void input_sequence_flat::group::find_final_group(const group*& out, const group*& out_parent) const {
      out        = nullptr;
      out_parent = nullptr;
      switch (this->type) {
         case group_type::single_control:
         case group_type::concurrent_unordered:
            out = this;
            break;
         case group_type::concurrent_ordered:
         case group_type::separated_ordered:
            if (this->children().empty())
               break;
            {
               auto& last = this->children().back();
               last.find_final_group(out, out_parent);
               if (out == &last)
                  out_parent = this;
            }
            break;
         default:
            cobb::unreachable();
      }
   }

   constexpr void input_sequence_flat::group::terminal_inputs(std::vector<inputs::button>& append_to) const {
      if (this->type == group_type::single_control) {
         append_to.push_back(this->button);
         return;
      }
      if (this->type == group_type::separated_ordered) {
         if (this->children().empty())
            return;
         this->children().back().terminal_inputs(append_to);
         return;
      }
      for (const auto& item : this->children()) {
         item.terminal_inputs(append_to);
      }
   }

   constexpr const input_sequence_flat::group& input_sequence_flat::group::current_item() const {
      if (std::is_constant_evaluated()) {
         if (this->type != group_type::separated_ordered)
            throw;
      }
      return this->children()[this->state.current_item_index];
   }

      #pragma region Algorithms
      constexpr bool input_sequence_flat::group::_is_separate_ordered_group_complete(interruption_check& check) const {
         if (this->type == input_sequence::group_type::single_control) {
            for (const auto& item : check.buttons)
               if (item.button == this->button)
                  return true;
            return false;
         }
         if (this->type == input_sequence::group_type::separated_ordered) {
            if (this->state.current_item_index < this->children().size() - 1)
               return false;

            assert(this->state.current_item_index < this->children().size());
            return this->children().back()._is_separate_ordered_group_complete(check);
         }
         for (const auto& child : this->children()) {
            if (!child._is_separate_ordered_group_complete(check))
               return false;
         }
         return true;
      }
      constexpr bool input_sequence_flat::group::run_interruption_check(interruption_check& check) const {
         assert(this->type == group_type::separated_ordered);

         if (this->state.current_item_index == 0) {
            return false;
         }
         if (this->state.current_item_index == this->children().size() - 1) {
            //
            // We're on our last item. Is it already down? If so, then our separate and 
            // ordered group has been entered in full, so it can't be interrupted. (We 
            // need this check so that keys belonging to the group's next-sibling(s) don't 
            // count as retroactively "interrupting" it.)
            //
            if (this->children()[this->state.current_item_index]._is_separate_ordered_group_complete(check)) {
               return false;
            }
         }
         assert(this->state.current_item_index < this->children().size());

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
                  assert(current.state.current_item_index < current.children().size());
               
                  auto& next = current.children()[current.state.current_item_index];
                  recurse(next, recurse);
                  return;
               }
               for (const auto& item : current.children()) {
                  recurse(item, recurse);
               }
            };
            recurse(current, recurse);
         };
         traverse(this->children()[this->state.current_item_index]);

         for (size_t i = check.start_at; i < check.buttons.size(); ++i) {
            if (!check.buttons[i].matched) {
               return true;
            }
         }
         return false;
      }

      constexpr void input_sequence_flat::group::clear_all_progress() {
         this->state.current_item_index = 0;
      }
      #pragma endregion
   #pragma endregion

   #pragma region input_sequence_flat
   constexpr const input_sequence_flat::group* input_sequence_flat::root() const {
      if (this->contents.empty())
         return nullptr;
      return &this->contents[0];
   }

   constexpr bool input_sequence_flat::has_any_buttons() const {
      for (const auto& g : this->contents)
         if (g.type == group_type::single_control)
            return true;
      return false;
   }
   constexpr bool input_sequence_flat::has_range_requirement() const {
      if (this->range.vector != vector_input_control::none)
         return true;
      if (this->range.scalar.type != scalar_input_control::none)
         return true;
      return false;
   }
   constexpr bool input_sequence_flat::has_raycast_requirement() const {
      return (this->raycast.associated_button != index_of_none);
   }
   constexpr bool input_sequence_flat::is_probably_keyboard_impossible() const {
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
      if (this->contents.empty())
         return false;

      auto recurse = [](const group& self, auto& recurse) constexpr -> size_t {
         if (self.type == group_type::single_control) {
            if (self.button.is_modifier_key())
               return 0;
            return 1;
         }
         if (self.type == group_type::separated_ordered) {
            size_t count = 0;
            for (const auto& item : self.children()) {
               auto cc = recurse(item, recurse);
               if (cc > count)
                  count = cc;
            }
            return count;
         }
         size_t count = 0;
         for (const auto& item : self.children()) {
            count += recurse(item, recurse);
         }
         return count;
      };

      size_t concurrent_input_count = recurse(*this->root(), recurse);
      return (concurrent_input_count > 2);
   }
   constexpr size_t input_sequence_flat::specificity() const {
      size_t result = 0;
      for (const auto& item : this->contents)
         if (item.type == group_type::single_control)
            ++result;

      // Range requirements should increase specificity by 0.5, but I don't 
      // want to actually use a float for this. The cheap, lazy hack is to just 
      // double the specificity value we got above, and then conditionally add 1. 
      // These values are opaque to outside callers -- only comparisons between 
      // them are meaningful; the values themselves are not -- so this should be 
      // fine.
      result *= 2;
      if (this->has_range_requirement()) {
         result += 1;
      }

      return result;
   }
   constexpr std::vector<inputs::button> input_sequence_flat::terminal_inputs() const {
      std::vector<inputs::button> out;
      if (auto* r = this->root())
         r->terminal_inputs(out);
      return out;
   }

   constexpr const input_sequence_flat::group* input_sequence_flat::final_group() const {
      if (this->contents.empty())
         return nullptr;
      return this->root()->final_group();
   }

      #pragma region Algorithms
      constexpr void input_sequence_flat::clear_all_progress() {
         this->state.frame_status         = frame_status::inactive;
         this->state.frame_status_changed = false;
         this->state.went_down_at         = zero_timestamp;
         this->state.raycast_success_flag = false;
         for (auto& item : this->contents)
            item.clear_all_progress();
      }
      #pragma endregion
   #pragma endregion
}