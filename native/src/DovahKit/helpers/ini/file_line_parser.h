#pragma once
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace cobb::ini {
   enum class parsing_mode {
      strict,
      win32,
   };

   struct parsing_options {
      parsing_mode mode = parsing_mode::strict;
      struct {
         char delimiters[8] = { ';', 0, 0, 0, 0, 0, 0, 0 };
      } line_comments;
      char string_delimiters[8] = { '"', '\'', 0, 0, 0, 0, 0, 0};
   };
   //
   static constexpr const parsing_options default_parsing_options = parsing_options{};
   static constexpr const parsing_options win32_parsing_options = parsing_options{
      .mode = cobb::ini::parsing_mode::win32,
   };

   namespace file_line_parse_results {
      struct category_start {
         constexpr bool operator==(const category_start&) const = default;
         std::string_view leading;
         std::string_view name;
         std::string_view trailing;
      };
      struct key_value_pair {
         constexpr bool operator==(const key_value_pair&) const = default;
         std::string_view key;
         std::string_view between;
         std::string_view value_raw;
         char value_delim = '\0'; // if value is a quoted string, this is the quote char seen
      };
      struct no_op_line {
         constexpr bool operator==(const no_op_line&) const = default;
      };
      struct ill_formed_line {
         constexpr bool operator==(const ill_formed_line&) const = default;
      };

      using line_content = std::variant<
         std::monostate,
         category_start,
         key_value_pair,
         no_op_line,
         ill_formed_line
      >;

      struct line {
         constexpr bool operator==(const line&) const = default;
         std::string_view leading;
         line_content     content;
         std::string_view trailing;
      };
   }

   template<parsing_options Options>
   class file_line_parser {
      public:
         static constexpr const auto all_whitespace_chars   = std::string_view(" \t");
         static constexpr const auto all_comment_delimiters = std::string_view(Options.line_comments.delimiters);
         static constexpr const auto all_string_delimiters  = std::string_view(Options.string_delimiters);

         using category_start  = file_line_parse_results::category_start;
         using key_value_pair  = file_line_parse_results::key_value_pair;
         using no_op_line      = file_line_parse_results::no_op_line;
         using ill_formed_line = file_line_parse_results::ill_formed_line;
         
         using line_content = file_line_parse_results::line_content;
         using line = file_line_parse_results::line;

      protected:
         std::string_view view;

      public:
         constexpr file_line_parser(std::string_view line) : view(line) {}

         [[nodiscard]] constexpr line parse_line();

         [[nodiscard]] constexpr std::optional<category_start> try_extract_category_start();
         [[nodiscard]] constexpr std::optional<key_value_pair> try_extract_key_value_pair();

         static constexpr std::optional<char> delimiter_needed_for_string(std::string_view);
   };

   using default_file_line_parser = file_line_parser<default_parsing_options>;
}

#include "./file_line_parser.inl"