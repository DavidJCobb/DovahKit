#pragma once
#include <algorithm>
#include "./integer_literal.h"

#pragma push_macro("CLASS_NAME")
#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#define CLASS_TEMPLATE_PARAMS template<typename ValueType, char DigitSeparator> requires (std::is_integral_v<ValueType>&& std::is_arithmetic_v<ValueType>)
#define CLASS_NAME integer_literal<ValueType, DigitSeparator>

namespace cobb {
   CLASS_TEMPLATE_PARAMS
   constexpr CLASS_NAME::integer_literal(const std::string_view& v) : view(v), data(parse_state{}) {
      _consume_sign();
      _consume_base();
      _consume_digits();

      if (std::holds_alternative<parse_state>(this->data)) { // false if we failed with an error
         const auto& state = std::get<parse_state>(this->data);
         if constexpr (is_signed) {
            if (state.negative) {
               if (state.significand == (unsigned_type)std::numeric_limits<value_type>::max() + 1) {
                  this->data = std::numeric_limits<value_type>::lowest();
               } else {
                  this->data = -(value_type)state.significand;
               }
            } else {
               this->data = (value_type)state.significand;
            }
         } else {
            this->data = (value_type)state.significand;
         }
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
   constexpr void CLASS_NAME::_fail(error e) {
      this->data = e;
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_sign() {
      if constexpr (is_signed) {
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
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_base() {
      if (view.size() <= 2)
         return;
      if (view[0] != '0')
         return;

      auto& state = std::get<parse_state>(this->data);

      char c = view[1];
      if (c > 'Z')
         c -= 0x20; // to ASCII uppercase
      bool handled = true;
      switch (c) {
         case 'B': state.base = 2; break;
         case 'O': state.base = 8; break;
         case 'X': state.base = 16; break;
         default:
            handled = false;
      }
      if (handled)
         view = view.substr(2);
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_consume_digits() {
      if (view.empty())
         return _fail(error::no_digits);

      auto& state = std::get<parse_state>(this->data);

      if (state.base == 2) {
         constexpr const size_t max_bitcount = sizeof(value_type) * 8;
         if (view.size() > max_bitcount) {
            if (view.find_first_not_of("01") != std::string::npos)
               return _fail(error::invalid);
            return _fail(error::overflow);
         }
      }

      const unsigned_type maximum = [&state]() {
         if constexpr (is_signed) {
            return (unsigned_type)std::numeric_limits<value_type>::max() + (state.negative ? 1 : 0);
         } else {
            return std::numeric_limits<value_type>::max();
         }
      }();

      const int  max_numerals = (std::min)((int)state.base, 10);
      const char max_alphabet = 'A' + (state.base > 10 ? (std::min)((char)state.base, (char)(10 + 26)) : 0);

      unsigned_type working = 0;
      bool seen_any = false;
      for (size_t i = 0; i < view.size(); ++i) {
         char digit = view[i];
         if constexpr (DigitSeparator != '\0') {
            if (digit == DigitSeparator)
               continue;
         }
         if (state.base > 10) {
            if (digit >= 'a')
               digit -= 0x20; // to ASCII uppercase

            if (digit >= 'A' && digit <= max_alphabet) {
               digit -= 'A';
               digit += 10;
               working = (working * state.base) + digit;
               seen_any = true;
               continue;
            }
         }
         if (digit < '0')
            return _fail(error::invalid);
         digit -= '0';
         if (digit >= max_numerals)
            return _fail(error::invalid);
         if (state.base != 2) {
            if (working > (maximum - digit) / state.base)
               return _fail(error::overflow);
         }
         working = (working * state.base) + digit;
         seen_any = true;
      }
      if (!seen_any)
         return _fail(error::no_digits);
      state.significand = working;
   }
}

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")