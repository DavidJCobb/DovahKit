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