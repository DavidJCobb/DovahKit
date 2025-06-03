#include "./magic_delivery_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString magic_delivery_type(dovah::magic_delivery_type v) {
      switch (v) {
         case dovah::magic_delivery_type::aimed:
            return QCoreApplication::translate("dovah::magic_delivery_type", "Aimed");
         case dovah::magic_delivery_type::self:
            return QCoreApplication::translate("dovah::magic_delivery_type", "Self");
         case dovah::magic_delivery_type::target_actor:
            return QCoreApplication::translate("dovah::magic_delivery_type", "Target Actor");
         case dovah::magic_delivery_type::target_location:
            return QCoreApplication::translate("dovah::magic_delivery_type", "Target Location");
         case dovah::magic_delivery_type::touch:
            return QCoreApplication::translate("dovah::magic_delivery_type", "Touch");
      }
      return "";
   }
}