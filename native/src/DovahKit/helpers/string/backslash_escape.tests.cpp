#include "./backslash_escape.h"

#pragma region backslash_escape
static_assert(cobb::backslash_escape("abc") == "abc");

static_assert(cobb::backslash_escape("abc\fd") == "abc\\fd");
static_assert(cobb::backslash_escape("abc\nd") == "abc\\nd");
static_assert(cobb::backslash_escape("abc\rd") == "abc\\rd");
static_assert(cobb::backslash_escape("abc\td") == "abc\\td");
static_assert(cobb::backslash_escape("abc\vd") == "abc\\vd");

// Explicit length needed to avoid the null being mistaken for a terminator in this test:
static_assert(cobb::backslash_escape(std::string_view("abc" "\0" "d", 5)) == "abc\\0d");

// Test C++ escapes for non-printable characters.
// NOTE: C++ allows \x... sequences to have an arbitrary number of digits. Only safe way to handle this is braces.
static_assert(cobb::backslash_escape("abc" "\x01" "d") == "abc\\x{01}d");
static_assert(cobb::backslash_escape("abc" "\x02" "d") == "abc\\x{02}d");

// C++: Test non-"printable" glyph needing more than two digits:
static_assert([]() -> bool {
   std::string test = "abc";
   cobb::utf8::append(test, 0x200D); // ZERO-WIDTH JOINER
   test += "d";

   return cobb::backslash_escape(test) == "abc\\x{200D}d";
}());

// Test JD escapes for non-printable characters.
static_assert(cobb::backslash_escape<cobb::backslash_escape_type::javascript>("abc" "\x01" "d") == "abc\\x01d");
static_assert(cobb::backslash_escape<cobb::backslash_escape_type::javascript>("abc" "\x02" "d") == "abc\\x02d");

// JS: Test non-"printable" glyph needing more than two digits:
static_assert([]() -> bool {
   std::string test = "abc";
   cobb::utf8::append(test, 0x200D); // ZERO-WIDTH JOINER
   test += "d";

   return cobb::backslash_escape<cobb::backslash_escape_type::javascript>(test) == "abc\\u200Dd";
}());
#pragma endregion

#pragma region backslash_unescape
static_assert(cobb::backslash_unescape("abc\\x{1}d") == "abc" "\x0001" "d");
static_assert(cobb::backslash_unescape("abc\\x{000001}d") == "abc" "\x0001" "d");

static_assert([]() -> bool {
   std::string escaped = "abc\\x{FFFD}d";

   std::string desired = "abc";
   cobb::utf8::append(desired, 0xFFFD); // ZERO-WIDTH JOINER
   desired += "d";

   return cobb::backslash_unescape(escaped) == desired;
}());

// JS
static_assert([]() -> bool {
   std::string escaped = "abc\\x12d";
   std::string desired = "abc" "\x12" "d";
   return cobb::backslash_unescape<cobb::backslash_escape_type::javascript>(escaped) == desired;
}());
static_assert([]() -> bool {
   std::string escaped = "abc\\uFFFDd";

   std::string desired = "abc";
   cobb::utf8::append(desired, 0xFFFD); // ZERO-WIDTH JOINER
   desired += "d";

   return cobb::backslash_unescape<cobb::backslash_escape_type::javascript>(escaped) == desired;
}());
static_assert([]() -> bool {
   std::string escaped = "abc\\u{FFFD}d";

   std::string desired = "abc";
   cobb::utf8::append(desired, 0xFFFD); // ZERO-WIDTH JOINER
   desired += "d";

   return cobb::backslash_unescape<cobb::backslash_escape_type::javascript>(escaped) == desired;
}());
#pragma endregion