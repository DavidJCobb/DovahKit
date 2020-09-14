#include "color.h"
#include "../../helpers/bitwise.h"
#include "../../editor/core.h"

FormDialogColor::FormDialogColor(dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   if (stub->formType == dovah::form_type::color)
      this->form = stub->load().ptr_cast<dovah::loaded_forms::Color>();
   //
   this->ui.colorPicker->setHasAlpha(false);
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, [this]() {
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, [this]() {
      this->save();
      this->accept();
   });
   //
   this->load();
}
void FormDialogColor::load() {
   if (!this->form)
      return;
   this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
   this->ui.name->setText(QString::fromStdString(this->form->name.c_str()));
   //
   QColor color{ this->form->color.r, this->form->color.g, this->form->color.b, this->form->color.unused };
   this->ui.colorPicker->setColor(color);
   //
   this->ui.playable->setChecked((this->form->color_flags & dovah::loaded_forms::Color::color_flag::playable) != 0);
}
void FormDialogColor::save() {
   if (!this->form)
      return;
   auto stub = this->form->stub;
   stub->set_edited(true);
   stub->editorID = this->ui.editorID->text().toStdString();
   this->form->name = this->ui.name->text().toStdString();
   //
   auto color = this->ui.colorPicker->color();
   this->form->color.r = color.red();
   this->form->color.g = color.green();
   this->form->color.b = color.blue();
   //
   cobb::modify_bit(this->form->color_flags, dovah::loaded_forms::Color::color_flag::playable, this->ui.playable->isChecked());
   //
   emit DovahKitCore::get().formModified(stub);
}