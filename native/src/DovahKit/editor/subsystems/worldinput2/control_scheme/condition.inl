#pragma once
#include "./condition.h"
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace dovahkit::subsystems::worldinput2 {
   constexpr bool control_scheme_condition::impossible() const noexcept {
      if (auto& cnd = this->editor_modes; cnd.has_value()) {
         if (cnd.value().empty())
            return true;
      }
      if (auto& cnd = this->selection_count; cnd.has_value()) {
         auto& data = cnd.value();
         if (data.comparisons.empty())
            return true;
      }
      return false;
   }

   constexpr bool control_scheme_condition::operator==(const control_scheme_condition& other) const noexcept {
      if (this->editor_modes != other.editor_modes)
         return false;
      if (this->selection_count != other.selection_count)
         return false;
      return true;
   }

   constexpr control_scheme_condition& control_scheme_condition::operator&=(const control_scheme_condition& other) {
      if (other.editor_modes.has_value()) {
         auto& ov = other.editor_modes.value();
         if (this->editor_modes.has_value()) {
            this->editor_modes.value() &= ov;
         } else {
            this->editor_modes = ov;
         }
      }
      if (other.selection_count.has_value()) {
         auto& ov = other.selection_count.value();
         if (this->selection_count.has_value()) {
            this->selection_count.value() &= ov;
         } else {
            this->selection_count = ov;
         }
      }
      return *this;
   }

   constexpr void control_scheme_condition::stream(cobb::bitstreams::reader& s) {
      {  // Editor modes
         auto& cnd      = this->editor_modes;
         bool  presence = s.stream_bits(1);
         if (presence) {
            auto& data = cnd.emplace();
            if (s.stream_bits(1))
               data.set(editor_mode::objects);
            if (s.stream_bits(1))
               data.set(editor_mode::navmesh);
            if (s.stream_bits(1))
               data.set(editor_mode::terrain);
         } else {
            cnd = {};
         }
      }
      {  // Selection count
         auto& cnd      = this->selection_count;
         bool  presence = s.stream_bits(1);
         if (presence) {
            auto& data = cnd.emplace();

            size_t count    = 0;
            bool   multiple = s.stream_bits(1);
            if (multiple) {
               count = s.stream_bits(8);
            } else {
               count = 1;
            }

            data.comparisons.reserve(count);
            for (size_t i = 0; i < count; ++i) {
               auto& comparison = data.comparisons.emplace_back();
               s.stream(comparison.op);
               s.stream_bits(32, comparison.comparand);
            }
         } else {
            cnd = {};
         }
      }
   }
   constexpr void control_scheme_condition::stream(cobb::bitstreams::writer& s) const {
      {  // Editor modes
         auto& cnd = this->editor_modes;
         s.stream_bits(1, cnd.has_value());
         if (cnd.has_value()) {
            auto& data = cnd.value();
            s.stream_bits(1, data.test(editor_mode::objects));
            s.stream_bits(1, data.test(editor_mode::navmesh));
            s.stream_bits(1, data.test(editor_mode::terrain));
         }
      }
      {  // Selection count
         auto& cnd      = this->selection_count;
         s.stream_bits(1, cnd.has_value());
         if (cnd.has_value()) {
            auto& data = cnd.value();

            if (data.comparisons.size() == 1) {
               s.stream_bits(1, false);
            } else {
               assert(data.comparisons.size() < (1 << 8));
               s.stream_bits(1, true);
               s.stream_bits(8, data.comparisons.size());
            }
            for (const auto& cmp : data.comparisons) {
               s.stream(cmp.op);
               s.stream_bits(32, cmp.comparand);
            }
         }
      }
   }
}