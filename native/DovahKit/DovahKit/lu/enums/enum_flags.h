#pragma once
#include <cstdint>
#include <type_traits>
#include "../concepts/all_same_as.h"
#include "../type_aliases/uint_of_size.h"

namespace lu {
   //
   // Create a bitmask type, from an enum whose members are bit indices 
   // beginning with 0 for the least-significant bit.
   //
   template<typename Enum, size_t Bits> requires (std::is_enum_v<Enum> && Bits <= 64)
   class enum_flags {
      public:
         static constexpr const size_t bitcount = Bits;
         using value_type = Enum;
         using mask_type  = uint_of_size<(bitcount / 8 + (bitcount % 8 ? 1 : 0))>;

      protected:
         mask_type _value = 0;

         static constexpr mask_type _to_mask(value_type v) {
            return (mask_type)1 << (mask_type)v;
         }

         static constexpr bool _is_representable(value_type v) {
            return (size_t)v < bitcount;
         }

      public:
         #pragma region Constructors
            constexpr enum_flags() {}

            template<typename... E> requires (sizeof...(E) > 0 && concepts::all_same_as<value_type, E...>)
            constexpr enum_flags(E... values) {
               this->_value = 0;
               this->_value |= (_to_mask(values) | ...);
            }
         #pragma endregion

         template<value_type... Values> requires (sizeof...(Values) > 0 && (_is_representable(Values) && ...))
         static constexpr enum_flags from() {
            constexpr mask_type mask = (_to_mask(Values) | ...);
            enum_flags out;
            out._value = mask;
            return out;
         }

         static constexpr enum_flags from_mask(mask_type mask) {
            enum_flags out;
            out._value = mask;
            return out;
         }

         explicit constexpr operator mask_type() const noexcept {
            return this->_value;
         }

         constexpr void clear() {
            this->_value = 0;
         }
         constexpr void set(value_type v) {
            this->_value |= _to_mask(v);
         }
         constexpr void unset(value_type v) {
            this->_value &= ~_to_mask(v);
         }
         constexpr void filter_to(value_type v) {
            this->_value &= _to_mask(v);
         }

         template<value_type... Values> requires (sizeof...(Values) > 0 && (_is_representable(Values) && ...))
         constexpr void set_each_of() {
            constexpr mask_type mask = (_to_mask(Values) | ...);
            this->_value |= mask;
         }

         template<value_type... Values> requires (sizeof...(Values) > 0 && (_is_representable(Values) && ...))
         constexpr void unset_each_of() {
            constexpr mask_type mask = (_to_mask(Values) | ...);
            this->_value &= ~mask;
         }

         template<value_type... Values> requires (sizeof...(Values) > 0 && (_is_representable(Values) && ...))
         constexpr void filter_to_each_of() {
            constexpr mask_type mask = (_to_mask(Values) | ...);
            this->_value &= mask;
         }

         constexpr bool has(value_type v) const noexcept {
            return this->_value & _to_mask(v);
         }
         template<value_type... Values> requires (sizeof...(Values) > 0 && (_is_representable(Values) && ...))
         constexpr bool has_any_of() const noexcept {
            constexpr mask_type mask = (_to_mask(Values) | ...);
            return !!(this->_value & mask);
         }
         template<value_type... Values> requires (sizeof...(Values) > 0 && (_is_representable(Values) && ...))
         constexpr bool has_all_of() const noexcept {
            constexpr mask_type mask = (_to_mask(Values) | ...);
            return (this->_value & mask) == mask;
         }

         #pragma region Operator overloads
            constexpr enum_flags& operator&=(value_type v) noexcept {
               this->filter_to(v);
               return *this;
            }
            constexpr enum_flags& operator|=(value_type v) noexcept {
               this->set(v);
               return *this;
            }
            
            constexpr enum_flags operator&(value_type v) const noexcept {
               auto out = *this;
               out.filter_to(v);
               return out;
            }
            constexpr enum_flags operator|(value_type v) const noexcept {
               auto out = *this;
               out.set(v);
               return out;
            }
         #pragma endregion
   };

   template<typename T>
   concept enum_flags_type = requires {
      typename T::value_type;
      typename T::mask_type;
      { T::bitcount };
      requires std::is_same_v<T, enum_flags<typename T::value_type, T::bitcount>>;
   };
}
