#pragma once
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>

namespace cobb {
   template<typename ValueType, char DigitSeparator = '\0'> requires (std::is_integral_v<ValueType>&& std::is_arithmetic_v<ValueType>)
   class integer_literal {
      public:
         using value_type = ValueType;

         enum class error {
            empty,
            no_digits,
            invalid,
            overflow,
         };

      protected:
         static constexpr const bool is_signed = std::is_signed_v<value_type>;

         using unsigned_type = std::make_unsigned_t<value_type>;

         struct dummy {};

         struct parse_state {
            unsigned_type significand = 0;
            uint8_t base = 10;
            [[no_unique_adddress]] std::conditional_t<is_signed, bool, const dummy> negative = {};
         };

         std::string_view view;
         std::variant<parse_state, value_type, error> data;

         constexpr void _fail(error);

         constexpr void _consume_sign();
         constexpr void _consume_base();
         constexpr void _consume_digits();

      public:
         constexpr integer_literal(const std::string_view&);

         constexpr std::optional<error> get_error() const;

         constexpr bool is_valid() const;

         // Unchecked; throws on bad access.
         constexpr value_type get_value() const;
   };
}

#include "./integer_literal.inl"