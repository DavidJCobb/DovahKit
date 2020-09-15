#include "shout.h"
#include "../../editor/core.h"

FormDialogShout::FormDialogShout(dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   this->ui.word0->linkToForm(0, stub);
   this->ui.word1->linkToForm(1, stub);
   this->ui.word2->linkToForm(2, stub);
   //
   if (stub->formType == dovah::form_type::shout)
      this->form = stub->load().ptr_cast<dovah::loaded_forms::Shout>();
   //
   this->ui.menuDisplayObject->addFormType(dovah::form_type::statik);
   this->ui.menuDisplayObject->setAllowNone(true);
   this->ui.menuDisplayObject->populate();
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
void FormDialogShout::load() {
   if (!this->form)
      return;
   this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
   this->ui.name->setText(this->form->name.c_str());
   this->ui.treatAsPower->setChecked(this->form->treat_as_power());
   this->ui.menuDisplayObject->setFormByID(this->form->menuDisplayObjectID);
   this->ui.description->setPlainText(this->form->description.c_str());
   this->ui.word0->load();
   this->ui.word1->load();
   this->ui.word2->load();
}
void FormDialogShout::save() {
   if (!this->form)
      return;
   auto stub = this->form->stub;
   stub->set_edited(true);
   stub->editorID = this->ui.editorID->text().toStdString();
   this->form->name = this->ui.name->text().toStdString();
   this->form->treat_as_power(this->ui.treatAsPower->isChecked());
   this->form->menuDisplayObjectID = this->ui.menuDisplayObject->formID();
   this->form->description = this->ui.description->toPlainText().toStdString();
   this->ui.word0->save();
   this->ui.word1->save();
   this->ui.word2->save();
   //
   emit DovahKitCore::get().formModified(stub);
}