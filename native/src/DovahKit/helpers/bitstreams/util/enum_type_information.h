#pragma once
#include <type_traits>
#include "../enum_serialization_options.h"
#include "./enum_has_valid_values_list.h"

#include "helpers/macros/static_warning.h"

namespace cobb::bitstreams::util {
   template<typename Enum> requires std::is_enum_v<Enum>
   struct enum_type_information {
      enum_type_information() = delete;

      using value_type = Enum;

      using options         = enum_serialization_options<value_type>;
      using underlying_type = std::underlying_type_t<value_type>;

      static constexpr const bool   bitcount_is_explicitly_defined = enum_has_explicit_bitcount_override<Enum>;
      static constexpr const size_t default_bitcount = []() {
         if constexpr (bitcount_is_explicitly_defined) {
            return options::bitcount;
         }
         return sizeof(underlying_type) * 8;
      }();
      static constexpr const bool validate_values  = enum_has_valid_values_list<value_type>;

      static constexpr const underlying_type min_underlying = []() {
         underlying_type v = std::numeric_limits<underlying_type>::max();

         if constexpr (validate_values) {
            for (const auto item : options::valid_values)
               if ((underlying_type)item < v)
                  v = (underlying_type)item;
         } else {
            v = std::numeric_limits<underlying_type>::min();
         }

         return v;
      }();
      static constexpr const underlying_type max_underlying = []() {
         underlying_type v = std::numeric_limits<underlying_type>::lowest();
         
         if constexpr (validate_values) {
            for (const auto item : options::valid_values)
               if ((underlying_type)item > v)
                  v = (underlying_type)item;
         } else {
            v = std::numeric_limits<underlying_type>::max();
         }

         return v;
      }();

      static constexpr const bool valid_values_are_contiguous = []() {
         if constexpr (validate_values) {
            const auto& list = options::valid_values;
            for (size_t i = 1; i < list.size(); ++i)
               if ((underlying_type)list[i] != (underlying_type)list[i - 1] + underlying_type{1})
                  return false;
         }
         return false;
      }();

      static constexpr const bool value_is_valid(value_type v) noexcept {
         if constexpr (validate_values) {
            if constexpr (!valid_values_are_contiguous) {
               for (const auto item : options::valid_values)
                  if (v == item)
                     return true;
               return false;
            } else {
               underlying_type u = (underlying_type)v;
               if (u < min_underlying || v > max_underlying)
                  return false;
            }
         }
         return true;
      }

      static_warning(
         (!std::is_same_v<underlying_type, int> || bitcount_is_explicitly_defined),
         "\
Serializing a bitcount is unsafe if it doesn't have either an explicit bitcount override, or an underlying type with a consistent size (e.g. uint16_t). \
(NOTE: This warning may emit as a false-positive for some types, e.g. int32_t, since some compilers just typedef these over the non-explicit integer types. \
In that case, redundantly specifying a bitcount can suppress this warning.)"
      );
   };
}

#include "helpers/macros/static_warning.undef.h"