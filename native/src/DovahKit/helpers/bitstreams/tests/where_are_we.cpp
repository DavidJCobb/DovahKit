#pragma once
#include "../reader.h"
#include "../writer.h"

#include <cstdint>
#include "helpers/name_of.h"

namespace {
   constexpr bool result_01 = []() -> bool {
      constexpr uintmax_t bit_pattern = (uintmax_t(0b10101010101010101010101010101010) << 32) | uintmax_t(0b10101010101010101010101010101010);

      struct test_type_inner {
         uint8_t data = 5;

         std::vector<std::string> where_are_we;

         constexpr void stream(cobb::bitstreams::reader& s) {
            s.stream(data);

            const auto& list = s.where_are_we();
            for (size_t i = 0; i < list.size(); ++i) { // IntelliSense chokes if we use a range-based for loop here, for some reason
               const auto& item = list[i];
               this->where_are_we.emplace_back(item.type_name);
            }
         }
         constexpr void stream(cobb::bitstreams::writer& s) const {
            s.stream(data);
         }

         constexpr bool operator==(const test_type_inner&) const noexcept = delete;
      };

      struct test_type_outer {
         uint8_t  field_a = 0;
         uint16_t field_b = 0;
         test_type_inner field_c;

         std::vector<std::string> where_are_we;

         constexpr void stream(cobb::bitstreams::reader& s) {
            s.stream(field_a, field_b, field_c);

            const auto& list = s.where_are_we();
            for (size_t i = 0; i < list.size(); ++i) { // IntelliSense chokes if we use a range-based for loop here, for some reason
               const auto& item = list[i];
               this->where_are_we.emplace_back(item.type_name);
            }
         }
         constexpr void stream(cobb::bitstreams::writer& s) const {
            s.stream(field_a, field_b, field_c);
         }

         constexpr bool operator==(const test_type_outer&) const noexcept = delete;
      };

      test_type_outer src;
      src.field_a = bit_pattern;
      src.field_b = bit_pattern;
      src.field_c.data = bit_pattern;

      cobb::bitstreams::writer w;
      w.stream(w.header());
      w.stream(src);

      test_type_outer dst;

      cobb::bitstreams::reader r;
      r.set_buffer(w.data(), w.get_bytespan());
      r.stream(dst);

      if (src.field_a != dst.field_a)
         return false;
      if (src.field_b != dst.field_b)
         return false;
      if (src.field_c.data != dst.field_c.data)
         return false;

      #ifndef __INTELLISENSE__
         auto& where = dst.field_c.where_are_we;
         if (where.size() != 2)
            return false;
         if (where[0] != cobb::name_of_type<test_type_outer>())
            return false;
         if (where[1] != cobb::name_of_type<test_type_inner>())
            return false;
      #endif

      return true;
   }();
   static_assert(
      result_01,
      "Testing whether the reader bitstream properly keeps track of where it's at, for error-reporting purposes."
   );
}