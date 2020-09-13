#include "form.h"
#include "../form_stub.h"

namespace dovah::loaded_forms {
   const char* Form::get_editor_id() const noexcept {
      return this->stub ? this->stub->get_editor_id() : nullptr;
   }
}