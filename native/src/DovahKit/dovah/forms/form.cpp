#include "form.h"
#include "../form_stub.h"
#include "../files/tes_file_reading/elements.h"
#include "../files/tes_file_writing/elements.h"
#include "factories/construct.h"

namespace dovah::loaded_forms {
   const char* Form::get_editor_id() const noexcept {
      return this->stub ? this->stub->get_editor_id() : nullptr;
   }
   void Form::load(tes_record_reader& record) {
      this->flags = record.flags();
   }
   Form* Form::clone(form_stub& receiving_stub) const noexcept {
      assert(receiving_stub.form == nullptr && "Cannot clone a loaded form into a stub that already has a loaded form.");
      auto instance = create_blank_loaded_form_by_type(this->formType);
      if (instance) {
         receiving_stub.form = instance;
         instance->stub  = &receiving_stub;
         instance->flags = this->flags;
         if (!this->_clone_impl(instance)) {
            receiving_stub.form = nullptr;
            delete instance;
            instance = nullptr;
         } else {
            receiving_stub.set_edited(true);
         }
      }
      return instance;
   }
   bool Form::save(tes_record_writer& record) {
      auto editor_id = this->get_editor_id();
      if (editor_id && editor_id[0])
         record.write_string_subrecord('EDID', editor_id);
      //
      return this->_save_impl(record);
   }
}