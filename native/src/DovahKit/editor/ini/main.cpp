#include "./main.h"

#pragma push_macro("MAKE_INI_SETTING")
#pragma push_macro("MAKE_INI_STRING_SETTING")
#pragma push_macro("MAKE_INI_SETTING_WITH_CONSTRAINTS")

#define MAKE_INI_SETTING(setting_name, initial) \
   cobb::ini::setting setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::decay_t<decltype(initial)>>{ \
      .name          = #setting_name, \
      .initial_value = initial, \
   }>(category_data);

#define MAKE_INI_STRING_SETTING(setting_name, initial) \
   cobb::ini::setting setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::string>{ \
      .name          = #setting_name, \
      .initial_value = initial, \
   }>(category_data);

#define MAKE_INI_SETTING_WITH_CONSTRAINTS(setting_name, initial, a_constraints) \
   cobb::ini::setting setting_name = cobb::ini::setting::define<cobb::ini::setting_definition<std::decay_t<decltype(initial)>>{ \
      .name          = #setting_name, \
      .initial_value = initial, \
      .constraints   = a_constraints, \
   }>(category_data);

namespace dovahkit::ini::main {
   cobb::ini::file file_data = cobb::ini::file{};

   namespace saving {
      cobb::ini::category category_data = cobb::ini::category(file_data, "saving");

      MAKE_INI_SETTING(bApplyRefPersistenceAsNeeded, true);
      MAKE_INI_SETTING(bClearRefPersistenceWhenAble, false);
   }
   namespace worldedit {
      cobb::ini::category category_data = cobb::ini::category(file_data, "worldedit");

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
      cobb::ini::category category_data = cobb::ini::category(file_data, "worldinput");

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