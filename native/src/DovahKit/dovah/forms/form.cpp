#include "form.h"
#include "../form_stub.h"
#include "../files/tes_file_reading/elements.h"
#include "../files/tes_file_writing/elements.h"
#include "factories/construct.h"

namespace dovah::loaded_forms {
   const char* Form::get_editor_id() const noexcept {
      return this->stub ? this->stub->get_editor_id() : nullptr;
   }
   void Form::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
   }
   Form* Form::clone(form_stub& receiving_stub, bool* out_complete) const noexcept {
      assert(receiving_stub.form == nullptr && "Cannot clone a loaded form into a stub that already has a loaded form.");
      auto instance = create_blank_loaded_form_by_type(this->formType);
      if (instance) {
         receiving_stub.form = instance;
         instance->stub = &receiving_stub;
         receiving_stub.set_edited(true);
         bool result = this->_clone_impl(instance);
         receiving_stub.set_edited(true);
         if (out_complete)
            *out_complete = result;
      }
      return instance;
   }
   void Form::clear() {
      this->_clear_impl();
   }
   bool Form::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (this->stub->is_deleted()) {
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
      return this->_save_impl(record, intfc);
   }
   void Form::friendly_delete_override(const file_load_order& load_order) noexcept {
      bool flag = !this->_friendly_delete_impl(load_order);
      this->stub->edit_record_flags(form_flag::deleted, flag);
   }
   void Form::flag_as_deleted() noexcept {
      this->stub->edit_record_flags(form_flag::deleted, true);
   }
   void Form::sever_outbound_references_to(form_stub& other) noexcept {
      this->_sever_outbound_references_impl(other);
   }

   /*static*/ bool Form::subrecord_is_handled_elsewhere(uint32_t signature) {
      switch (signature) {
         case 'EDID':
            return true;
      }
      return false;
   }
}