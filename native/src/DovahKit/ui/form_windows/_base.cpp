#include "_base.h"
#include "../../editor/core.h"

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