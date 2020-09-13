#include "form.h"
#include "../form_stub.h"
#include "../files/tes_file_reading/elements.h"

namespace dovah::loaded_forms {
   const char* Form::get_editor_id() const noexcept {
      return this->stub ? this->stub->get_editor_id() : nullptr;
   }
   void Form::load(tes_record_reader& record) {
      this->flags = record.flags();
   }
}