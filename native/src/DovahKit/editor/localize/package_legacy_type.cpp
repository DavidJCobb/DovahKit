#include "./package_legacy_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_legacy_type(dovah::packages::legacy_type v) {
      switch (v) {
         case dovah::packages::legacy_type::accompany:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Accompany");
         case dovah::packages::legacy_type::ambush:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Ambush");
         case dovah::packages::legacy_type::custom:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Package");
         case dovah::packages::legacy_type::custom_template:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Package Template");
         case dovah::packages::legacy_type::dialogue:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Dialogue");
         case dovah::packages::legacy_type::eat:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Eat");
         case dovah::packages::legacy_type::escort:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Escort");
         case dovah::packages::legacy_type::find:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Find");
         case dovah::packages::legacy_type::find_deprecated:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Deprecated Find");
         case dovah::packages::legacy_type::flee_non_combat:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Flee (Non-Combat)");
         case dovah::packages::legacy_type::follow:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Follow");
         case dovah::packages::legacy_type::guard:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Guard");
         case dovah::packages::legacy_type::patrol:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Patrol");
         case dovah::packages::legacy_type::sandbox:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Sandbox");
         case dovah::packages::legacy_type::sleep:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Sleep");
         case dovah::packages::legacy_type::travel:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Travel");
         case dovah::packages::legacy_type::use_item_at:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Use Item");
         case dovah::packages::legacy_type::use_magic:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Use Magic");
         case dovah::packages::legacy_type::use_weapon:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Use Weapon");
         case dovah::packages::legacy_type::wander:
            return QCoreApplication::translate("dovah::packages::legacy_type", "Legacy Wander");
      }
      return "";
   }
}