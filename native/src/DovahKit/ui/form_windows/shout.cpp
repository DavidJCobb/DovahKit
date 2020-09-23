#include "shout.h"
#include "_base_cpp.h"

FormDialogShout::FormDialogShout(dovah::form_stub* stub, QWidget* parent) : FormDialogBaseTemplate(stub, parent) {
   form_dialog_helpers::initialize<FormDialogShout, dovah::loaded_forms::Shout>(*this, stub);
   this->ui.word0->linkToForm(0, stub);
   this->ui.word1->linkToForm(1, stub);
   this->ui.word2->linkToForm(2, stub);
   //
   this->ui.menuDisplayObject->addFormType(dovah::form_type::statik);
   this->ui.menuDisplayObject->setAllowNone(true);
   this->ui.menuDisplayObject->populate();
   //
   this->load();
}
void FormDialogShout::_load_impl() {
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.name->setText(this->form->name.c_str());
   this->ui.treatAsPower->setChecked(this->form->treat_as_power());
   this->ui.menuDisplayObject->setFormByID(this->form->menuDisplayObjectID);
   this->ui.description->setPlainText(this->form->description.c_str());
   this->ui.word0->load();
   this->ui.word1->load();
   this->ui.word2->load();
}
void FormDialogShout::_save_impl() {
   this->stub->editorID = this->ui.editorID->text().toStdString();
   this->form->name = this->ui.name->text().toStdString();
   this->form->treat_as_power(this->ui.treatAsPower->isChecked());
   this->form->menuDisplayObjectID = this->ui.menuDisplayObject->formID();
   this->form->description = this->ui.description->toPlainText().toStdString();
   this->ui.word0->save();
   this->ui.word1->save();
   this->ui.word2->save();
}