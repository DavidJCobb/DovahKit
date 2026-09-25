#pragma once
#include "helpers/ini/file.h"
#include "helpers/ini/category.h"
#include "helpers/ini/setting.h"
#include "helpers/ini/setting_definition.h"

//
// TODO: If it ever becomes possible to make these data structures `constinit`, then 
//       do so. That was my original goal, and it worked in Release at one point (not 
//       now, apparently), but never in Debug, due to:
// 
//       https://developercommunity.visualstudio.com/t/MDd-makes-it-impossible-to-have-constin/10439085
// 
//       The explanation given there does note that empty constinit STL containers are 
//       possible but only based on implementation i.e. not mandated by the standard. 
//       Perhaps that'll change someday?
//

#pragma push_macro("MAKE_INI_SETTING")
#pragma push_macro("MAKE_INI_STRING_SETTING")
#pragma push_macro("MAKE_INI_SETTING_WITH_CONSTRAINTS")

#undef MAKE_INI_SETTING
#define MAKE_INI_SETTING(setting_name, initial) \
   extern cobb::ini::setting setting_name ;

#undef MAKE_INI_STRING_SETTING
#define MAKE_INI_STRING_SETTING(setting_name, initial) \
   extern cobb::ini::setting setting_name ;

#undef MAKE_INI_SETTING_WITH_CONSTRAINTS
#define MAKE_INI_SETTING_WITH_CONSTRAINTS(setting_name, initial, a_constraints) \
   extern cobb::ini::setting setting_name ;

namespace dovahkit::ini::main {
   extern cobb::ini::file file_data;
   
   namespace debug {
      extern cobb::ini::category category_data;

      MAKE_INI_SETTING(bShowMegaTests, false);
   }
   namespace saving {
      extern cobb::ini::category category_data;

      MAKE_INI_SETTING(bApplyRefPersistenceAsNeeded, true);
      MAKE_INI_SETTING(bClearRefPersistenceWhenAble, false);
   }
   namespace skyrim {
      extern cobb::ini::category category_data;

      MAKE_INI_STRING_SETTING(sOverridePathClassic, "");
      MAKE_INI_STRING_SETTING(sOverridePathSpecial, "");
   }
   namespace worldedit {
      extern cobb::ini::category category_data;

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
      extern cobb::ini::category category_data;

      MAKE_INI_SETTING(fPressToLongPressThreshold, 0.35);
      MAKE_INI_SETTING(fPressToHoldThreshold,      0.50);
      MAKE_INI_SETTING(fKeySequenceExpireTime,     0.25);

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