#include "./_all.h"

using namespace dovahkit::subsystems::worldedit::tools;

namespace {
   namespace tests::serialization_correctness {
      constexpr bool result = []() constexpr -> bool {
         bool success = all_tools::for_each_until_false<[]<typename Current>() -> bool {
            if constexpr (tool_with_options<Current>) {
               typename Current::options src = {};
               typename Current::options dst = {};

               {
                  cobb::streams::bitwriter writer = {};
                  writer.write(src);

                  cobb::streams::bitreader reader = {};
                  reader.set_buffer(writer.data(), writer.size());
                  dst.read(Current::options::serialization_version, reader);

                  if (writer.get_bitpos() != reader.get_bitpos())
                     return false;
               }

               if (src != dst)
                  return false;

               return true;
            }
            return true;
         }>();
         return success;
      }();

      static_assert(
         result,
         "No two tools can have the same serialization code."
      );
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   // Basic serialization verification checks.
}