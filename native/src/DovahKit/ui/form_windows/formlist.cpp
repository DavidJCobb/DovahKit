#include "formlist.h"
#include "./_base_cpp.h"
#include "editor/subsystems/gui_adjust/core.h"

FormDialogFormList::FormDialogFormList(dovah::form_stub* stub, QWidget* parent) : FormEditDialogBase(stub, parent) {
   form_dialog_helpers::initialize(*this, stub);
   dovahkit::subsystems::gui_adjust::core::get_or_create().registerWidgets(*this, {
      { "LstForms", this->ui.forms },
   });
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