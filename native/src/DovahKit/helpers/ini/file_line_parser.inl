#pragma once
#include "./file_line_parser.h"

namespace cobb::ini {
   template<parsing_options Options>
   constexpr file_line_parse_results::line file_line_parser<Options>::parse_line() {
      size_t i = view.find_first_not_of(all_whitespace_chars);
      if (i == std::string::npos) {
         return line{
            .content = no_op_line{}
         };
      }

      if constexpr (!all_comment_delimiters.empty()) {
         if (all_comment_delimiters.contains(view[i])) {
            return line{
               .content = no_op_line{}
            };
         }
      }

      line out;
      out.leading = view.substr(0, i);
      view.remove_prefix(i);

      auto opt_category = try_extract_category_start();
      if (opt_category.has_value()) {
         auto& data = opt_category.value();
         out.content  = data;
         out.trailing = view;
         return out;
      }

      auto opt_kv = try_extract_key_value_pair();
      if (opt_kv.has_value()) {
         out.content  = opt_kv.value();
         out.trailing = view;
         return out;
      }

      return line{
         .content = ill_formed_line{}
      };
   }

   template<parsing_options Options>
   constexpr std::optional<file_line_parse_results::category_start> file_line_parser<Options>::try_extract_category_start() {
      if (view.size() < 2) // minimum possible length e.g. "[a]"
         return {};
      if (view[0] != '[')
         return {};
      view.remove_prefix(1); // remove '['

      category_start out;
      
      std::string_view body;
      {
         size_t end = view.find_first_of(']');
         if (end == std::string::npos)
            return {}; // ill-formed: unterminated header

         if constexpr (Options.mode != parsing_mode::win32) {
            size_t j = view.find_first_of(all_comment_delimiters, 1);
            if (j < end)
               return {}; // ill-formed: unterminated header (interrupted by a line comment)
         }
         body = view.substr(0, end);
      }

      size_t i = body.find_first_not_of(all_whitespace_chars);
      if (i == std::string::npos) {
         out.leading = body;
      } else {
         size_t j = body.find_last_not_of(all_whitespace_chars) + 1;

         out.leading  = body.substr(0, i);
         out.trailing = body.substr(j);
         out.name     = body.substr(i, j - i);
      }

      view = view.substr(body.size() + 1); // plus 1 to skip the ']' too

      return out;
   }

   template<parsing_options Options>
   constexpr std::optional<file_line_parse_results::key_value_pair> file_line_parser<Options>::try_extract_key_value_pair() {
      if (view.size() < 2) // minimum possible length e.g. "s="
         return {};

      size_t i = view.find_first_of('=');
      if (i == std::string::npos)
         return {}; // no key/value separator

      if constexpr (Options.mode != parsing_mode::win32) {
         size_t j = view.find_first_of(all_comment_delimiters);
         if (j < i)
            return {}; // no key/value separator
      }

      std::string_view lhs = view.substr(0, i);
      std::string_view rhs = view.substr(i + 1);

      size_t rhs_start = i + 1;

      if constexpr (Options.mode == parsing_mode::win32) {
         if (lhs.size() > 0)
            if (all_comment_delimiters.contains(lhs[0]))
               return {}; // Win32 treats lines starting with a ';' as comments, but allows ';' in key names
      }

      // right-trim LHS
      {
         size_t i = lhs.find_last_not_of(all_whitespace_chars);
         if (i == std::string::npos)
            return {}; // no key
         lhs = lhs.substr(0, i + 1);
      }
      if (lhs.empty())
         return {}; // no key

      // trim RHS
      {
         size_t i = rhs.find_first_not_of(all_whitespace_chars);
         if (i == std::string::npos) {
            //
            // Entire RHS is whitespace.
            //
            rhs_start += rhs.size();
            rhs = {};
         } else {
            rhs_start += i;
            size_t j = rhs.find_last_not_of(all_whitespace_chars) + 1;
            rhs = rhs.substr(i, j - i);
         }
      }

      std::string_view separator = view.substr(lhs.size(), rhs_start - lhs.size());
      view = view.substr(rhs_start);

      char value_delim = '\0';
      if constexpr (!all_string_delimiters.empty()) {
         if (rhs.size() >= 2 && all_string_delimiters.contains(rhs[0])) { // Handle RHS string delimiters, if present
            char delim = rhs[0];
            if constexpr (Options.mode == parsing_mode::strict) {
               size_t final_delim_at = std::string::npos;
               for (size_t i = rhs.find_first_of(delim, 1); i != std::string::npos; i = rhs.find_first_of(delim, i + 1)) {
                  if (i != std::string::npos && rhs[i - 1] != '\\') {
                     //
                     // We've found a possible closing string delimiter: no preceding backslash.
                     //
                     final_delim_at = i + 1;
                     break;
                  }
               }
               if (final_delim_at != std::string::npos) {
                  if (final_delim_at != rhs.size() - 1) {
                     size_t i = rhs.find_first_not_of(all_whitespace_chars, final_delim_at + 1);
                     if (i != std::string::npos) {
                        if constexpr (!all_comment_delimiters.empty()) {
                           if (!all_comment_delimiters.contains(rhs[i]))
                              //
                              // There is additional non-comment content after the final delimiter.
                              //
                              final_delim_at = std::string::npos;
                        } else {
                           //
                           // There is additional content after the final delimiter.
                           //
                           final_delim_at = std::string::npos;
                        }
                     }
                  }
                  if (final_delim_at != std::string::npos) {
                     //
                     // Confirmed: this is a delimited string. Trim the delimiters.
                     //
                     value_delim = delim;
                     rhs = rhs.substr(1, final_delim_at - 2);
                  }
               }
            } else {
               //
               // Win32 handling: just trim the string delimiter if it's the first and last character.
               //
               if (rhs.back() == delim) {
                  value_delim = delim;
                  rhs.remove_prefix(1);
                  rhs.remove_suffix(1);
               }
            }
         }
      }
      if constexpr (Options.mode == parsing_mode::strict) {
         if (value_delim == '\0') {
            size_t comment_at = rhs.find_first_of(all_comment_delimiters);
            if (comment_at != std::string::npos) {
               rhs = rhs.substr(0, comment_at);
               //
               // Now we need to handle the case of "key=value   ;", such that the value is "value", 
               // by trimming whitespace to the left of the comment delimiter.
               //
               size_t re_trim = rhs.find_last_not_of(all_whitespace_chars);
               if (re_trim == std::string::npos)
                  rhs = {};
               else
                  rhs = rhs.substr(0, re_trim + 1);
            }
         }
      }

      view.remove_prefix(rhs.size());
      if (value_delim != '\0') {
         view = view.substr(2);
      }

      return key_value_pair{
         .key          = lhs,
         .between      = separator,
         .value_raw    = rhs,
         .value_delim  = value_delim,
      };
   }

   template<parsing_options Options>
   /*static*/ constexpr std::optional<char> file_line_parser<Options>::delimiter_needed_for_string(std::string_view v) {
      if (v.empty())
         return {};

      // Leading and trailing whitespace will be stripped unless we use delimiters.
      if (all_whitespace_chars.contains(v.front()) || all_whitespace_chars.contains(v.back()))
         return '"';

      if (Options.mode == parsing_mode::win32) {
         // Win32 GetPrivateProfileString strips leading and trailing quotes, if they match.
         if (all_string_delimiters.contains(v.front()) || all_string_delimiters.contains(v.back()) && v.front() == v.back())
            return v.front();
      } else {
         size_t i = v.find_first_of(all_string_delimiters);
         if (i != std::string::npos)
            return v[i];
         if (v.find_first_of(all_comment_delimiters) != std::string::npos)
            return '"';
      }

      return false;
   }
}