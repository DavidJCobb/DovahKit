#include "widget_utils.h"

namespace cobb::qt {
   extern bool is_explicitly_shown(const QWidget* widget) {
      if (!widget->testAttribute(Qt::WidgetAttribute::WA_WState_ExplicitShowHide)) // undocumented flag; internal?
         return false;
      return widget->isVisible();
   }
   extern bool is_explicitly_hidden(const QWidget* widget) {
      if (!widget->testAttribute(Qt::WidgetAttribute::WA_WState_ExplicitShowHide)) // undocumented flag; internal?
         return false;
      return widget->isHidden();
   }
}
