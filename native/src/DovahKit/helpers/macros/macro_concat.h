
#pragma push_macro("MACRO_CONCAT_IMPL")
#pragma push_macro("MACRO_CONCAT")
#define MACRO_CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) MACRO_CONCAT_IMPL(x, y)