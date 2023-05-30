#pragma once
#include "./input_sequence.h"
#include <type_traits> // std::is_constant_evaluated
#include "helpers/bitstreams/writer.h"
#include "helpers/bitstreams/reader.h"

namespace dovahkit::subsystems::worldinput2 {
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
   constexpr input_sequence::~input_sequence() {
      if (auto*& p = this->root) {
         delete p;
         p = nullptr;
      }
      this->raycast.associated_button = nullptr;
   }

   constexpr bool input_sequence::has_any_buttons() const {
      if (!this->root)
         return false;
      auto recurse = [](const input_sequence::group& current, const auto& recurse) constexpr -> bool {
         if (current.type == group_type::single_control) {
            return !current.button.empty();
         }
         for (const auto* item : current.children) {
            if (recurse(*item, recurse))
               return true;
         }
         return false;
      };
      return recurse(*this->root, recurse);
   }
   constexpr bool input_sequence::has_range_requirement() const {
      return (this->range.control != range_input_control::none);
   }
   constexpr bool input_sequence::has_raycast_requirement() const {
      if (this->raycast.associated_button)
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
   constexpr std::vector<inputs::button> input_sequence::terminal_inputs() const {
      std::vector<inputs::button> out;
      if (this->root)
         this->root->terminal_inputs(out);
      return out;
   }

   constexpr input_sequence& input_sequence::operator=(input_sequence&& other) noexcept {
      std::swap(this->root, other.root);
      std::swap(this->range, other.range);
      std::swap(this->raycast.associated_button, other.raycast.associated_button);
      std::swap(this->raycast.requirement,       other.raycast.requirement);
      std::swap(this->state, other.state);
      return *this;
   }
   #pragma endregion

   #pragma region Serialization
   constexpr void input_sequence::range_requirement::stream(cobb::bitstreams::reader& s) {
      bool presence;

      s.stream(presence);
      if (presence) {
         s.stream(this->control);
         if (range_input_control_has_multiple_axes(this->control)) {
            s.stream(this->axes);
         } else {
            this->axes = range_input_axes::all;
         }
      } else {
         this->control = range_input_control::none;
         this->axes    = range_input_axes::all;
      }
   }
   constexpr void input_sequence::range_requirement::stream(cobb::bitstreams::writer& s) const {
      s.stream((bool)(this->control != range_input_control::none));
      if (this->control != range_input_control::none) {
         s.stream(this->control);
         if (range_input_control_has_multiple_axes(this->control)) {
            s.stream(this->axes);
         }
      }
   }

   constexpr void input_sequence::group::stream(cobb::bitstreams::reader& s) {
      this->type = (group_type)s.stream_bits(2);
      if (this->type == group_type::single_control) {
         s.stream(this->button);
      } else {
         uint32_t size;
         s.stream(size);
         this->children.resize(size);
         for (size_t i = 0; i < size; ++i) {
            (this->children[i] = new group)->stream(s);
         }
      }
   }
   constexpr void input_sequence::group::stream(cobb::bitstreams::writer& s) const {
      s.stream_bits(2, (uint32_t)this->type);
      if (this->type == group_type::single_control) {
         s.stream(this->button);
         assert(this->children.empty());
      } else {
         uint32_t size = this->children.size();
         s.stream(size);
         if (size > 0) {
            for (const auto* g : this->children) {
               g->stream(s);
            }
         }
      }
   }

   constexpr void input_sequence::stream(cobb::bitstreams::reader& s) {
      this->range.stream(s);

      cobb::bitstreams::reader::position_type position_of_rab;
      uint32_t index_of_rab    = -1;

      bool presence;
      s.stream(presence);
      if (presence) {
         position_of_rab = s.get_position();
         s.stream(index_of_rab);
         //
         // Raycast requirements have a lot of bitfields, and it's impossible to take 
         // pointers or references to bitfield elements, so the code below will be a 
         // bit... verbose.
         //
         {
            auto& req = this->raycast.requirement;
            {
               uint8_t v;
               s.stream(
                  v,
                  req.fail_if_target_changes
               );
               req.timing = (raycast_requirement::timing_type)v;
            }
            {
               auto& t = req.targets;

               raycast_requirement::gizmo_mode gm;
               s.stream(gm);
               t.edit_gizmo_mode = gm;
               if (gm != raycast_requirement::gizmo_mode::none) {
                  raycast_requirement::axis3D ga;
                  s.stream(ga);
                  t.edit_gizmo_axis = ga;
               }

               bool bit;

               s.stream(bit);
               t.landscapes = bit;
               s.stream(bit);
               t.nothing = bit;
               s.stream(bit);
               t.object_references = bit;
            }
            {
               optional_yn v;
               s.stream(v);
               req.target_options.selected = v;
            }
         }
      }

      s.stream(presence);
      if (presence) {
         this->root = new group;
         this->root->stream(s);

         if (index_of_rab != -1) {
            size_t seen = 0;
            auto recurse = [this, index_of_rab, &seen](input_sequence::group& current, auto& recurse) -> bool {
               if (seen == index_of_rab) {
                  this->raycast.associated_button = &current;
                  return true;
               }
               for (auto* child : current.children) {
                  ++seen;
                  if (recurse(*child, recurse))
                     return true;
               }
               return false;
            };
            recurse(*this->root, recurse);

            if (this->raycast.associated_button == nullptr) {
               //
               // TODO: Improve error reporting for this.
               //
               throw cobb::bitstreams::exceptions::read_exception{ position_of_rab };
            }
         }
      } else {
         if (index_of_rab != -1) {
            //
            // TODO: Improve error reporting for this.
            //
            throw cobb::bitstreams::exceptions::read_exception{ position_of_rab };
         }
      }
   }
   constexpr void input_sequence::stream(cobb::bitstreams::writer& s) const {
      s.stream(this->range);

      bool presence;
      
      presence = this->has_raycast_requirement();
      s.stream(presence);
      if (presence) {
         uint32_t index_of_rab = -1;
         {
            size_t seen = 0;
            const auto* rab = this->raycast.associated_button;
            auto recurse = [rab, &seen](const input_sequence::group& current, auto& recurse) -> bool {
               if (&current == rab)
                  return true;
               for (const auto* child : current.children) {
                  ++seen;
                  if (recurse(*child, recurse))
                     return true;
               }
               return false;
            };
            assert(recurse(*this->root, recurse));
            index_of_rab = seen;
         }
         s.stream(index_of_rab);
         //
         {
            const auto& req = this->raycast.requirement;
            s.stream(
               (uint8_t)req.timing,
               req.fail_if_target_changes
            );
            {
               auto& t = req.targets;
               s.stream(t.edit_gizmo_mode);
               if (t.edit_gizmo_mode != raycast_requirement::gizmo_mode::none) {
                  s.stream(t.edit_gizmo_axis);
               }
               s.stream(t.landscapes);
               s.stream(t.nothing);
               s.stream(t.object_references);
            }
            s.stream(req.target_options.selected);
         }
      }

      presence = (this->root != nullptr);
      s.stream(presence);
      if (this->root) {
         s.stream(*this->root);
      }
   }
   #pragma endregion
}