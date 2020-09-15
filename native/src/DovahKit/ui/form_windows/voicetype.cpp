#include "voicetype.h"
#include "../../helpers/bitwise.h"
#include "../../editor/core.h"

FormDialogVoicetype::FormDialogVoicetype(dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   if (stub->formType == dovah::form_type::voicetype)
      this->form = stub->load().ptr_cast<dovah::loaded_forms::Voicetype>();
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
void FormDialogVoicetype::load() {
   if (!this->form)
      return;
   this->ui.editorID->setText(QString::fromStdString(this->form->stub->get_editor_id()));
   this->ui.gender->setCurrentIndex((this->form->voicetype_flags & dovah::loaded_forms::Voicetype::voicetype_flag::female) ? 1 : 0);
   this->ui.allowDefault->setChecked((this->form->voicetype_flags & dovah::loaded_forms::Voicetype::voicetype_flag::allow_default_dialogue) != 0);
}
void FormDialogVoicetype::save() {
   if (!this->form)
      return;
   auto stub = this->form->stub;
   stub->set_edited(true);
   stub->editorID = this->ui.editorID->text().toStdString();
   cobb::modify_bit(this->form->voicetype_flags, dovah::loaded_forms::Voicetype::voicetype_flag::female, this->ui.gender->currentIndex() == 1);
   cobb::modify_bit(this->form->voicetype_flags, dovah::loaded_forms::Voicetype::voicetype_flag::allow_default_dialogue, this->ui.allowDefault->isChecked());
   //
   emit DovahKitCore::get().formModified(stub);
}