#include "./category.h"
#include "./setting.h"
#include "./file.h"

#include <sstream>

namespace {
   static_assert(
      []() -> bool {
         auto file = cobb::ini::file();

         auto cat = cobb::ini::category(file, "MyCategory");

         auto dfn = cobb::ini::setting_definition<bool>{
            .name = "bTestValue",
            .initial_value = false,
         };
         if (!dfn.is_valid())
            return false;

         auto setting = cobb::ini::setting::define<cobb::ini::setting_definition<bool>{
            .name = "bTestValue",
            .initial_value = false,
         }>(cat);

         return true;
      }(),
      ""
   );
   
   static_assert(
      []() -> bool {
         auto file = cobb::ini::file();

         auto cat = cobb::ini::category(file, "MyCategory");

         auto setting = cobb::ini::setting::define<cobb::ini::setting_definition<bool>{
            .name = "bTestValue",
            .initial_value = true,
         }>(cat);

         file.load(R"--([MyCategory]
bTestValue=false )--");

         return setting.get_current_value<bool>() == false;
      }(),
      ""
   );
}

#include <sstream>

#if _DEBUG && defined(_MSC_VER)
   //
   // August 13, 2023: constinit + STL containers is broken in MSVC Debug builds.
   // https://developercommunity.visualstudio.com/t/MDd-makes-it-impossible-to-have-constin/10439085
   // 
   // For my particular use case, the static initialization order fiasco shouldn't 
   // be a risk because I intend to define an INI file and its contents inside of 
   // a single translation unit, but there's other jank associated with static vars 
   // that I would've liked to avoid.
   //
   #define constinit static
#endif
namespace {
   namespace _TestINIFile {
      constinit cobb::ini::file file_object = cobb::ini::file();

      namespace TestCategory {
         constinit cobb::ini::category category_object = cobb::ini::category(file_object, "TestCategory");

         constinit cobb::ini::setting bMySetting = cobb::ini::setting::define<cobb::ini::setting_definition<bool>{
            .name = "bMySetting",
            .initial_value = false,
         }>(category_object);
      }
   }
}

// And now go back to hell where you belong, o foul macro.
#undef constinit
