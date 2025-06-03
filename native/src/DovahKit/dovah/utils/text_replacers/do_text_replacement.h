#pragma once
#include <optional>
#include <string>
#include <string_view>

namespace dovah::text_replacers {
   template<typename TokenHandler>
   constexpr std::string do_text_replacement(std::string_view src, TokenHandler& handler) {
      size_t tag_start = src.find('<');
      if (tag_start == std::string::npos) {
         return std::string(src);
      }
      std::string dst;
      size_t search_from = 0;
      do {
         size_t tag_end = src.find('>', search_from);
         if (tag_end != std::string::npos) {
            std::string_view tag_body = src.substr(tag_start + 1, tag_end - tag_start - 1);
            std::string_view tag_name = tag_body;
            std::string_view subtoken;
            std::string_view argument;
            {
               size_t i = tag_body.find('.');
               size_t j = tag_body.find('=');
               if (i != std::string::npos) {
                  if (j != std::string::npos && j > i)
                     subtoken = tag_body.substr(i + 1, j - (i + 1));
                  else
                     subtoken = tag_body.substr(i + 1);
               }
               if (j != std::string::npos) {
                  if (i != std::string::npos)
                     //
                     // The way the game extracts the tag and subtag is such that while 
                     // the intended syntax is <Tag.SubtagCap=Argument>, you can also do 
                     // <Tag=Argument.SubtagCap>.
                     //
                     argument = tag_body.substr(j + 1, i - (j + 1));
                  else
                     argument = tag_body.substr(j + 1);
               }
               if (i != std::string::npos || j != std::string::npos) {
                  tag_name = tag_name.substr(0, std::min(i, j));
               }
            }
            bool capitalize = subtoken.ends_with("Cap");
            if (capitalize) {
               subtoken.remove_suffix(3);
            }

            std::optional<std::string> replacement = handler.try_token_substitution(tag_name, subtoken, argument);
            if (replacement.has_value()) {
               dst += src.substr(search_from, tag_start - search_from);
               if (capitalize) {
                  if (!(*replacement).empty()) {
                     char& leading = (*replacement)[0];
                     leading = toupper(leading);
                  }
               }
               dst += *replacement;
               search_from = tag_end + 1;
               continue;
            }
         }
         //
         // Copy everything from `search_from` up to and including the found
         // '<', and then move on to the next '<' and see if that forms a 
         // valid substitution.
         //
         dst += src.substr(search_from, tag_start - search_from);
         ++tag_start;
         search_from = tag_start;
      } while ((tag_start = src.find('<', search_from)) != std::string::npos);
      dst += src.substr(search_from);
      return dst;
   }
}