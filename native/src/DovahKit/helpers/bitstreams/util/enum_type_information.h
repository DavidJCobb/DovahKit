#pragma once
#include <bit>
#include <type_traits>
#include "../enum_serialization_options.h"
#include "./enum_has_explicit_bitcount_override.h"
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
      static constexpr const bool   validate_values  = enum_has_valid_values_list<value_type>;

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

      static constexpr const bool is_signed = []() -> bool {
         //
         // An enum with a signed underlying type may not necessarily define any actual 
         // signed values; the default underlying type for all enums is `signed int`. 
         // If we know the list of allowed values for an enum, then we can check if any 
         // of them is negative. Otherwise, we just have to trust the signedness of the 
         // underlying type, unfortunately.
         //
         if constexpr (!validate_values) {
            return std::is_signed_v<underlying_type>;
         }
         return min_underlying < 0;
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

      static_assert(
         []() -> bool {
            if constexpr (!bitcount_is_explicitly_defined || !validate_values) {
               return true;
            }

            constexpr size_t needed_bits = std::bit_width(std::make_unsigned_t<underlying_type>(max_underlying)) + ((std::is_signed_v<underlying_type> && min_underlying < 0) ? 1 : 0);

            return needed_bits <= default_bitcount;
         }(),
         "Assertion: If this enum has an explicitly set bitcount and valid values list, the bitcount must be large enough to encode all valid values."
      );
   };
}

#include "helpers/macros/static_warning.undef.h"