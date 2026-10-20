#include "./form_stub_collection.h"
#include "./form_stub.h"
#include "./_impl/form_stub_collection/create_hardcoded_form_stubs.h"

namespace dovah {
   void form_stub_collection::_add_hardcoded_form(form_stub& stub) {
      auto& type = this->forms_by_type[stub.form_type];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      
      const bare_form_id formID = stub.form_id;
      type.forms[formID] = &stub;
      this->forms.forms[formID] = &stub;
   }

   //
   // passkeyed
   //

   void form_stub_collection::_make_hardcoded_forms(lu::passkey<active_load_order>, tes::file& hardcoded_forms_file) {
      impl::_form_stub_collection::create_hardcoded_form_stubs(*this, hardcoded_forms_file);
   }
   void form_stub_collection::_on_stub_flagged_as_edited(lu::passkey<form_stub>, form_stub& stub) {
      this->active_file_forms.forms[stub.form_id] = &stub;
      auto& at = this->active_file_forms_by_type[stub.form_type];
      at.forms[stub.form_id] = &stub;
      //
      // NOTE: Merely flagging a form stub as edited SHOULD NOT result in the active file being 
      // added to the stub's source file list (if not already present). Currently, our handling 
      // of singleton forms depends on no such addition being made (refer to documentation for 
      // more information).
      //
   }
}