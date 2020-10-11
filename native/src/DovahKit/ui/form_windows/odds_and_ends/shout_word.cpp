#include "shout_word.h"
#include "../_base_cpp.h"
#include "../../../editor/core.h"

FormShoutWordEditor::FormShoutWordEditor(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   this->initialize();
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->form = nullptr;
      this->stub = nullptr;
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub == this->stub) {
         this->form = nullptr;
         this->stub = nullptr;
      }
   });
}
void FormShoutWordEditor::initialize() {
   this->ui.word->addFormType(dovah::form_type::word_of_power);
   this->ui.word->setAllowNone(true);
   this->ui.spell->addFormType(dovah::form_type::spell);
   this->ui.spell->setAllowNone(true);
   //
   this->ui.word->populate();
   this->ui.word->setFormByID(0);
   this->ui.spell->populate();
   this->ui.spell->setFormByID(0);
}
void FormShoutWordEditor::load() {
   if (!this->form)
      return;
   auto& list = this->form->words;
   if (list.size() <= this->which_word)
      return;
   auto& word = list[this->which_word];
   this->ui.word->setFormByID(word.word_of_power.formID());
   this->ui.spell->setFormByID(word.spell.formID());
   this->ui.recoveryTime->setValue(word.recoveryTime);
}
void FormShoutWordEditor::save() {
   if (!this->form)
      return;
   auto& list = this->form->words;
   if (list.size() <= this->which_word)
      return;
   auto& word = list[this->which_word];
   this->save_form_id(word.word_of_power, this->ui.word->formStub());
   this->save_form_id(word.spell,         this->ui.spell->formStub());
   word.recoveryTime  = this->ui.recoveryTime->value();
}
void FormShoutWordEditor::linkToForm(int which_word, dovah::form_stub* stub) {
   this->which_word = which_word;
   if (stub->formType == dovah::form_type::shout) {
      this->form = stub->load().ptr_cast<dovah::loaded_forms::Shout>();
      this->stub = stub;
   } else {
      this->form = nullptr;
      this->stub = nullptr;
   }
}
void FormShoutWordEditor::save_form_id(dovah::form_reference_t& target, dovah::bare_form_id_t id) {
   auto& editor = DovahKitCore::get();
   auto* value  = editor.get_form(id);
   this->save_form_id(target, value);
}
void FormShoutWordEditor::save_form_id(dovah::form_reference_t& target, dovah::form_stub* value) {
   target.set(*this->stub, value);
}