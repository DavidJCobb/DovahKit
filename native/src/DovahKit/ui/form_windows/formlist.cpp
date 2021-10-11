#include "formlist.h"
#include "_base_cpp.h"

FormDialogFormList::FormDialogFormList(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogFormList, dovah::loaded_forms::FormList>(*this, stub);
   //
   this->load();
}
void FormDialogFormList::_load_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.forms->pullStubs(this->form->contents);
}
void FormDialogFormList::_save_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   this->ui.forms->commitStubs(this->form->contents, *this->form);
}