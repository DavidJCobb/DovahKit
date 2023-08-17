// for use on https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
// given info at https://www.unicode.org/L2/L1999/UnicodeData.html

// Unusable in IntelliSense. Resulting file is 191992 lines long, consisting of definitions for 
// a single array containing some 1114111 entries, annihilating my system RAM and CPU and making 
// my computer nigh-unusable. IntelliSense itself runs into and reports out-of-memory errors.
//
// I wanted to keep any codegen *outside* of C++ to a minimum -- feed source data into constexpr 
// code and generate as much as possible there. Not even remotely feasible.

(function() {
   const WE_CARE_ABOUT_UNICODE_1_NAMES = false;
   const WE_CARE_ABOUT_COMMENT_10646   = false;
   
   let chars  = [];
   let ranges = [];
   
   class UnicodeChar {
      constructor(obj) {
         this.name          = null;
         this.category_gen  = null;
         this.combining     = null;
         this.category_bidi = null;
         this.decomposition = null;
         this.decimal_digit = null;
         this.digit_value   = null;
         this.numeric_value = null;
         this.mirrored      = null;
         if (WE_CARE_ABOUT_UNICODE_1_NAMES) {
            this.uni_1_name = null;
         }
         if (WE_CARE_ABOUT_COMMENT_10646) {
            this.comment_10646 = null;
         }
         this.uppercase_map = null;
         this.lowercase_map = null;
         this.titlecase_map = null;
         Object.seal(this);
         
         if (obj) {
            for(let key of Object.keys(obj))
               if (this.hasOwnProperty(key))
                  this[key] = obj[key];
            for(let key of ["decimal_digit", "digit_value", "numeric_value"])
               if (this[key] === "" || isNaN(+this[key]))
                  this[key] = null;
         }
      }
      
      isNumeric() {
         for(let key of ["decimal_digit", "digit_value", "numeric_value"])
            if (this[key] !== null)
               return true;
         return false;
      }
      
      equals(other) {
         if (!other)
            return false;
         if (this === other)
            return true;
         for(let key of Object.keys(this))
            if (other[key] != this[key])
               return false;
         return true;
      }
   };

   let lines = document.documentElement.innerText.split("\n");
   for(let line of lines) {
      line = line.split(";");
      if (!line[1] && line[1] !== "")
         continue;

      let code = parseInt(line[0], 16);
      
      function _fix_info(info) {
         if (info.combining === "")
            info.combining = null;
         (["decimal_digit", "digit_value", "numeric_value"]).forEach(function(e) {
            let field = info[e];
            if (field === "")
               info[e] = null;
            else
               info[e] = +field;
         });
         (["uppercase_map", "lowercase_map", "titlecase_map"]).forEach(function(e) {
            if (info[e] === "")
               info[e] = null;
            else
               info[e] = parseInt(info[e], 16);
         });
         return info;
      }
      
      {
         let name = line[1];
         let data = name.match(/^\<(.*), (First|Last)\>$/);
         if (data) {
            if (data[2] == "Last") {
               ranges[ranges.length - 1].max = code;
               continue;
            }
            ranges.push({
               min: code,
               data: new UnicodeChar(_fix_info({
                  name:          data[1] + " (Range)",
                  category_gen:  line[2],
                  combining:     line[3],
                  category_bidi: line[4],
                  decomposition: line[5],
                  decimal_digit: line[6],
                  digit_value:   line[7],
                  numeric_value: line[8],
                  mirrored:      line[9],
                  uni_1_name:    line[10],
                  comment_10646: line[11],
                  uppercase_map: line[12],
                  lowercase_map: line[13],
                  titlecase_map: line[14],
               })),
            });
            continue;
         }
      }
      
      let info = new UnicodeChar(_fix_info({
         name:          line[1],
         category_gen:  line[2],
         combining:     line[3],
         category_bidi: line[4],
         decomposition: line[5],
         decimal_digit: line[6],
         digit_value:   line[7],
         numeric_value: line[8],
         mirrored:      line[9],
         uni_1_name:    line[10],
         comment_10646: line[11],
         uppercase_map: line[12],
         lowercase_map: line[13],
         titlecase_map: line[14]
      }));
      chars[code] = info;
   }
   
   for(let range of ranges) {
      for(let i = range.min; i <= range.max; ++i)
         chars[i] = range.data;
   }
   
   let CATEGORIES = {
      "Lu": "letter_uppercase",
      "Ll": "letter_lowercase",
      "Lt": "letter_titlecase",
      "Mn": "mark_non_spacing",
      "Mc": "mark_spacing_combining",
      "Me": "mark_enclosing",
      "Nd": "number_digit",
      "Nl": "number_letter",
      "No": "number_other",
      "Zs": "separator_space",
      "Zl": "separator_line",
      "Zp": "separator_paragraph",
      "Cc": "other_control",
      "Cf": "other_format",
      "Cs": "other_surrogate",
      "Co": "other_private_use",
      "Cn": "other_not_assigned",
      //
      // Informative:
      //
      "Lm": "letter_modifier",
      "Lo": "letter_other",
      "Pc": "punctuation_connector",
      "Pd": "punctuation_dash",
      "Ps": "punctuation_open",
      "Pe": "punctuation_close",
      "Pi": "punctuation_quote_initial",
      "Pf": "punctuation_quote_final",
      "Po": "punctuation_other",
      "Sm": "symbol_math",
      "Sc": "symbol_currency",
      "Sk": "symbol_modifier",
      "So": "symbol_other",
   };
   
   let out = `#pragma once
#include <array>
#include "./character_metadata_types.h"

namespace cobb::unicode {`;
   
   out += `   namespace impl {\n`;
   out += `      constexpr const auto private_use_character_info = unicode_character_data{\n`;
   out += `         .category_general  = general_character_category::other_control,\n`;
   out += `         .combining_classes = 0,\n`;
   out += `         .category_bidi     = bidirectional_character_type::bn,\n`;
   out += `      };\n`;
   out += `   }\n`;
   out += "   \n";
   
   function _to_charcode(n) {
      return "0x" + n.toString(16).toUpperCase().padStart(6, "0");
   }
   
   out += `   constexpr const auto all_characters = []() {\n`;
   out += `      std::array<unicode_character_data, ${_to_charcode(chars.length)}> out = {};\n`;
   for(let i = 0; i < chars.length; ++i) {
      let ch = chars[i];
      if (!ch)
         continue;
      
      // Shortcut for private-use characters.
      if (ch.category_gen == "Co") {
         let max;
         for(max = i + 1; max < chars.length; ++max)
            if (!chars[max] || chars[max].category_gen != "Co")
               break;
         
         out += "      \n";
         out += `      for(size_t i = ${_to_charcode(i)}; i < ${_to_charcode(max)}; ++i)\n`;
         out += `         out[i] = impl::private_use_character_info;\n`;
         out += "      \n";
         
         i = max - 1;
         continue;
      }
      
      function _stringify_char_info(ch, indent) {
         let out = `{\n`;
         out += `${indent}   .name = "${ch.name.replaceAll('"', '\\"')}",\n`;
         out += `${indent}   .category_general  = general_character_category::${CATEGORIES[ch.category_gen || "Cn"]},\n`;
         if (ch.combining !== null && ch.combining != 0)
            out += `${indent}   .combining_classes = ${ch.combining},\n`;
         if (ch.category_bidi)
            out += `${indent}   .category_bidi     = bidirectional_character_type::${ch.category_bidi.toLowerCase()},\n`;
         if (ch.isNumeric()) {
            out += `${indent}   .numerics = {\n`;
            if (ch.decimal_digit !== null)
               out += `${indent}      .decimal_digit = ${ch.decimal_digit},\n`;
            if (ch.digit_value !== null)
               out += `${indent}      .digit         = ${ch.digit_value},\n`;
            if (ch.numeric_value !== null)
               out += `${indent}      .numeric_value = ${ch.numeric_value},\n`;
            out += `${indent}   },\n`;
         }
         if (WE_CARE_ABOUT_UNICODE_1_NAMES) {
            if (ch.uni_1_name)
               out += `${indent}   .name_unicode_1 = "${ch.uni_1_name}"\n`;
         }
         if (WE_CARE_ABOUT_COMMENT_10646) {
            if (ch.comment_10646)
               out += `${indent}   .comment_10646 = "${ch.comment_10646}"\n`;
         }
         if (ch.uppercase_map !== null || ch.lowercase_map !== null || ch.titlecase_map !== null) {
            out += `${indent}   .mappings = {\n`;
            if (ch.uppercase_map !== null)
               out += `${indent}      .uppercase = ${_to_charcode(ch.uppercase_map)},\n`;
            if (ch.lowercase_map !== null)
               out += `${indent}      .lowercase = ${_to_charcode(ch.lowercase_map)},\n`;
            if (ch.titlecase_map !== null)
               out += `${indent}      .titlecase = ${_to_charcode(ch.titlecase_map)},\n`;
            out += `${indent}   },\n`;
         }
         out += `${indent}}`;
         return out;
      }
      
      // Sometimes, consecutive characters have exactly equal data, as in the case of many 
      // control characters.
      if (ch.equals(chars[i + 1])) {
         let j = i + 2;
         for(; j < chars.length; ++j)
            if (!ch.equals(chars[j]))
               break;
         
         if (j > i + 2) {
            out += "      \n";
            out += `      for(size_t i = ${_to_charcode(i)}; i < ${_to_charcode(j)}; ++i) {\n`;
            out += `         out[i] = ${_stringify_char_info(ch, "         ")};\n`;
            out += `      }`;
            out += "      \n";
         } else {
            out += `      out[${_to_charcode(i)}] = ${_stringify_char_info(ch, "      ")};\n`;
            for(let k = i + 1; k < j; ++k)
               out += `      out[${_to_charcode(k)}] = out[${_to_charcode(i)}];\n`;
         }
         i = j - 1;
         continue;
      }
      
      out += `      out[${_to_charcode(i)}] = ${_stringify_char_info(ch, "      ")};\n`;
   }
   out += `      return out;\n`;
   out += `   }();\n`;
   
   out += `}`;
   
   return out;
})();
