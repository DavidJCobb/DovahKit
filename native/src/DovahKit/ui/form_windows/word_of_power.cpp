#include "./word_of_power.h"
#include "editor/subsystems/game_localized_strings/core.h"

FormDialogWordOfPower::FormDialogWordOfPower(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   initialize(stub);
   
   this->load();
}
void FormDialogWordOfPower::_load_impl() {
   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   
   this->ui.editorID->setText(QString::fromStdString(this->form->stub.get_editor_id()));
   this->ui.dragonName->setText(gls.convert_localized_string(this->form->dragon_name));
   this->ui.humanName->setText(gls.convert_localized_string(this->form->human_name));
}
void FormDialogWordOfPower::_save_impl() {
   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   
   this->stub->set_edited(true);
   gls.assign_localized_string(this->form->dragon_name, this->ui.dragonName->text());
   gls.assign_localized_string(this->form->human_name,  this->ui.humanName->text());
}