
#define FAIL_ON_EXPANDOS(L, pos, ...) \
   { \
      auto pair = table_contains_expandos((L), (pos), std::to_array<std::string_view>({ __VA_ARGS__ })); \
      if (pair.first) { \
         if (!pair.second.empty()) { \
            FAIL(std::format("table contains one or more unexpected keys (first seen: `{}`)", pair.second)); \
         } \
         FAIL("table contains one or more unexpected keys"); \
      } \
   }
   