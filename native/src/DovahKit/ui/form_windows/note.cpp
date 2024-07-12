#include "./note.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogNote::FormDialogNote(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);

   {
      auto* widget = this->ui.noteType;
      widget->clear();
      widget->addItem(tr("Sound",    "note type"), (int)loaded_form_type::note_type::sound);
      widget->addItem(tr("Text",     "note type"), (int)loaded_form_type::note_type::text);
      widget->addItem(tr("Image",    "note type"), (int)loaded_form_type::note_type::image);
      widget->addItem(tr("Dialogue", "note type"), (int)loaded_form_type::note_type::voice);
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         auto type_index = this->ui.noteType->currentData().toInt();
         this->form->type = (loaded_form_type::note_type)type_index;
         this->ui.noteData->setCurrentIndex(type_index);
      });
   }
   this->ui.noteDataSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.noteDataSpeaker->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.noteDataTopic->setAllowedFormType(dovah::form_type::topic);

   this->load(); // this creates the working copy.
}
void FormDialogNote::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.icon, working.icon);
   ui::bind(this->ui.soundTake, working.take_sound, working);
   ui::bind(this->ui.soundDrop, working.drop_sound, working);

   ui::bind(this->ui.noteType, working.type);
   //
   ui::bind(this->ui.noteDataSound,   working.content.sound, working);
   ui::bind(this->ui.noteDataImage,   working.content.image);
   ui::bind(this->ui.noteDataSpeaker, working.content.speaker, working);
   ui::bind(this->ui.noteDataTopic,   working.content.topic, working);
   this->ui.noteDataText->setPlainText(editor.convert_localized_string(working.content.text));
}
void FormDialogNote::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);

   editor.assign_localized_string(working.content.text, this->ui.noteDataText->toPlainText());
}