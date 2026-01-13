#include "./color.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "helpers/bitwise.h"

FormDialogColor::FormDialogColor(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   initialize(stub);
   
   this->ui.colorPicker->setHasAlpha(false);
   
   this->load();
}
void FormDialogColor::_load_impl() {
   auto& editor = DovahKitCore::get();
   auto& gls    = dovahkit::subsystems::game_localized_strings::core::get();
   
   this->ui.editorID->setText(QString::fromStdString(this->form->stub.get_editor_id()));
   this->ui.name->setText(gls.convert_localized_string(this->form->name));
   
   QColor color{ this->form->color.r, this->form->color.g, this->form->color.b, this->form->color.unused };
   this->ui.colorPicker->setColor(color);
   
   this->ui.playable->setChecked((this->form->color_flags & dovah::loaded_forms::Color::color_flag::playable) != 0);
}
void FormDialogColor::_save_impl() {
   auto& editor = DovahKitCore::get();
   auto& gls    = dovahkit::subsystems::game_localized_strings::core::get();
   
   this->stub->editorID = this->ui.editorID->text().toStdString();
   gls.assign_localized_string(this->form->name, this->ui.name->text());
   
   auto color = this->ui.colorPicker->color();
   this->form->color.r = color.red();
   this->form->color.g = color.green();
   this->form->color.b = color.blue();
   
   cobb::modify_bit(this->form->color_flags, dovah::loaded_forms::Color::color_flag::playable, this->ui.playable->isChecked());
}