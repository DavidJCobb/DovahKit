#include "./idle_node.h"
#include "dovah/form_stub.h"

namespace IdleAnimationFormsModel_impl {
   /*virtual*/ void idle_node::update_cached_form_data() /*override*/ {
      if (this->stub == nullptr) {
         this->cached.editor_id = QString("NONE");
         return;
      }
      this->cached.editor_id = QString::fromStdString(this->stub->get_editor_id());
   }
}