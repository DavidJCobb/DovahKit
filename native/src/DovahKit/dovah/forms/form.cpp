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
   Form* Form::clone(form_stub& receiving_stub, bool* out_complete) const noexcept {
      assert(receiving_stub.form == nullptr && "Cannot clone a loaded form into a stub that already has a loaded form.");
      auto instance = create_blank_loaded_form_by_type(this->formType);
      if (instance) {
         receiving_stub.form = instance;
         instance->stub  = &receiving_stub;
         instance->flags = this->flags;
         receiving_stub.set_edited(true);
         bool result = this->_clone_impl(instance);
         receiving_stub.set_edited(true);
         if (out_complete)
            *out_complete = result;
      }
      return instance;
   }
   bool Form::save(tes_record_writer& record) {
      if (this->flags & form_flag::deleted) {
         //
         // Specific form types don't appear to save ANY data -- not even editor IDs -- if they 
         // are flagged as deleted.
         //
         auto& info = form_type_info::lookup(this->formType);
         if (info.flags & form_type_info::flag::empty_if_deleted)
            return true;
      }
      //
      auto editor_id = this->get_editor_id();
      if (editor_id && editor_id[0])
         record.write_string_subrecord('EDID', editor_id);
      //
      return this->_save_impl(record);
   }
   void Form::friendly_delete_override() noexcept {
      bool flag = !this->_friendly_delete_impl();
      cobb::edit_bit(this->flags, form_flag::deleted, flag);
   }
   void Form::flag_as_deleted() noexcept {
      cobb::edit_bit(this->flags, form_flag::deleted, true);
   }
   void Form::sever_outbound_references_to(form_stub& other) noexcept {
      this->_sever_outbound_references_impl(other);
   }
}