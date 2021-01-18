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
   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(this->stub);
   this->stub->set_edited(true);
   this->_save_impl();
   emit editor.formModified(this->stub);
}

void FormDialogBaseTemplate::save_form_id(dovah::form_reference_t& target, dovah::bare_form_id_t id) {
   auto& editor = DovahKitCore::get();
   auto* value  = editor.get_form(id);
   this->save_form_id(target, value);
}
void FormDialogBaseTemplate::save_form_id(dovah::form_reference_t& target, dovah::form_stub* value) {
   assert(this->stub->form && "");
   target.set(*this->stub->form, value);
}

void FormDialogBaseTemplate::save_extra_form(dovah::bare_form_id_t formID, extra_data_list& extra, extra_data_type et, bool remove_if_no_form) {
   using dummy_t = dovah::loaded_forms::components::formID_extra_data<0, extra_data_type::action>;
   //
   if (formID || !remove_if_no_form) {
      auto* data = extra.get_or_create_by_type(et);
      if (data)
         this->save_form_id(((dummy_t*)data)->form, formID);
   } else {
      extra.remove_by_type(et);
   }
}
void FormDialogBaseTemplate::save_extra_form(dovah::form_stub* stub, extra_data_list& extra, extra_data_type et, bool remove_if_no_form) {
   using dummy_t = dovah::loaded_forms::components::formID_extra_data<0, extra_data_type::action>;
   //
   if (stub || !remove_if_no_form) {
      auto* data = extra.get_or_create_by_type(et);
      if (data)
         this->save_form_id(((dummy_t*)data)->form, stub);
   } else {
      extra.remove_by_type(et);
   }
}