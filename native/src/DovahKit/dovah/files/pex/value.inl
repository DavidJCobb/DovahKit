#pragma once
#include "./value.h"

namespace dovah::pex {
   constexpr bool value::is_of_type(underlying_value_type t) const noexcept {
      if (this->type != t)
         return false;
      switch (t) {
         using enum underlying_value_type;
         case boolean:
            return std::holds_alternative<bool>(this->content);
         case float32:
            return std::holds_alternative<float>(this->content);
         case integer:
            return std::holds_alternative<int32_t>(this->content);
         case object:
         case string:
            return std::holds_alternative<tabled_string>(this->content);
      }
      return false;
   }

   template<typename Stream>
   constexpr void value::read(Stream& stream) {
      stream.read(type);
      switch (type) {
         case underlying_value_type::object:
         case underlying_value_type::string:
            stream.read(content.emplace<tabled_string>());
            break;
         case underlying_value_type::integer:
            stream.read(content.emplace<int32_t>());
            break;
         case underlying_value_type::float32:
            stream.read(content.emplace<float>());
            break;
         case underlying_value_type::boolean:
            stream.read(content.emplace<bool>());
            break;
      }
   }
   template<typename Stream>
   /*static*/ constexpr void value::skip(Stream& stream) {
      underlying_value_type t;
      stream.read(t);
      //
      switch (t) {
         case underlying_value_type::object:
         case underlying_value_type::string:
            tabled_string::skip(stream);
            break;
         case underlying_value_type::integer:
         case underlying_value_type::float32:
            stream.skip_bytes(4);
            break;
         case underlying_value_type::boolean:
            stream.skip_bytes(1);
            break;
      }
   }
}
