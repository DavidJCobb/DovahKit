#include "./dialogue_category.h"
#include <QCoreApplication>

#define ENUMERATION_TYPE dovah::dialogue::category
#include "./_macros.h"

namespace editor::localize {
   extern QString dialogue_category(ENUMERATION_TYPE v) {
      using enum ENUMERATION_TYPE;
      switch (v) {
         case topic:
            return STRING("Player");
         case favor_dialogue:
            return STRING("Favor");
         case combat:
            return STRING("Combat");
         case detection:
            return STRING("Detection");
         case service:
            return STRING("Service");
         case miscellaneous:
            return STRING("Misc");
         case favors:
            return STRING("Favor (Deprecated)");
      }
      return "";
   }
}