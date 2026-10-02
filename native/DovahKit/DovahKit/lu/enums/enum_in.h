#pragma once
#include <type_traits>

namespace lu {
   //
   // Store an enum in a different underlying type than its normal one.
   //
   template<typename Enum, typename Underlying> requires (std::is_enum_v<Enum> && (std::is_integral_v<Underlying> || std::is_arithmetic_v<Underlying>))
   class alignas(sizeof(Underlying)) enum_in {
      public:
         using value_type      = Enum;
         using underlying_type = Underlying;

      protected:
         Underlying _value = {};

      public:
         constexpr enum_in() {}
         constexpr enum_in(value_type v) : _value((underlying_type)v) {}

         constexpr bool operator==(value_type v) const noexcept {
            return (value_type)this->_value == v;
         }

         constexpr operator value_type() const noexcept {
            return (value_type)this->_value;
         }

         template<typename U> requires std::is_convertible_v<underlying_type, U>
         explicit constexpr operator U() const noexcept {
            return (U)this->_value;
         }

         constexpr enum_in& operator=(value_type v) noexcept {
            this->_value = (underlying_type)v;
            return *this;
         }
   };
}
