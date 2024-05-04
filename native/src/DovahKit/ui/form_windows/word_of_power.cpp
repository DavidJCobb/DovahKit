#include "./word_of_power.h"
#include "helpers/bitwise.h"

FormDialogWordOfPower::FormDialogWordOfPower(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   initialize(stub);
   
   this->load();
}
void FormDialogWordOfPower::_load_impl() {
   auto& editor = DovahKitCore::get();
   
   this->ui.editorID->setText(QString::fromStdString(this->form->stub.get_editor_id()));
   this->ui.dragonName->setText(editor.convert_localized_string(this->form->dragon_name));
   this->ui.humanName->setText(editor.convert_localized_string(this->form->human_name));
}
void FormDialogWordOfPower::_save_impl() {
   auto& editor = DovahKitCore::get();
   
   this->stub->set_edited(true);
   editor.assign_localized_string(this->form->dragon_name, this->ui.dragonName->text());
   editor.assign_localized_string(this->form->human_name,  this->ui.humanName->text());
}