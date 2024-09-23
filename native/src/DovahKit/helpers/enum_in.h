#pragma once
#include <type_traits>

namespace cobb {
   //
   // Override an enum's underlying type; that is, use a given enum 
   // in a given place but with an underlying type of your choosing, 
   // rather than the enum's "natural" underlying type.
   //
   template<typename Enum, typename Underlying> requires (std::is_enum_v<Enum> && std::is_integral_v<Underlying>)
   class enum_in {
      public:
         using value_type      = Enum;
         using underlying_type = Underlying;

      protected:
         underlying_type _value = {0};

      public:
         constexpr enum_in() noexcept = default;
         constexpr enum_in(const enum_in&) noexcept = default;
         constexpr enum_in(enum_in&&) noexcept = default;
         constexpr enum_in(value_type v) noexcept : _value(static_cast<underlying_type>(v)) {}

         constexpr operator value_type() const { return (value_type)this->_value; }
         
         template<typename T> requires std::is_integral_v<T>
         constexpr explicit operator T() const { return static_cast<T>(this->_value); }
   };
}
