#pragma once
#include "helpers/ini/file.h"
#include "helpers/ini/category.h"
#include "helpers/ini/setting.h"
#include "helpers/ini/setting_definition.h"

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
   // Remove this hacky garbage as soon as Microsoft fixes things on their end! Be 
   // sure to wipe the *.cpp file as well.
   //
   #define MSVC_CONSTINIT_STILL_BROKEN 1
#endif

#pragma push_macro("MAKE_INI_SETTING")
#pragma push_macro("MAKE_INI_STRING_SETTING")
#pragma push_macro("MAKE_INI_SETTING_WITH_CONSTRAINTS")

#define MAKE_INI_SETTING(setting_name, initial) \
   constinit cobb::ini::setting setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::decay_t<decltype(initial)>>{ \
      .name          = #setting_name, \
      .initial_value = initial, \
   }>(category_data);

#define MAKE_INI_STRING_SETTING(setting_name, initial) \
   constinit cobb::ini::setting setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::string>{ \
      .name          = #setting_name, \
      .initial_value = initial, \
   }>(category_data);

#define MAKE_INI_SETTING_WITH_CONSTRAINTS(setting_name, initial, a_constraints) \
   constinit cobb::ini::setting setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::decay_t<decltype(initial)>>{ \
      .name          = #setting_name, \
      .initial_value = initial, \
      .constraints   = a_constraints, \
   }>(category_data);

#if MSVC_CONSTINIT_STILL_BROKEN
   #define constinit extern

   #undef MAKE_INI_SETTING
   #define MAKE_INI_SETTING(setting_name, initial) \
      extern cobb::ini::setting setting_name ;

   #undef MAKE_INI_STRING_SETTING
   #define MAKE_INI_STRING_SETTING(setting_name, initial) \
      extern cobb::ini::setting setting_name ;

   #undef MAKE_INI_SETTING_WITH_CONSTRAINTS
   #define MAKE_INI_SETTING_WITH_CONSTRAINTS(setting_name, initial, a_constraints) \
      extern cobb::ini::setting setting_name ;
#endif

namespace dovahkit::ini::main {
   constinit cobb::ini::file file_data
      #if !MSVC_CONSTINIT_STILL_BROKEN
      = cobb::ini::file{}
      #endif
   ;

   namespace saving {
      constinit cobb::ini::category category_data
         #if !MSVC_CONSTINIT_STILL_BROKEN
         = cobb::ini::category(file_data, "saving")
         #endif
      ;

      MAKE_INI_SETTING(bApplyRefPersistenceAsNeeded, true);
      MAKE_INI_SETTING(bClearRefPersistenceWhenAble, false);
   }
   namespace worldedit {
      constinit cobb::ini::category category_data
         #if !MSVC_CONSTINIT_STILL_BROKEN
         = cobb::ini::category(file_data, "worldedit")
         #endif
      ;

      MAKE_INI_SETTING_WITH_CONSTRAINTS(fCameraSpeedNormal,        (double)180.0, (cobb::ini::value_constraint_info<double>{ .min = 1.0, .max = 512.0 }));
      MAKE_INI_SETTING_WITH_CONSTRAINTS(fCameraSpeedMultBoost,     (double)2.0,   (cobb::ini::value_constraint_info<double>{ .min = 0.1, .max = 20.0 }));
      MAKE_INI_SETTING_WITH_CONSTRAINTS(fCameraSpeedMultPrecision, (double)0.3,   (cobb::ini::value_constraint_info<double>{ .min = 0.1, .max = 20.0 }));
      
      MAKE_INI_SETTING_WITH_CONSTRAINTS(uLoadedGridSize, (unsigned int)5, (cobb::ini::value_constraint_info<unsigned int>{.min = 5 }));
      MAKE_INI_SETTING(bLoadedGridSizeOverrideFromSkyrimINI, true);

      MAKE_INI_SETTING(bInvertLookX, false);
      MAKE_INI_SETTING(bInvertLookY, true);

      MAKE_INI_SETTING_WITH_CONSTRAINTS(fTurnSpeedDegreesPerSecondX, (double)90.0, (cobb::ini::value_constraint_info<double>{.min = 1.0, .max = 720.0 }));
      MAKE_INI_SETTING_WITH_CONSTRAINTS(fTurnSpeedDegreesPerSecondY, (double)90.0, (cobb::ini::value_constraint_info<double>{.min = 1.0, .max = 720.0 }));
   }
   namespace worldinput {
      constinit cobb::ini::category category_data
         #if !MSVC_CONSTINIT_STILL_BROKEN
         = cobb::ini::category(file_data, "worldinput")
         #endif
      ;

      MAKE_INI_STRING_SETTING(sCurrentControlSchemeGamepad,  "");
      MAKE_INI_STRING_SETTING(sCurrentControlSchemeKeyboard, "");
   }
}

#undef MAKE_INI_SETTING_WITH_CONSTRAINTS
#undef MAKE_INI_STRING_SETTING
#undef MAKE_INI_SETTING
#pragma pop_macro("MAKE_INI_SETTING_WITH_CONSTRAINTS")
#pragma pop_macro("MAKE_INI_STRING_SETTING")
#pragma pop_macro("MAKE_INI_SETTING")

// Remove this when we remove the MSVC Debug hack up above.
#if MSVC_CONSTINIT_STILL_BROKEN
   #undef constinit
#endif