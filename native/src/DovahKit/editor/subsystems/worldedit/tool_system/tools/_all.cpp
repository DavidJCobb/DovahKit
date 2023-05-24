#include "./_all.h"
#include "helpers/bitstreams/round_trip_test.h"

using namespace dovahkit::subsystems::worldedit::tools;

namespace {
   namespace tests::serialization_correctness {
      constexpr bool result = []() constexpr -> bool {
         constexpr bool success = all_tools::for_each_until_false<[]<typename Current>() -> bool {
            if constexpr (tool_with_options<Current>) {
               return cobb::bitstreams::round_trip_test<Current::options>;
            }
            return true;
         }>();
         return success;
      }();

      static_assert(
         result,
         "One of the tools here has broken serialization code."
      );
   }
}