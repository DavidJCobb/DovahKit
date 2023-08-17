#pragma once
#include "./value_serialization.h"
#include "../string/backslash_escape.h"
#include "../string/floating_point_literal.h"
#include "../string/integer_literal.h"
#include "../string/replace_all.h"
#include "../string/strieq_ascii.h"

namespace cobb::ini {
   constexpr bool parse_value(std::string_view raw, bool& out) {
      if (cobb::strieq_ascii(raw, "true")) {
         out = true;
         return true;
      }
      if (cobb::strieq_ascii(raw, "false")) {
         out = false;
         return true;
      }
      return false;
   }
   constexpr bool parse_value(std::string_view raw, double& out) {
      const auto lit = cobb::floating_point_literal<std::decay_t<decltype(out)>>(raw);
      if (lit.is_valid()) {
         out = lit.get_value();
         return true;
      }
      return false;
   }
   constexpr bool parse_value(std::string_view raw, signed int& out) {
      const auto lit = cobb::integer_literal<std::decay_t<decltype(out)>>(raw);
      if (lit.is_valid()) {
         out = lit.get_value();
         return true;
      }
      return false;
   }
   constexpr bool parse_value(std::string_view raw, unsigned int& out) {
      const auto lit = cobb::integer_literal<std::decay_t<decltype(out)>>(raw);
      if (lit.is_valid()) {
         out = lit.get_value();
         return true;
      }
      return false;
   }
   constexpr bool parse_value(std::string_view raw, std::string& out) {
      out = cobb::backslash_unescape(raw);
      return true;
   }

   constexpr std::string stringify_value(bool v) {
      if (v)
         return "true";
      return "false";
   }
   constexpr std::string stringify_value(double v) {
      bool   negative  = v < 0;
      double pos_value = negative ? -v : v;

      auto   v_whole = (std::intmax_t)pos_value;
      double v_frac  = pos_value - (std::intmax_t)pos_value;

      std::string out_whole;
      {
         if (v_whole == 0)
            out_whole = "0";
         else {
            while (v_whole > 0) {
               int digit = v_whole % 10;
               v_whole /= 10;
               out_whole += (digit + '0');
            }
            if (negative)
               out_whole += '-';

            std::reverse(out_whole.begin(), out_whole.end());
         }
      }

      std::string out_frac;
      if (v_frac) {
         out_frac = '.';

         std::intmax_t i_frac = 0;
         for (size_t i = 0; i < 7; ++i) {
            v_frac *= 10;
            auto digit = (std::intmax_t)(v_frac);
            v_frac -= digit;

            out_frac += ('0' + digit);
         }

         size_t trunc_at = out_frac.find_last_not_of('0');
         if (trunc_at != std::string::npos)
            out_frac.resize(trunc_at + 1);
      }

      return out_whole + out_frac;
   }
   constexpr std::string stringify_value(signed int v) {
      if (v == 0)
         return "0";

      bool negative = v < 0;

      std::string out;
      while (v > 0) {
         int digit = v % 10;
         v /= 10;
         out += (digit + '0');
      }
      if (negative)
         out += '-';

      std::reverse(out.begin(), out.end());
      return out;
   }
   constexpr std::string stringify_value(unsigned int v) {
      if (v == 0)
         return "0";

      std::string out;
      while (v > 0) {
         int digit = v % 10;
         v /= 10;
         out += (digit + '0');
      }

      std::reverse(out.begin(), out.end());
      return out;
   }
   constexpr std::string stringify_value(const std::string& v) {
      return cobb::backslash_escape(v);
   }
}