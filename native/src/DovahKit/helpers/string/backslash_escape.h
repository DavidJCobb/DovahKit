#pragma once
#include <string>
#include <string_view>

namespace cobb {
   enum backslash_escape_type {

      // Mostly follows C++ escape sequences, with some exceptions:
      //   
      //  - Named escape sequences (slash-u) are not supported, as embedding the entire 
      //    Unicode character table constexpr will make nearly all compilers explode. 
      //    (Why "slash-u"? IntelliSense tooltips break on the mere mention of them!)
      //    
      //  - Octal escape sequences (e.g. \012) are not supported, because screw octal.
      //
      cpp,

      javascript,
   };

   // Assumes UTF-8 text. If `delim` is not null, then it will always be escaped.
   template<backslash_escape_type Mode = backslash_escape_type::cpp>
   constexpr std::string backslash_escape(std::string_view src, char delim = '\0');

   // Assumes UTF-8 text. Throws on encountering bad escape sequences.
   template<backslash_escape_type Mode = backslash_escape_type::cpp>
   constexpr std::string backslash_unescape(std::string_view src);
}

#include "./backslash_escape.inl"