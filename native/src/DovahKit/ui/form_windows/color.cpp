#include "color.h"
#include "_base_cpp.h"
#include "../../helpers/bitwise.h"

FormDialogColor::FormDialogColor(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogColor, dovah::loaded_forms::Color>(*this, stub);
   //
   this->ui.colorPicker->setHasAlpha(false);
   //
   this->load();
}
void FormDialogColor::_load_impl() {
   this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
   this->ui.name->setText(QString::fromStdString(this->form->name.c_str()));
   //
   QColor color{ this->form->color.r, this->form->color.g, this->form->color.b, this->form->color.unused };
   this->ui.colorPicker->setColor(color);
   //
   this->ui.playable->setChecked((this->form->color_flags & dovah::loaded_forms::Color::color_flag::playable) != 0);
}
void FormDialogColor::_save_impl() {
   this->stub->editorID = this->ui.editorID->text().toStdString();
   this->form->name = this->ui.name->text().toStdString();
   //
   auto color = this->ui.colorPicker->color();
   this->form->color.r = color.red();
   this->form->color.g = color.green();
   this->form->color.b = color.blue();
   //
   cobb::modify_bit(this->form->color_flags, dovah::loaded_forms::Color::color_flag::playable, this->ui.playable->isChecked());
}