#include "voicetype.h"
#include "./_base_cpp.h"
#include "helpers/bitwise.h"

FormDialogVoicetype::FormDialogVoicetype(dovah::form_stub* stub, QWidget* parent) : FormEditDialogBase(stub, parent) {
   form_dialog_helpers::initialize(*this, stub);
   //
   this->load();
}
void FormDialogVoicetype::_load_impl() {
   this->ui.editorID->setText(QString::fromStdString(this->form->stub.get_editor_id()));
   this->ui.gender->setCurrentIndex((this->form->voicetype_flags & dovah::loaded_forms::Voicetype::voicetype_flag::female) ? 1 : 0);
   this->ui.allowDefault->setChecked((this->form->voicetype_flags & dovah::loaded_forms::Voicetype::voicetype_flag::allow_default_dialogue) != 0);
}
void FormDialogVoicetype::_save_impl() {
   this->stub->editorID = this->ui.editorID->text().toStdString();
   cobb::modify_bit(this->form->voicetype_flags, dovah::loaded_forms::Voicetype::voicetype_flag::female, this->ui.gender->currentIndex() == 1);
   cobb::modify_bit(this->form->voicetype_flags, dovah::loaded_forms::Voicetype::voicetype_flag::allow_default_dialogue, this->ui.allowDefault->isChecked());
}