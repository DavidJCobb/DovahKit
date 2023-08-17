#pragma once
#include "helpers/ini/file.h"
#include "helpers/ini/category.h"
#include "helpers/ini/setting.h"
#include "helpers/ini/setting_definition.h"

#if _DEBUG && defined(_MSC_VER)
   //
   // August 13, 2023: constinit + STL containers is broken in MSVC Debug builds.
   // https://developercommunity.visualstudio.com/t/MDd-makes-it-impossible-to-have-constin/10439085
   // I will forever remain impressed at present-day Microsoft's ability to never 
   // actually finish making things before publishing them.
   // 
   // For my particular use case, the static initialization order fiasco shouldn't 
   // be a risk because I intend to define an INI file and its contents inside of 
   // a single translation unit, but there's other jank associated with static vars 
   // that I would've liked to avoid.
   // 
   // Remove this hacky garbage as soon as Microsoft fixes things on their end!
   //
   #define constinit static
#endif

#pragma push_macro("MAKE_INI_SETTING")

#define MAKE_INI_SETTING(setting_name, initial) \
   constinit auto setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::decay_t<decltype(initial)>>{ \
      .name          = #setting_name, \
      .initial_value = initial \
   }>(category_data);

namespace dovahkit::ini::main {
   constinit cobb::ini::file file_data = cobb::ini::file{};

   namespace worldedit {
      constinit auto category_data = cobb::ini::category(file_data, "worldedit");

      MAKE_INI_SETTING(fCameraSpeedNormal,        (double)180.0);
      MAKE_INI_SETTING(fCameraSpeedMultBoost,     (double)2.0);
      MAKE_INI_SETTING(fCameraSpeedMultPrecision, (double)0.3);
      //
      MAKE_INI_SETTING(iLoadedGridSize, 5);
      MAKE_INI_SETTING(bLoadedGridSizeOverrideFromSkyrimINI, true);
   }
   namespace worldinput {
      constinit auto category_data = cobb::ini::category(file_data, "worldinput");

      MAKE_INI_SETTING(fTurnSpeedDegreesPerSecondX, (double)90.0);
      MAKE_INI_SETTING(fTurnSpeedDegreesPerSecondY, (double)90.0);

      MAKE_INI_SETTING(bInvertLookX, false);
      MAKE_INI_SETTING(bInvertLookY, true);
   }
}
#undef MAKE_INI_SETTING
#pragma pop_macro("MAKE_INI_SETTING")

// Remove this when we remove the MSVC Debug hack up above.
#if _DEBUG && defined(_MSC_VER)
   #undef constinit
#endif