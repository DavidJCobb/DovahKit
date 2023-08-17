#pragma once
#include <string>
#include <string_view>
#include "./types.h"

namespace cobb::ini {
   constexpr bool parse_value(std::string_view raw, bool& out);
   constexpr bool parse_value(std::string_view raw, double& out);
   constexpr bool parse_value(std::string_view raw, signed int& out);
   constexpr bool parse_value(std::string_view raw, unsigned int& out);
   constexpr bool parse_value(std::string_view raw, std::string& out);

   constexpr bool parse_value(std::string_view raw, value_variant& out) {
      bool success = false;
      value_types::for_each_pair_until_true([&raw, &out, &success]<typename Key, char Value>() {
         if (std::holds_alternative<Key>(out)) {
            success = parse_value(raw, std::get<Key>(out));
            return true;
         }
         return false;
      });
      return success;
   }

   constexpr std::string stringify_value(bool);
   constexpr std::string stringify_value(double);
   constexpr std::string stringify_value(signed int);
   constexpr std::string stringify_value(unsigned int);
   constexpr std::string stringify_value(const std::string&);

   constexpr std::string stringify_value(const value_variant& out) {
      std::string result;
      value_types::for_each_pair_until_true([&out, &result]<typename Key, char Value>() {
         if (std::holds_alternative<Key>(out)) {
            result = stringify_value(std::get<Key>(out));
            return true;
         }
         return false;
      });
      return result;
   }
}

#include "./value_serialization.inl"