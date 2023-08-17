#include "./file_line_parser.h"

namespace {
   using parser_type = cobb::ini::file_line_parser<cobb::ini::parsing_options{}>;

   using win32_parser_type = cobb::ini::file_line_parser<cobb::ini::win32_parsing_options>;

   // Ensure leading and trailing whitespace is separated properly.
   static_assert(parser_type{ "  [myCategory]    " }.parse_line().leading == "  ");
   static_assert(parser_type{ "  [myCategory]    " }.parse_line().trailing == "    ");
   static_assert(parser_type{ "  key=value    " }.parse_line().leading == "  ");
   static_assert(parser_type{ "  key=value    " }.parse_line().trailing == "    ");
   static_assert(parser_type{ "  key    =  value    " }.parse_line().leading == "  ");
   static_assert(parser_type{ "  key    =  value    " }.parse_line().trailing == "    ");

   // Ensure trailing whitespace *and comments* are separated from the main content properly.
   static_assert(parser_type{ "[myCategory];abc" }.parse_line().trailing == ";abc");
   static_assert(parser_type{ "key=value;abc" }.parse_line().trailing == ";abc");
   //
   static_assert(parser_type{ "[myCategory]   ;abc" }.parse_line().trailing == "   ;abc");
   static_assert(parser_type{ "key=value   ;abc" }.parse_line().trailing == "   ;abc");
   //
   static_assert(parser_type{ "  [myCategory];abc" }.parse_line().trailing == ";abc");
   static_assert(parser_type{ "  key=value;abc" }.parse_line().trailing == ";abc");
   //
   static_assert(parser_type{ "  [myCategory]    ;abc" }.parse_line().trailing == "    ;abc");
   static_assert(parser_type{ "  key=value    ;abc" }.parse_line().trailing == "    ;abc");
   static_assert(parser_type{ "  key    =  value    ;abc" }.parse_line().trailing == "    ;abc");

   //
   // Tests below are for specific syntax elements. These tests should not include 
   // leading whitespace: the code for parsing these elements assumes that leading 
   // whitespace has already been stripped away (which will be the case when they're 
   // invoked as part of `parse_line`) and will fail upon seeing it.
   //

   // Test category parsing.
   static_assert(parser_type{ "[]" }.try_extract_category_start().value().name == "");
   static_assert(parser_type{ "[myCategory]" }.try_extract_category_start().value().name == "myCategory");
   static_assert(parser_type{ "[ myCategory  ]" }.try_extract_category_start().value().leading == " ");
   static_assert(parser_type{ "[ myCategory  ]" }.try_extract_category_start().value().trailing == "  ");
   static_assert([]() -> bool {
      //
      // Win32 GetPrivateProfileString allows semicolons inside of category names, even 
      // though some parsers treat those as comment markers.
      //
      auto  parsed  = win32_parser_type{ "[my;category]" }.parse_line();
      auto& content = std::get<cobb::ini::file_line_parse_results::category_start>(parsed.content);
      return content.name == "my;category";
   }());
   static_assert([]() -> bool {
      //
      // Win32 GetPrivateProfileString ignores trailing garbage after a category header. 
      // It is ignored even if it looks like a key/value pair.
      //
      auto  parsed  = win32_parser_type{ "[myCategory] trailing-garbage" }.parse_line();
      auto& content = std::get<cobb::ini::file_line_parse_results::category_start>(parsed.content);
      return content.name == "myCategory";
   }());

   // Test invalid category lines.
   static_assert(parser_type{ "[myCategory" }.try_extract_category_start().has_value() == false);
   static_assert(parser_type{ "[myCategory;]" }.try_extract_category_start().has_value() == false); // non-win32 parser modes should consider this ill-formed

   //

   // Test key/value pair parsing.
   static_assert([]() -> bool {
      auto result = parser_type{ "key=value" }.try_extract_key_value_pair();
      return result.value() == cobb::ini::file_line_parse_results::key_value_pair{
         .key       = "key",
         .between   = "=",
         .value_raw = "value"
      };
   }());
   static_assert([]() -> bool {
      auto result = parser_type{ "key  =    value" }.try_extract_key_value_pair();
      return result.value() == cobb::ini::file_line_parse_results::key_value_pair{
         .key       = "key",
         .between   = "  =    ",
         .value_raw = "value"
      };
   }());
   static_assert([]() -> bool {
      auto result = parser_type{ "key = \"  value    \"" }.try_extract_key_value_pair();
      return result.value() == cobb::ini::file_line_parse_results::key_value_pair{
         .key         = "key",
         .between     = " = ",
         .value_raw   = "  value    ",
         .value_delim = '"',
      };
   }());
   static_assert([]() -> bool {
      using test_type = cobb::ini::file_line_parse_results::key_value_pair;
      //
      auto  parsed  = parser_type{ "key = \";\"" }.parse_line();
      return std::get<test_type>(parsed.content) == test_type{
         .key         = "key",
         .between     = " = ",
         .value_raw   = ";",
         .value_delim = '"',
      };
   }());
   
   // Test: key/value pair with unbalanced quotation marks or multiple sets thereof: permit this; just don't strip quotes.
   static_assert([]() -> bool {
      auto result = parser_type{ "key=\"1\",\"2\",\"3\"" }.try_extract_key_value_pair();
      return result.value() == cobb::ini::file_line_parse_results::key_value_pair{
         .key         = "key",
         .between     = "=",
         .value_raw   = "\"1\",\"2\",\"3\"",
      };
   }());
   
   // Test key/value pair with empty value.
   static_assert([]() -> bool {
      auto result = parser_type{ "key=" }.try_extract_key_value_pair();
      return result.value() == cobb::ini::file_line_parse_results::key_value_pair{
         .key       = "key",
         .between   = "=",
         .value_raw = ""
      };
   }());
   static_assert([]() -> bool {
      auto result = parser_type{ "key=     " }.try_extract_key_value_pair();
      return result.value() == cobb::ini::file_line_parse_results::key_value_pair{
         .key       = "key",
         .between   = "=     ", // it is expected, if unintuitive, behavior that the whitespace in this case be lumped in with the delimiter
         .value_raw = ""
      };
   }());

   // Test invalid key/value pair lines.
   static_assert(parser_type{ "key" }.try_extract_key_value_pair().has_value() == false);
   static_assert(parser_type{ "=value" }.try_extract_key_value_pair().has_value() == false);
   static_assert(parser_type{ "   =value" }.try_extract_key_value_pair().has_value() == false);

   //
   // Tests below are for roundtripping capacity.
   //
   
   static_assert([]() -> bool {
      std::string src = " key  =   value    ;     comment";

      auto parsed = parser_type{ src }.parse_line();

      std::string dst;
      dst += parsed.leading;
      {
         auto& data = std::get<parser_type::key_value_pair>(parsed.content);
         dst += data.key;
         dst += data.between;
         if (data.value_delim)
            dst += data.value_delim;
         dst += data.value_raw;
         if (data.value_delim)
            dst += data.value_delim;
      }
      dst += parsed.trailing;

      return dst == src;
   }());
   static_assert([]() -> bool {
      std::string src = " key  =   'value'    ;     comment";

      auto parsed = parser_type{src}.parse_line();

      std::string dst;
      dst += parsed.leading;
      {
         auto& data = std::get<parser_type::key_value_pair>(parsed.content);
         dst += data.key;
         dst += data.between;
         if (data.value_delim)
            dst += data.value_delim;
         dst += data.value_raw;
         if (data.value_delim)
            dst += data.value_delim;
      }
      dst += parsed.trailing;

      return dst == src;
   }());
   static_assert([]() -> bool {
      std::string src = " key  =\t\"1\",\"2\",\"3\"   ;     comment";

      auto parsed = parser_type{ src }.parse_line();

      std::string dst;
      dst += parsed.leading;
      {
         auto& data = std::get<parser_type::key_value_pair>(parsed.content);
         dst += data.key;
         dst += data.between;
         if (data.value_delim)
            dst += data.value_delim;
         dst += data.value_raw;
         if (data.value_delim)
            dst += data.value_delim;
      }
      dst += parsed.trailing;

      return dst == src;
   }());
}