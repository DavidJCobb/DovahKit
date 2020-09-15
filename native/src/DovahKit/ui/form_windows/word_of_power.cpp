#include "word_of_power.h"
#include "../../helpers/bitwise.h"
#include "../../editor/core.h"

FormDialogWordOfPower::FormDialogWordOfPower(dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   if (stub->formType == dovah::form_type::word_of_power)
      this->form = stub->load().ptr_cast<dovah::loaded_forms::WordOfPower>();
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, [this]() {
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, [this]() {
      this->save();
      this->accept();
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->form = nullptr;
      this->reject();
   });
   //
   this->load();
}
void FormDialogWordOfPower::load() {
   if (!this->form)
      return;
   this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
   this->ui.dragonName->setText(QString::fromStdString(this->form->dragon_name.c_str()));
   this->ui.humanName->setText(QString::fromStdString(this->form->human_name.c_str()));
}
void FormDialogWordOfPower::save() {
   if (!this->form)
      return;
   auto stub = this->form->stub;
   stub->set_edited(true);
   stub->editorID = this->ui.editorID->text().toStdString();
   this->form->dragon_name = this->ui.dragonName->text().toStdString();
   this->form->human_name  = this->ui.humanName->text().toStdString();
   //
   emit DovahKitCore::get().formModified(stub);
}