#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include "../unicode/general_category_is_other.h"
#include "../utf8/append.h"
#include "../utf8/for_each.h"

namespace cobb {
   namespace impl::_backslash_escape {
      struct escape_map_entry {
         char seq;
         char out;
      };
      constexpr const auto escapes = std::array{
         escape_map_entry{'0', '\0'},
         escape_map_entry{'\\', '\\'},
         escape_map_entry{'a',  '\a'},
         escape_map_entry{'b',  '\b'},
         escape_map_entry{'f',  '\f'},
         escape_map_entry{'n',  0xA},
         escape_map_entry{'r',  0xD},
         escape_map_entry{'t',  '\t'},
         escape_map_entry{'v',  '\v'},
      };

      template<size_t DigitCount>
      constexpr void append_hex_digits(std::string& out, std::uint32_t v) {
         for (size_t i = 0; i < DigitCount; ++i) {
            char n = v >> (4 * (DigitCount - i - 1));
            n &= 0xF;
            if (n >= 10)
               out += (n - 10) + 'A';
            else
               out += n + '0';
         }
      }
   }

   // Assumes UTF-8 text.
   template<backslash_escape_type Mode>
   constexpr std::string backslash_escape(std::string_view src, char delim) {
      std::string out;

      cobb::utf8::for_each(src, [delim, &out](uint32_t c) -> void {
         if (c == '\\') {
            out += "\\\\";
            return;
         }
         {
            bool replaced = false;
            for (const auto& item : impl::_backslash_escape::escapes) {
               if (c == item.out) {
                  out += '\\';
                  out += item.seq;
                  replaced = true;
                  break;
               }
            }
            if (replaced)
               return;
         }

         bool printable = !unicode::general_category_is_other(c);
         if (!printable || (c >= 0x0b10000000 && c == delim)) {
            //
            // There is no defined escape sequence for this glyph, so use \x or \u as appropriate.
            //
            if constexpr (Mode == backslash_escape_type::javascript) {
               if (c <= 0xFF) {
                  out += "\\x";
                  impl::_backslash_escape::append_hex_digits<2>(out, c);
               } else if (c <= 0xFFFF) {
                  out += "\\u";
                  impl::_backslash_escape::append_hex_digits<4>(out, c);
               } else {
                  out += "\\u{";
                  impl::_backslash_escape::append_hex_digits<6>(out, c);
                  out += '}';
               }
               return;
            } else if constexpr (Mode == backslash_escape_type::cpp) {
               //
               // C++ allows any arbitrary number of digits for the \x... sequences. We *could* 
               // use shorter sequences when possible, but that's actually a recipe for disaster. 
               // If we try to concatenate "\x02" and "d", for example, we get "\x02d", which... 
               // is actually equivalent to "\x2D". Nice, C++. Nice.
               //
               out += "\\x{";
               if (c <= 0xFF) {
                  impl::_backslash_escape::append_hex_digits<2>(out, c);
               } else if (c <= 0xFFFF) {
                  impl::_backslash_escape::append_hex_digits<4>(out, c);
               } else if (c <= 0xFFFFFF) {
                  impl::_backslash_escape::append_hex_digits<6>(out, c);
               } else {
                  impl::_backslash_escape::append_hex_digits<8>(out, c);
               }
               out += '}';
               return;
            } else {
               if (std::is_constant_evaluated())
                  throw; // ERROR: NOT IMPLEMENTED
            }
         }

         if (c >= 128) {
            cobb::utf8::append(out, c);
            return;
         }
         if (c == delim)
            out += '\\';
         out += c;
      });

      return out;
   }

   namespace impl::_backslash_unescape {
      enum class escape_type {
         none,
         unknown,
         two_digit,
         four_digit,
         unlimited_digits, // cpp: \x...
         brace_delimited,  // cpp: `\x{...}`; JS: `\u{...}`
      };
      struct unescape_state {
         escape_type type        = escape_type::none;
         size_t      digits_seen = 0;
         uint32_t    codepoint   = 0;
      };
   }

   // Assumes UTF-8 text.
   template<backslash_escape_type Mode>
   constexpr std::string backslash_unescape(std::string_view src) {
      using namespace impl::_backslash_unescape;

      std::string out;

      unescape_state escape_state;

      cobb::utf8::for_each(src, [&out, &escape_state](uint32_t c) -> void {
         if constexpr (Mode == backslash_escape_type::cpp) {
            //
            // This state actually has to be handled first, because a C++ \x... escape 
            // sequence stops upon finding any invalid hex digit (so long as we've read 
            // at least one valid digit). This means that to avoid swallowing the glyph 
            // immediately after the escape sequence, we need to fall through.
            //
            if (escape_state.type == escape_type::unlimited_digits) {
               if (escape_state.digits_seen == 0 && c == '{') {
                  escape_state.type = escape_type::brace_delimited;
                  return;
               }

               if (c >= 'a' && c <= 'z')
                  c -= 0x20; // to uppercase ASCII
               bool is_digit = (c >= '0' && c <= '9');
               bool is_alpha = (c >= 'A' && c <= 'F');

               if (is_digit || is_alpha) {
                  int digit = 0;
                  if (is_digit)
                     digit = c - '0';
                  else if (is_alpha)
                     digit = c - 'A' + 10;

                  // TODO: Throw on overflow
                  escape_state.codepoint <<= 4;
                  escape_state.codepoint |= digit;
                  ++escape_state.digits_seen;
                  //
                  // NOTE: Visual Studio's syntax highlighter makes it seem like \x... escapes cap out 
                  // at four hex digits. This is not true. If not brace-delimited, they swallow anything 
                  // that looks like a hex digit.
                  //
                  return;
               }

               if (escape_state.digits_seen == 0)
                  throw std::logic_error("malformed escape sequence");
               cobb::utf8::append(out, escape_state.codepoint);
               escape_state = {};
               //
               // And fall through.
            }
         }

         if (escape_state.type == escape_type::none) {
            if (c != '\\') {
               cobb::utf8::append(out, c);
               return;
            }
            escape_state.type = escape_type::unknown;
            return;
         }
         if (escape_state.type == escape_type::unknown) { // last glyph was '\\'
            if (c == '\\') {
               out += '\\';
               escape_state = {};
               return;
            }
            for (const auto& item : impl::_backslash_escape::escapes) {
               if (c == item.seq) {
                  out += item.out;
                  escape_state = {};
                  return;
               }
            }
            if (c == 'x') {
               if constexpr (Mode == backslash_escape_type::cpp) {
                  escape_state.type = escape_type::unlimited_digits;
               } else if constexpr (Mode == backslash_escape_type::javascript) {
                  escape_state.type = escape_type::two_digit;
               }
               return;
            }
            if constexpr (Mode == backslash_escape_type::javascript) {
               if (c == 'u') {
                  escape_state.type = escape_type::four_digit;
                  return;
               }
            }
            cobb::utf8::append(out, c);
            escape_state = {};
            return;
         }
         if constexpr (Mode == backslash_escape_type::javascript) {
            if (escape_state.type == escape_type::four_digit && escape_state.digits_seen == 0 && c == '{') {
               escape_state.type = escape_type::brace_delimited;
               return;
            }
            if (escape_state.type == escape_type::two_digit || escape_state.type == escape_type::four_digit) {
               if (c >= 'a' && c <= 'z')
                  c -= 0x20; // to uppercase ASCII
               //
               int digit = 0;
               if (c >= '0' && c <= '9')
                  digit = c - '0';
               else if (c >= 'A' && c <= 'Z')
                  digit = c - 'A' + 10;
               else {
                  throw std::logic_error("malformed escape sequence");
               }

               escape_state.codepoint <<= 4;
               escape_state.codepoint  |= digit;
               ++escape_state.digits_seen;
               if (
                  (escape_state.type == escape_type::two_digit  && escape_state.digits_seen == 2)
               || (escape_state.type == escape_type::four_digit && escape_state.digits_seen == 4)
               ) {
                  cobb::utf8::append(out, escape_state.codepoint);
                  escape_state = {};
               }
               return;
            }
         }
         if (escape_state.type == escape_type::brace_delimited) {
            if (c == '}') {
               if (escape_state.digits_seen == 0)
                  throw std::logic_error("malformed escape sequence");
               cobb::utf8::append(out, escape_state.codepoint);
               escape_state = {};
               return;
            }

            if (c >= 'a' && c <= 'z')
               c -= 0x20; // to uppercase ASCII
            //
            int digit = 0;
            if (c >= '0' && c <= '9')
               digit = c - '0';
            else if (c >= 'A' && c <= 'Z')
               digit = c - 'A' + 10;
            else {
               throw std::logic_error("malformed escape sequence");
            }
            
            // TODO: Throw on overflow
            escape_state.codepoint <<= 4;
            escape_state.codepoint  |= digit;
            ++escape_state.digits_seen;
            return;
         }
      });

      return out;
   }
}