#include "./form_stub_collection.h"
#include "./form_stub.h"

namespace dovah {
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