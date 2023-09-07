#pragma once
#include <limits>
#include <optional>
#include <vector>
#include "../enums/comparison_operator.h"

namespace dovahkit::subsystems::worldinput::util {
   template<typename ComparandType>
   struct comparison {
      using comparand_type = ComparandType;

      comparison_operator op        = comparison_operator::equal;
      comparand_type      comparand = {};

      constexpr bool operator==(const comparison&) const noexcept = default;

      constexpr bool test(comparand_type v) const;
   };

   // A set of AND-linked numeric comparisons.
   // An empty comparison set is considered impossible.
   template<typename ComparandType>
   struct comparison_set {
      public:
         using comparand_type  = ComparandType;
         using comparison_type = comparison<comparand_type>;

      protected:
         struct _normalized {
            using inclusive_bound_bool = std::conditional_t<
               std::is_integral_v<comparand_type>,
               const bool,
               bool
            >;

            std::optional<comparand_type> must_equal;
            std::vector<comparand_type>   must_not_equal;
            comparand_type       min        = std::numeric_limits<comparand_type>::lowest();
            comparand_type       max        = std::numeric_limits<comparand_type>::max();
            inclusive_bound_bool can_be_min = true;
            inclusive_bound_bool can_be_max = true;
            //
            bool impossible = false;

            constexpr bool range_is_possible() const; // tests min/max
            constexpr void sort_not_equals();

            // Add constraints:

            constexpr void process_must_equal(comparand_type);
            constexpr void process_must_not_equal(comparand_type);
            constexpr void process_must_greater(comparand_type);
            constexpr void process_must_greater_or_equal(comparand_type);
            constexpr void process_must_less(comparand_type);
            constexpr void process_must_less_or_equal(comparand_type);

            // After:

            constexpr void limit_range_by_not_equals(); // ensure `must_not_equal` list is sorted first
         };

      public:
         std::vector<comparison_type> comparisons;

         // If it's impossible for any number to pass the comparison set, then the set is emptied.
         // If any comparisons are `==`, then the result will contain only that one comparison.
         // The result will have at most one `>` or `>=` comparison.
         // The result will have at most one `<` or `<=` comparison.
         // If the comparand type is integral, the result will always use `>=` and `<=` rather than `>` and `<`.
         // When the result has multiple comparisons, they come in the following order: > or >=; < or <=; !=
         constexpr void normalize();

         constexpr bool test(comparand_type value) const;

         comparison_set& operator&=(const comparison_set&);

         constexpr bool operator==(const comparison_set&) const noexcept = default;
   };
}

#include "./numeric_comparisons.inl"