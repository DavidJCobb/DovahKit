#include "create_button_group.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/lifetime.h"

namespace dovahscript::tasks::s2m {
   void create_button_group::_exec_impl() {
      this->created = new QButtonGroup;
      core::subsystems::lifetime::get().on_hierarchy_item_created(*this->created);
   }
}