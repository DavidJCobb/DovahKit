#pragma once
#include "./input_sequence.h"
#include <type_traits> // std::is_constant_evaluated

namespace dovahkit::subsystems::worldinput2 {
   #pragma region input_sequence::directional_constraint
   constexpr bool input_sequence::directional_constraint::operator==(const directional_constraint& other) const {
      if (this->vector != other.vector)
         return false;
      if (this->vector != vector_input_control::none) {
         //
         // Scalar info isn't relevant if we're using a vector. (We can only 
         // use one.)
         //
         return true;
      }
      if (this->scalar.type != other.scalar.type)
         return false;
      if (this->scalar.type != scalar_input_control::none) {
         if (this->scalar.axis != other.scalar.axis)
            return false;
      }
      return true;
   }
   #pragma endregion

   #pragma region input_sequence::control_set
   constexpr bool input_sequence::control_set::contains(const inputs::button& btn) const {
      for (const auto& item : this->buttons)
         if (item == btn)
            return true;
      return false;
   }
   constexpr bool input_sequence::control_set::overlaps(const control_set& other) const {
      if (this->direction == other.direction)
         return true;
      for (const auto& item_a : this->buttons)
         for (const auto& item_b : other.buttons)
            if (item_a == item_b)
               return true;
      return false;
   }
   #pragma endregion

   #pragma region input_sequence::group
   constexpr input_sequence::group::~group() {
      for (auto* child : this->children)
         delete child;
      this->children.clear();
   }

   constexpr bool input_sequence::group::can_have_children() const noexcept {
      switch (this->type) {
         case group_type::single_control:
            return false;
      }
      return true;
   }
   constexpr bool input_sequence::group::is_concurrent() const noexcept {
      switch (this->type) {
         case group_type::concurrent_ordered:
         case group_type::concurrent_unordered:
            return true;
      }
      return false;
   }
   constexpr bool input_sequence::group::is_ordered() const noexcept {
      switch (this->type) {
         case group_type::concurrent_ordered:
         case group_type::separated_ordered:
            return true;
      }
      return false;
   }

   constexpr void input_sequence::group::terminal_inputs(std::vector<inputs::button>& append_to) const {
      if (this->type == group_type::single_control) {
         append_to.push_back(this->button);
         return;
      }
      if (this->type == group_type::separated_ordered) {
         if (this->children.empty())
            return;
         this->children.back()->terminal_inputs(append_to);
         return;
      }
      for (const auto* item : this->children) {
         item->terminal_inputs(append_to);
      }
   }
   constexpr size_t input_sequence::group::input_control_count() const {
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
   constexpr bool input_sequence::group::is_or_contains_input_control(const inputs::button& button) const {
      if (this->type == group_type::single_control) {
         return this->button == button;
      }
      for (const auto* item : this->children) {
         if (item->is_or_contains_input_control(button))
            return true;
      }
      return false;
   }

   constexpr const input_sequence::group& input_sequence::group::current_item() const {
      if (std::is_constant_evaluated()) {
         if (this->type != group_type::separated_ordered)
            throw;
      }
      return *(this->children[this->state.current_item_index]);
   }
   #pragma endregion

   #pragma region input_sequence
   constexpr bool input_sequence::has_directional_requirement() const {
      if (this->directional.vector != vector_input_control::none)
         return true;
      if (this->directional.scalar.type != scalar_input_control::none)
         return true;
      return false;
   }
   constexpr bool input_sequence::is_probably_keyboard_impossible() const {
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

      auto recurse = [](const input_sequence::group& self, auto& recurse) constexpr -> size_t {
         if (self.type == input_sequence::group_type::single_control) {
            if (self.button.is_modifier_key())
               return 0;
            return 1;
         }
         if (self.type == input_sequence::group_type::separated_ordered) {
            size_t count = 0;
            for (const auto* item : self.children) {
               auto cc = recurse(*item, recurse);
               if (cc > count)
                  count = cc;
            }
            return count;
         }
         size_t count = 0;
         for (const auto* item : self.children) {
            count += recurse(*item, recurse);
         }
         return count;
      };

      size_t concurrent_input_count = recurse(*this->root, recurse);
      return (concurrent_input_count > 2);
   }

   constexpr size_t input_sequence::specificity() const {
      if (!this->root)
         return 0;
      size_t result;
      if (this->root->type == group_type::single_control) {
         result = 1;
      } else {
         result = this->root->input_control_count();
      }

      // Directional requirements should increase specificity by 0.5, but I don't 
      // want to actually use a float for this. The cheap, lazy hack is to just 
      // double the specificity value we got above, and then conditionally add 1. 
      // These values are opaque to outside callers -- only comparisons between 
      // them are meaningful; the values themselves are not -- so this should be 
      // fine.
      result *= 2;
      if (this->has_directional_requirement()) {
         result + 1;
      }

      return result;
   }
   constexpr std::vector<inputs::button> input_sequence::terminal_inputs() const {
      std::vector<inputs::button> out;
      if (this->root)
         this->root->terminal_inputs(out);
      return out;
   }
   #pragma endregion
}