#pragma once
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>

namespace cobb {
   template<typename ValueType, char DigitSeparator = '\0'> requires std::is_floating_point_v<ValueType>
   class floating_point_literal {
      public:
         using value_type = ValueType;

         enum class error {
            empty,
            no_digits,
            invalid,
            overflow,
         };

      protected:
         struct parse_state {
            uint32_t significand = 0;
            int16_t  exponent    = 0; // significand times 10 to the N-th power
            uint8_t  base        = 10;

            bool negative : 1 = false;
            bool saw_decimal_separator : 1 = false;
         };

         std::string_view view;
         std::variant<parse_state, value_type, error> data;

         constexpr void _fail(error);

         constexpr bool _is_exponent_delimiter(char) const;
         constexpr std::optional<int> _parse_digit(char) const;

         constexpr void _consume_sign();
         constexpr void _consume_base();
         constexpr void _consume_digits();
         constexpr void _consume_exponent();

      public:
         constexpr floating_point_literal(const std::string_view&);

         constexpr std::optional<error> get_error() const;

         constexpr bool is_valid() const;

         // Unchecked; throws on bad access.
         constexpr value_type get_value() const;

         // Throws on bad access.
         constexpr bool within_epsilon_of(value_type v) const;
   };
}

#include "./floating_point_literal.inl"