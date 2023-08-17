#pragma once
#include <cstdint>
#include <optional>

namespace cobb::unicode {
   using character_code_t = uint32_t;

   enum class general_character_category {
      //
      // Normative:
      //
      letter_uppercase,
      letter_lowercase,
      letter_titlecase,
      mark_non_spacing,
      mark_spacing_combining,
      mark_enclosing,
      number_digit,
      number_letter,
      number_other,
      separator_space,
      separator_line,
      separator_paragraph,
      other_control,
      other_format,
      other_surrogate,
      other_private_use,
      other_not_assigned,
      //
      // Informative:
      //
      letter_modifier,
      letter_other,
      punctuation_connector,
      punctuation_dash,
      punctuation_open,
      punctuation_close,
      punctuation_quote_initial,
      punctuation_quote_final,
      punctuation_other,
      symbol_math,
      symbol_currency,
      symbol_modifier,
      symbol_other,
   };

   enum class bidirectional_character_type {
      none = -1,
      //
      l,   left_to_right        = l,
      r,   right_to_left        = r,
      al,  right_to_left_arabic = al,
      //
      en,  european_number            = en,
      es,  european_number_separator  = es,
      et,  european_number_terminator = et,
      an,  arabic_number              = an,
      cs,  common_number_separator    = cs,
      nsm, nonspacing_mark            = nsm,
      bn,  boundary_neutral           = bn,
      //
      b,  paragraph_separator = b,
      s,  segment_separator   = s,
      ws, whitespace          = ws,
      on, other_neutrals      = on,
      //
      lre, left_to_right_embedding = lre,
      lro, left_to_right_override  = lro,
      rle, right_to_left_embedding = rle,
      rlo, right_to_left_override  = rlo,
      pdf, pop_directional_format  = pdf,
      lri, left_to_right_isolate   = lri,
      rli, right_to_left_isolate   = rli,
      fsi, first_strong_isolate    = fsi,
      pdi, pop_directional_isolate = pdi,
   };

   struct unicode_character_data {
      const char* name = nullptr;
      //
      general_character_category   category_general  = general_character_category::other_not_assigned;
      unsigned int                 combining_classes = 0;
      bidirectional_character_type category_bidi     = bidirectional_character_type::none;
      //
      struct {
         std::optional<int> decimal_digit;
         std::optional<int> digit;
         std::optional<int> numeric_value;
      } numerics;
      const char* name_unicode_1 = nullptr;
      const char* comment_10646  = nullptr;
      struct {
         std::optional<character_code_t> uppercase;
         std::optional<character_code_t> lowercase;
         std::optional<character_code_t> titlecase;
      } mappings;
   };
}