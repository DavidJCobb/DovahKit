#include "form.h"
#include "../form_stub.h"
#include "../files/tes_file_reading/elements.h"
#include "../files/tes_file_writing/elements.h"

namespace dovah::loaded_forms {
   const char* Form::get_editor_id() const noexcept {
      return this->stub ? this->stub->get_editor_id() : nullptr;
   }
   void Form::load(tes_record_reader& record) {
      this->flags = record.flags();
   }
   bool Form::save(tes_record_writer& record) {
      auto editor_id = this->get_editor_id();
      if (editor_id && editor_id[0])
         record.write_string_subrecord('EDID', editor_id);
      //
      return this->_save_impl(record);
   }
}