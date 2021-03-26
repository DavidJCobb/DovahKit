#include "ui_dialog_operation.h"
#include "../../editor_script_core.h"
#include "../../../core.h"

namespace editor_script::tasks::s2m {
   /*virtual*/ void ui_dialog_operation::_exec_impl() /*override*/ {
      if (!this->target)
         return;
      switch (this->operation) {
         case operation_type::none:
            break;
            //
         case operation_type::get_size_grip:
            this->results.boolean = this->target->isSizeGripEnabled();
            break;
         case operation_type::set_size_grip:
            this->target->setSizeGripEnabled(this->params.boolean);
            break;
            //
         case operation_type::get_title:
            this->results.text = this->target->windowTitle();
            break;
         case operation_type::set_title:
            this->target->setWindowTitle(this->params.text);
            break;
            //
         case operation_type::get_visibility:
            this->results.boolean = this->target->isVisible();
            break;
         case operation_type::set_visibility:
            if (this->params.boolean)
               this->target->open();
            else
               this->target->done(-1);
            break;
      }
   }
}