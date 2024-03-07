#include "form.h"
#include "../form_stub.h"
#include "../form_stub_addenda.h"
#include "../files/tes_file_reading/elements.h"
#include "../files/tes_file_writing/elements.h"
#include "factories/construct.h"
#include "_component_access.h"

#include "../form_stub_helpers.h"
#include "./components/papyrus/attachment_data.h"
#include "./components/papyrus/attached_script.h"
#include "./components/papyrus/property.h"

namespace dovah::loaded_forms {
   Form::Form(enum form_type ft, const constructor_params& c) : type(ft), is_working_copy(c.is_working_copy), stub(*c.stub) {
      assert(c.stub && "Form::constructor_params::stub must not be nullptr at the time construction occurs!");
   }

   components::model* Form::get_model() noexcept {
      return component_access::get_model(this);
   }
   components::object_bounds* Form::get_object_bounds() noexcept {
      return component_access::get_object_bounds(this);
   }
   const components::papyrus_attachment_data* Form::get_raw_papyrus_data() const noexcept {
      return component_access::get_papyrus_data(const_cast<Form*>(this));
   }
   components::papyrus_attachment_data* Form::get_raw_papyrus_data() noexcept {
      return component_access::get_papyrus_data(this);
   }

   const char* Form::get_editor_id() const noexcept {
      return this->stub.get_editor_id();
   }
   void Form::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
   }
   Form* Form::clone(form_stub& receiving_stub, bool* out_complete) const noexcept {
      assert(receiving_stub.form == nullptr && "Cannot clone a loaded form into a stub that already has a loaded form.");
      //
      constructor_params fcp;
      fcp.stub = &receiving_stub;
      //
      auto instance = create_blank_loaded_form_by_type(this->form_type, fcp);
      if (instance) {
         receiving_stub.form = instance;
         receiving_stub.set_edited(true);
         if (this->stub.addenda)
            receiving_stub.get_or_create_addenda().clone_from(*this->stub.addenda);
         bool result = this->_clone_impl(instance);
         if (out_complete)
            *out_complete = result;
      }
      return instance;
   }
   void Form::clear() {
      this->_clear_impl();
   }
   bool Form::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      assert(!this->is_working_copy && "Do not call Form::save on a working copy of a loaded form!");
      if (this->stub.is_deleted()) {
         //
         // Specific form types don't appear to save ANY data -- not even editor IDs -- if they 
         // are flagged as deleted.
         //
         auto& info = form_type_info::lookup(this->form_type);
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
      this->stub.edit_record_flags(form_flag::deleted, flag);
   }
   void Form::flag_as_deleted() noexcept {
      this->stub.edit_record_flags(form_flag::deleted, true);
   }
   void Form::sever_outbound_references_to(form_stub& other) noexcept {
      if (!this->is_working_copy) {
         if (auto* working = this->stub.working_copy)
            working->sever_outbound_references_to(other);
      }
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