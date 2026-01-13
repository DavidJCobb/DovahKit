#include "./shout_word.h"
#include "editor/core.h"

FormShoutWordEditor::FormShoutWordEditor(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);

   this->ui.word->setAllowedFormType(dovah::form_type::word_of_power);
   this->ui.spell->setAllowedFormType(dovah::form_type::spell);

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

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->ui.word);
}
void FormShoutWordEditor::load() {
   if (!this->form)
      return;
   auto& list = this->form->words;
   if (list.size() <= this->which_word)
      return;
   auto& word = list[this->which_word];
   this->ui.word->setFormStub(word.word_of_power.get_form_stub());
   this->ui.spell->setFormStub(word.spell.get_form_stub());
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
   if (stub->form_type == dovah::form_type::shout) {
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
   target.set(*this->form, value);
}