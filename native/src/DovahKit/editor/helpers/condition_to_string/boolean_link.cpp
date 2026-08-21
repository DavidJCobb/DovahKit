#include "./boolean_link.h"
#include <QCoreApplication>

namespace editor_helpers::condition_to_string {
   extern QString boolean_link(bool is_or) {
      if (is_or)
         return QCoreApplication::translate("condition boolean link", "OR");
      return QCoreApplication::translate("condition boolean link", "AND");
   }
}
