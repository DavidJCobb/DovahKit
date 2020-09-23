#include "word_of_power.h"
#include "_base_cpp.h"
#include "../../helpers/bitwise.h"

FormDialogWordOfPower::FormDialogWordOfPower(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogWordOfPower, dovah::loaded_forms::WordOfPower>(*this, stub);
   //
   this->load();
}
void FormDialogWordOfPower::_load_impl() {
   this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
   this->ui.dragonName->setText(QString::fromStdString(this->form->dragon_name.c_str()));
   this->ui.humanName->setText(QString::fromStdString(this->form->human_name.c_str()));
}
void FormDialogWordOfPower::_save_impl() {
   this->stub->set_edited(true);
   this->form->dragon_name = this->ui.dragonName->text().toStdString();
   this->form->human_name  = this->ui.humanName->text().toStdString();
}