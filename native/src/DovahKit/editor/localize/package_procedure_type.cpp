#include "./package_procedure_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_procedure_type(dovah::packages::procedure_type v) {
      switch (v) {
         case dovah::packages::procedure_type::acquire:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Acquire");
         case dovah::packages::procedure_type::activate:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Activate");
         case dovah::packages::procedure_type::dialogue_activate:
            return QCoreApplication::translate("dovah::packages::procedure_type", "DialogueActivate");
         case dovah::packages::procedure_type::dialogue:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Dialogue");
         case dovah::packages::procedure_type::done:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Done");
         case dovah::packages::procedure_type::eat:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Eat");
         case dovah::packages::procedure_type::escort:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Escort");
         case dovah::packages::procedure_type::find:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Find");
         case dovah::packages::procedure_type::flee:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Flee");
         case dovah::packages::procedure_type::flight_grab:
            return QCoreApplication::translate("dovah::packages::procedure_type", "FlightGrab");
         case dovah::packages::procedure_type::follow:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Follow");
         case dovah::packages::procedure_type::follow_to:
            return QCoreApplication::translate("dovah::packages::procedure_type", "FollowTo");
         case dovah::packages::procedure_type::force_greet:
            return QCoreApplication::translate("dovah::packages::procedure_type", "ForceGreet");
         case dovah::packages::procedure_type::guard:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Guard");
         case dovah::packages::procedure_type::hold_position:
            return QCoreApplication::translate("dovah::packages::procedure_type", "HoldPosition");
         case dovah::packages::procedure_type::hover:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Hover");
         case dovah::packages::procedure_type::keep_an_eye_on:
            return QCoreApplication::translate("dovah::packages::procedure_type", "KeepAnEyeOn");
         case dovah::packages::procedure_type::lock_doors:
            return QCoreApplication::translate("dovah::packages::procedure_type", "LockDoors");
         case dovah::packages::procedure_type::orbit:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Orbit");
         case dovah::packages::procedure_type::patrol:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Patrol");
         case dovah::packages::procedure_type::pursue:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Pursue");
         case dovah::packages::procedure_type::sandbox:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Sandbox");
         case dovah::packages::procedure_type::say:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Say");
         case dovah::packages::procedure_type::shout:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Shout");
         case dovah::packages::procedure_type::sit:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Sit");
         case dovah::packages::procedure_type::sleep:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Sleep");
         case dovah::packages::procedure_type::travel:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Travel");
         case dovah::packages::procedure_type::unlock_doors:
            return QCoreApplication::translate("dovah::packages::procedure_type", "UnlockDoors");
         case dovah::packages::procedure_type::use_idle_marker:
            return QCoreApplication::translate("dovah::packages::procedure_type", "UseIdleMarker");
         case dovah::packages::procedure_type::use_magic:
            return QCoreApplication::translate("dovah::packages::procedure_type", "UseMagic");
         case dovah::packages::procedure_type::use_weapon:
            return QCoreApplication::translate("dovah::packages::procedure_type", "UseWeapon");
         case dovah::packages::procedure_type::wait:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Wait");
         case dovah::packages::procedure_type::wander:
            return QCoreApplication::translate("dovah::packages::procedure_type", "Wander");
      }
      return QCoreApplication::translate("dovah::packages::procedure_type", "Unknown #%1").arg((size_t)v);
   }
}