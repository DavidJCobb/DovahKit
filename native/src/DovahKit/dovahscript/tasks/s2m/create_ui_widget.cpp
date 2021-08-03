#include "create_ui_widget.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/lifetime.h"

namespace dovahscript::tasks::s2m {
   namespace impl::create_ui_widget {
      extern void set_up_widget(QWidget& widget) {
         core::subsystems::coordinator::get().set_up_widget(widget);
         core::subsystems::lifetime::get().on_hierarchy_item_created(widget);
      }
   }
}