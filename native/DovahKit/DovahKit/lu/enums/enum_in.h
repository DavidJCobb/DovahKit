#pragma once
#include <type_traits>
#include "../utils/integral_wrapper.h"

namespace lu {
   //
   // Store an enum in a different underlying type than its normal one.
   //
   template<typename Enum, typename Underlying>
      requires (std::is_enum_v<Enum> && (std::is_integral_v<Underlying> || std::is_arithmetic_v<Underlying>))
   class enum_in;

   template<typename Enum, typename Underlying>
      requires (std::is_enum_v<Enum> && std::is_integral_v<Underlying>)
   class alignas(sizeof(Underlying)) enum_in<Enum, Underlying> : protected utils::integral_wrapper<enum_in<Enum, Underlying>, Underlying> {
      private:
         using _my_base = utils::integral_wrapper<enum_in, Underlying>;
      public:
         using _my_base::as_integral;
         using _my_base::set_from_integral;

         using value_type      = Enum;
         using underlying_type = Underlying;

      public:
         constexpr enum_in() {}
         constexpr enum_in(value_type v) {
            this->wrapped_integral = (underlying_type)v;
         }

         constexpr bool operator==(value_type v) const noexcept {
            return (value_type)this->wrapped_integral == v;
         }

         constexpr operator value_type() const noexcept {
            return (value_type)this->wrapped_integral;
         }

         template<typename U> requires std::is_convertible_v<underlying_type, U>
         explicit constexpr operator U() const noexcept {
            return (U)this->wrapped_integral;
         }

         constexpr enum_in& operator=(value_type v) noexcept {
            this->wrapped_integral = (underlying_type)v;
            return *this;
         }
   };

   template<typename Enum, typename Underlying>
      requires (std::is_enum_v<Enum> && !std::is_integral_v<Underlying>)
   class alignas(sizeof(Underlying)) enum_in<Enum, Underlying> {
      public:
         using value_type      = Enum;
         using underlying_type = Underlying;

      protected:
         underlying_type wrapped_value = {};

      public:
         constexpr enum_in() {}
         constexpr enum_in(value_type v) {
            this->wrapped_value = (underlying_type)v;
         }

         constexpr bool operator==(value_type v) const noexcept {
            return (value_type)this->wrapped_value == v;
         }

         constexpr operator value_type() const noexcept {
            return (value_type)this->wrapped_value;
         }

         template<typename U> requires std::is_convertible_v<underlying_type, U>
         explicit constexpr operator U() const noexcept {
            return (U)this->wrapped_value;
         }

         constexpr enum_in& operator=(value_type v) noexcept {
            this->wrapped_value = (underlying_type)v;
            return *this;
         }
   };
}
