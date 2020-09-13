#include "shout_word.h"

FormShoutWordEditor::FormShoutWordEditor(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   this->initialize();
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
   this->ui.word->setFormByID(word.wordOfPowerID);
   this->ui.spell->setFormByID(word.spellID);
   this->ui.recoveryTime->setValue(word.recoveryTime);
}
void FormShoutWordEditor::save() {
   if (!this->form)
      return;
   auto& list = this->form->words;
   if (list.size() >= this->which_word)
      return;
   auto& word = list[this->which_word];
   word.wordOfPowerID = this->ui.word->formID();
   word.spellID       = this->ui.spell->formID();
   word.recoveryTime  = this->ui.recoveryTime->value();
}
void FormShoutWordEditor::linkToForm(int which_word, dovah::form_stub* stub) {
   this->which_word = which_word;
   if (stub->formType == dovah::form_type::shout)
      this->form = stub->load().ptr_cast<dovah::loaded_forms::Shout>();
   else
      this->form = nullptr;
}