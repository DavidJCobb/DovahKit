#include "./package_interrupt_override_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_interrupt_override_type(dovah::packages::interrupt_override_type v) {
      switch (v) {
         case dovah::packages::interrupt_override_type::none:
            return QCoreApplication::translate("dovah::packages::interrupt_override_type", "None");
         case dovah::packages::interrupt_override_type::combat:
            return QCoreApplication::translate("dovah::packages::interrupt_override_type", "Combat");
         case dovah::packages::interrupt_override_type::guard_warn:
            return QCoreApplication::translate("dovah::packages::interrupt_override_type", "Guard Warn");
         case dovah::packages::interrupt_override_type::observe_dead:
            return QCoreApplication::translate("dovah::packages::interrupt_override_type", "Observe Dead");
         case dovah::packages::interrupt_override_type::spectator:
            return QCoreApplication::translate("dovah::packages::interrupt_override_type", "Spectate Combat");
      }
      return "";
   }
}