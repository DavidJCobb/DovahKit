#include "Form.h"
#include "../../formstub.h"

namespace LoadedForms {
   const char* Form::get_editor_id() const noexcept {
      return this->stub ? this->stub->get_editor_id() : nullptr;
   }
}