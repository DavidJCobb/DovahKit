#pragma once
#include "./floating_point_literal.h"

#pragma push_macro("CLASS_NAME")
#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#define CLASS_TEMPLATE_PARAMS template<typename ValueType, char DigitSeparator> requires std::is_floating_point_v<ValueType>
#define CLASS_NAME floating_point_literal<ValueType, DigitSeparator>

namespace cobb {
   CLASS_TEMPLATE_PARAMS
   constexpr CLASS_NAME::floating_point_literal(const std::string_view& v) : view(v), data(parse_state{}) {
      _consume_sign();
      _consume_base();
      _consume_digits();
      if (std::holds_alternative<parse_state>(this->data)) {
         _consume_exponent();
      }

      if (std::holds_alternative<parse_state>(this->data)) { // false if we failed with an error
         const auto& state = std::get<parse_state>(this->data);

         value_type result = (value_type)state.significand;
         if (state.negative)
            result = -result;

         // std::pow isn't constexpr until C++26, so we'll just do this instead:
         auto exponent = state.exponent;
         if (exponent > 0) {
            if (exponent & 1) {
               result *= 10;
               exponent -= 1;
            }
            for (; exponent >= 4; exponent -= 4)
               result *= 10000;
            for (; exponent >= 2; exponent -= 2)
               result *= 100;
         } else if (exponent < 0) {
            for (; exponent <= -4; exponent += 4)
               result /= 10000;
            for (; exponent <= -2; exponent += 2)
               result /= 100;
            if (exponent == -1)
               result /= 10;
         }

         this->data = result;
      }
   }

   CLASS_TEMPLATE_PARAMS
   constexpr std::optional<typename CLASS_NAME::error> CLASS_NAME::get_error() const {
      if (std::holds_alternative<error>(this->data))
         return std::get<error>(this->data);
      return {};
   }

   CLASS_TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::is_valid() const {
      return std::holds_alternative<value_type>(this->data);
   }

   CLASS_TEMPLATE_PARAMS
   constexpr typename CLASS_NAME::value_type CLASS_NAME::get_value() const {
      return std::get<value_type>(this->data);
   }

   CLASS_TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::within_epsilon_of(value_type v) const {
      auto diff = get_value() - v;
      if (diff < 0)
         diff = -diff;
      return diff <= 0.0001;
   }


   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_fail(error e) {
      this->data = e;
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_sign() {
      if (view.empty())
         return;
      if (view[0] == '-') {
         view = view.substr(1);
         //
         auto& state = std::get<parse_state>(this->data);
         state.negative = true;
      } else if (view[0] == '+') {
         view = view.substr(1);
      }
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_base() {
      if (view.size() <= 2)
         return;
      if (view[0] == '0' && (view[1] == 'x' || view[1] == 'X')) {
         auto& state = std::get<parse_state>(this->data);
         state.base = 16;

         view = view.substr(2);
      }
   }

   CLASS_TEMPLATE_PARAMS
   constexpr std::optional<int> CLASS_NAME::_parse_digit(char digit) const {
      const auto& state = std::get<parse_state>(this->data);
      if (state.base > 10) {
         if (digit >= 'a')
            digit -= 0x20; // to ASCII uppercase

         if (digit >= 'A' && digit <= 'F') {
            digit -= 'A';
            digit += 10;
            return digit;
         }
      }

      if (digit < '0')
         return {};
      digit -= '0';
      if (digit >= 10)
         return {};
      return digit;
   }

   CLASS_TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::_is_exponent_delimiter(char c) const {
      auto& state = std::get<parse_state>(this->data);
      if (state.base == 16) {
         if (c == 'P' || c == 'p') // We make this optional; C++ makes it mandatory for hex floats.
            return true;
      } else {
         if (c == 'E' || c == 'e')
            return true;
      }
      return false;
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_digits() {
      if (view.empty())
         return _fail(error::no_digits);

      auto& state = std::get<parse_state>(this->data);

      size_t i        = 0;
      bool   seen_any = false;
      for (; i < view.size(); ++i) {
         char c = view[i];

         if (c == '.') {
            if (state.saw_decimal_separator)
               return _fail(error::invalid);
            state.saw_decimal_separator = true;
            continue;
         }
         if constexpr (DigitSeparator != '\0') {
            if (c == DigitSeparator)
               continue;
         }
         if (_is_exponent_delimiter(c)) {
            break;
         }

         auto opt_digit = _parse_digit(c);
         if (!opt_digit.has_value())
            return _fail(error::invalid);

         state.significand *= state.base;
         state.significand += opt_digit.value();
         if (state.saw_decimal_separator) {
            state.exponent -= 1;
         }
         seen_any = true;
      }
      if (!seen_any)
         return _fail(error::no_digits);

      view = view.substr(i);
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_exponent() {
      if (view.empty())
         return;
      auto& state = std::get<parse_state>(this->data);
      if (state.base == 16) {
         if (view[0] != 'P' && view[0] != 'p') // We make this optional; C++ makes it mandatory for hex floats.
            return;
      } else {
         if (view[0] != 'E' && view[0] != 'e')
            return;
      }

      size_t i = 1;

      bool negative = false;
      if (view.size() > 1) {
         if (view[1] == '-') {
            negative = true;
            ++i;
         } else if (view[1] == '+') {
            ++i;
         }
      }

      bool any_digits = false;
      int  exp_suffix = 0;
      for (; i < view.size(); ++i) {
         char c = view[i];
         if constexpr (DigitSeparator != '\0') {
            if (c == DigitSeparator)
               continue;
         }
         auto opt_digit = _parse_digit(c);
         if (!opt_digit.has_value())
            return _fail(error::invalid);
         
         any_digits = true;
         exp_suffix *= state.base;
         exp_suffix += opt_digit.value();
      }
      if (!any_digits)
         return _fail(error::invalid);
      if (negative)
         exp_suffix = -exp_suffix;

      state.exponent += exp_suffix;

      view = view.substr(i);
   }
}

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")