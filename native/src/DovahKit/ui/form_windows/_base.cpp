#include "_base.h"
#include "../../editor/core.h"
#include "../../dovah/forms/components/extra_data/_templates.h"

FormDialogBaseTemplate::FormDialogBaseTemplate(dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {}
void FormDialogBaseTemplate::load() {
   if (!this->stub)
      return;
   this->_load_impl();
}
void FormDialogBaseTemplate::save() {
   if (!this->stub)
      return;
   this->stub->set_edited(true);
   this->_save_impl();
   emit DovahKitCore::get().formModified(this->stub);
}

void FormDialogBaseTemplate::save_form_id(dovah::form_id_t& target, dovah::bare_form_id_t value) {
   target.set(this->stub, value);
}
void FormDialogBaseTemplate::save_form_id(dovah::form_id_t& target, dovah::form_stub* value) {
   target.set(this->stub, value);
}

void FormDialogBaseTemplate::save_extra_form(dovah::bare_form_id_t formID, extra_data_list& extra, extra_data_type et, bool remove_if_no_form) {
   using dummy_t = dovah::loaded_forms::components::formID_extra_data<0, extra_data_type::action>;
   //
   if (formID || !remove_if_no_form) {
      auto* data = extra.get_or_create_by_type(et);
      if (data)
         this->save_form_id(((dummy_t*)data)->formID, formID);
   } else {
      extra.remove_by_type(et);
   }
}