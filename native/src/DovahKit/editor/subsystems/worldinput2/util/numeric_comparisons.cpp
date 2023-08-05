#include "./numeric_comparisons.h"

namespace dovahkit::subsystems::worldinput2::util {
   #pragma region tests
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         comparison_set<unsigned int> values;
         values.comparisons.push_back({ .op = comparison_operator::less, .comparand = 0 });
         values.normalize();

         return values.comparisons.empty();
      }(),
      "The expression (x < 0) should be considered impossible if the comparand type is unsigned."
   );
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         comparison_set<unsigned int> values;
         values.comparisons.push_back({ .op = comparison_operator::equal, .comparand = 3 });
         values.comparisons.push_back({ .op = comparison_operator::equal, .comparand = 5 });
         values.normalize();

         return values.comparisons.empty();
      }(),
      "[Inconsistent equals are impossible.] The expression (x == 3 && x == 5) should be considered impossible."
   );
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         comparison_set<unsigned int> values;
         values.comparisons.push_back({ .op = comparison_operator::equal,     .comparand = 3 });
         values.comparisons.push_back({ .op = comparison_operator::not_equal, .comparand = 5 });
         values.normalize();

         return values.comparisons.size() == 1;
      }(),
      "[Equals rendering other comparisons redundant.] The expression (x == 3 && x != 5) should normalize to (x == 3)."
   );
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         auto cnd_a = comparison_type{ .op = comparison_operator::not_equal, .comparand = 3 };
         auto cnd_b = comparison_type{ .op = comparison_operator::not_equal, .comparand = 5 };

         comparison_set<unsigned int> values;
         values.comparisons.push_back(cnd_a);
         values.comparisons.push_back(cnd_b);
         values.normalize();

         if (values.comparisons.size() != 2)
            return false;
         if (values.comparisons[0] != cnd_a)
            return false;
         if (values.comparisons[1] != cnd_b)
            return false;
         return true;
      }(),
      "The expression (x != 3 && x != 5) should be unchanged when normalizing."
   );
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         auto expected_a = comparison_type{ .op = comparison_operator::greater_or_equal, .comparand = 7 };
         auto expected_b = comparison_type{ .op = comparison_operator::less_or_equal,    .comparand = 8 };

         comparison_set<unsigned int> values;
         values.comparisons.push_back({ .op = comparison_operator::greater,   .comparand =  5 });
         values.comparisons.push_back({ .op = comparison_operator::less,      .comparand = 10 });
         values.comparisons.push_back({ .op = comparison_operator::not_equal, .comparand =  6 });
         values.comparisons.push_back({ .op = comparison_operator::not_equal, .comparand =  9 });
         values.normalize();

         if (values.comparisons.size() != 2)
            return false;
         if (values.comparisons[0] != expected_a)
            return false;
         if (values.comparisons[1] != expected_b)
            return false;
         return true;
      }(),
      "[Not-equals constraining a range.] The expression (x > 5 && x < 10 && x != 6 && x != 9) should normalize to (x >= 7 && x <= 8)."
   );
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         auto expected_a = comparison_type{ .op = comparison_operator::greater_or_equal, .comparand = 6 };
         auto expected_b = comparison_type{ .op = comparison_operator::less_or_equal,    .comparand = 9 };

         comparison_set<unsigned int> values;
         values.comparisons.push_back({ .op = comparison_operator::greater,   .comparand =  5 });
         values.comparisons.push_back({ .op = comparison_operator::less,      .comparand = 10 });
         values.comparisons.push_back({ .op = comparison_operator::not_equal, .comparand =  1 });
         values.comparisons.push_back({ .op = comparison_operator::not_equal, .comparand = 12 });
         values.normalize();

         if (values.comparisons.size() != 2)
            return false;
         if (values.comparisons[0] != expected_a)
            return false;
         if (values.comparisons[1] != expected_b)
            return false;
         return true;
      }(),
      "[Out-of-range not-equals are redundant.] The expression (x > 5 && x < 10 && x != 1 && x != 12) should normalize to (x >= 6 && x <= 9), logically equivalent to (x > 5 && x < 10)."
   );
   static_assert(
      []() -> bool {
         using comparand_type  = unsigned int;
         using comparison_type = comparison<comparand_type>;

         auto expected = comparison_type{ .op = comparison_operator::greater_or_equal, .comparand = 1 };

         comparison_set<unsigned int> values;
         values.comparisons.push_back({ .op = comparison_operator::not_equal, .comparand = 0 });
         values.normalize();

         if (values.comparisons.size() != 1)
            return false;
         if (values.comparisons[0] != expected)
            return false;
         return true;
      }(),
      "The expression (x != 0) should normalize to (x >= 1), for an unsigned integral type."
      //
      // This is an incidental side-effect of how we handle not-equals constraining a range, but it 
      // means that we avoid an unnecessary heap allocation.
   );
   #pragma endregion
}