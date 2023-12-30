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
   Form::Form(form_type_t ft, const constructor_params& c) : formType(ft), is_working_copy(c.is_working_copy), stub(*c.stub) {
      assert(c.stub && "Form::constructor_params::stub must not be nullptr at the time construction occurs!");
   }

   dovah::papyrus::scriptobject_list Form::resolve_papyrus_scripts() const {
      loaded_form_ptr<Form> base_form;
      if (dovah::form_type_info::form_type_is_reference(this->stub.formType)) {
         auto* base_stub = form_stub_helpers::get_base_form(&this->stub);
         if (base_stub)
            base_form = base_stub->load();
      }

      auto* this_papyrus = this->get_raw_papyrus_data();
      if (!this_papyrus) {
         return {};
      }

      if (base_form) {
         if (auto* base_papyrus = base_form->get_raw_papyrus_data()) {
            return dovah::papyrus::scriptobject_list({}, *this_papyrus, *base_papyrus);
         }
      }
      return dovah::papyrus::scriptobject_list({}, *this_papyrus);
   }
   void Form::overwrite_papyrus_scripts(const dovah::papyrus::scriptobject_list& src) {
      auto* this_papyrus = this->get_raw_papyrus_data();
      assert(this_papyrus != nullptr);

      loaded_form_ptr<Form> base_form;
      const components::papyrus_attachment_data* base_papyrus = nullptr;
      //
      if (dovah::form_type_info::form_type_is_reference(this->stub.formType)) {
         auto* base_stub = form_stub_helpers::get_base_form(&this->stub);
         if (base_stub) {
            base_form = base_stub->load();
            if (base_form)
               base_papyrus = base_form->get_raw_papyrus_data();
         }
      }

      {  // Preserve fragment data; wipe everything else.
         auto* fragment_data = this_papyrus->fragment_data;
         this_papyrus->fragment_data = nullptr;
         this_papyrus->clear(*this);
         this_papyrus->fragment_data = fragment_data;
      }

      if (base_papyrus) {
         src._overwrite({}, *this, *this_papyrus, *base_papyrus);
      } else {
         src._overwrite({}, *this, *this_papyrus);
      }
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
      auto instance = create_blank_loaded_form_by_type(this->formType, fcp);
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