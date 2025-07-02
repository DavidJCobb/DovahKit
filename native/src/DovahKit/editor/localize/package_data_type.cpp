#include "./package_data_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_data_type(dovah::packages::package_data_type v) {
      switch (v) {
         case dovah::packages::package_data_type::boolean:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Bool");
         case dovah::packages::package_data_type::float32:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Float");
         case dovah::packages::package_data_type::integer:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Int");
         case dovah::packages::package_data_type::location:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Location");
         case dovah::packages::package_data_type::object_list:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Object List");
         case dovah::packages::package_data_type::single_ref:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Ref");
         case dovah::packages::package_data_type::target_selector:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Target Selector");
         case dovah::packages::package_data_type::topic:
            return QCoreApplication::translate("dovah::packages::package_data_type", "Topic");
      }
      return "";
   }
}