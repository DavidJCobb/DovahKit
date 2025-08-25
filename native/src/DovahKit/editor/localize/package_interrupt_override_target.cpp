#include "./package_interrupt_override_target.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_interrupt_override_target(dovah::packages::interrupt_override_target v) {
      switch (v) {
         case dovah::packages::interrupt_override_target::combat_target:
            return QCoreApplication::translate("dovah::packages::interrupt_override_target", "Combat Target");
         case dovah::packages::interrupt_override_target::corpse_to_observe:
            return QCoreApplication::translate("dovah::packages::interrupt_override_target", "Corpse to Observe");
         case dovah::packages::interrupt_override_target::ref_to_guard:
            return QCoreApplication::translate("dovah::packages::interrupt_override_target", "Ref to Guard");
         case dovah::packages::interrupt_override_target::threat_to_spectate:
            return QCoreApplication::translate("dovah::packages::interrupt_override_target", "Threat to Spectate");
         case dovah::packages::interrupt_override_target::trespasser:
            return QCoreApplication::translate("dovah::packages::interrupt_override_target", "Trespasser");
      }
      return "";
   }
}