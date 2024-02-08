#include "./interned_string_table.h"

namespace {
   struct _ascii_case_folded_test : cobb::interned_string_tables::default_parameters {
      static constexpr void fold(std::string& f) {
         for (auto& c : f)
            if (c >= 'A' && c <= 'Z')
               c += 0x20;
      };
      static constexpr bool are_equal(const view_type& a, const view_type& b) {
         size_t size = a.size();
         if (size != b.size())
            return false;
         for (size_t i = 0; i < size; ++i) {
            auto ac = a[i];
            auto bc = b[i];
            if (ac == bc)
               continue;
            if (ac >= 'a' && ac <= 'z')
               ac -= 0x20;
            if (bc >= 'a' && bc <= 'z')
               bc -= 0x20;
            if (ac != bc)
               return false;
         }
         return true;
      }
   };
   
   struct _ascii_case_folded_not_initial_test : public _ascii_case_folded_test {
      static constexpr const bool fold_on_store = false;
   };

   static_assert(
      []() -> bool {
         cobb::interned_string_table<> table;
         table.get_or_insert("Hello!");
         table.get_or_insert("hello!");
         return table.count_stored() == 2;
      }(),
      "This test should contain two interned strings."
   );
   static_assert(
      []() -> bool {
         cobb::interned_string_table<_ascii_case_folded_test> table;
         table.get_or_insert("Hello!");
         table.get_or_insert("hello!");
         return table.count_stored() == 1;
      }(),
      "This case-folding test should contain a single interned string."
   );

   static_assert(
      []() -> bool {
         cobb::interned_string_table<_ascii_case_folded_test> table;
         auto a = table.get_or_insert("Hello!");
         auto b = table.get_or_insert("hello!");
         return a == b && a == "hello!";
      }(),
      "Repeated accesses to a folded string should retrieve equivalent results, and strings folded on initial store should be retrieved as folded."
   );
   static_assert(
      []() -> bool {
         cobb::interned_string_table<_ascii_case_folded_not_initial_test> table;
         auto a = table.get_or_insert("Hello!");
         auto b = table.get_or_insert("hello!");
         return a == b && a == "Hello!";
      }(),
      "Repeated accesses to a folded string should retrieve equivalent results, and strings not folded on initial store should be retrieved as initially stored."
   );
   
   static_assert(
      []() -> bool {
         cobb::interned_string_table<> table;
         table.get_or_insert("Hello!");
         return table.contains("Hello!") && !table.contains("hello!");
      }()
   );
   static_assert(
      []() -> bool {
         cobb::interned_string_table<_ascii_case_folded_test> table;
         table.get_or_insert("Hello!");
         return table.contains("Hello!") && table.contains("hello!");
      }()
   );
}