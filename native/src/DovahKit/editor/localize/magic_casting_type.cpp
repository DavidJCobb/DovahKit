#include "./magic_casting_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString magic_casting_type(dovah::magic_casting_type v) {
      switch (v) {
         case dovah::magic_casting_type::concentration:
            return QCoreApplication::translate("dovah::magic_casting_type", "Concentration");
         case dovah::magic_casting_type::constant_effect:
            return QCoreApplication::translate("dovah::magic_casting_type", "Constant Effect");
         case dovah::magic_casting_type::fire_and_forget:
            return QCoreApplication::translate("dovah::magic_casting_type", "Fire and Forget");
      }
      return "";
   }
}