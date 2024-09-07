#include "./soul_gem.h"
#include <limits>
#include "dovah/data/soul_size.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

namespace {
   // Currently disabled: the Creation Kit doesn't allow you to edit VMAD data 
   // on Soul Gems, even though they seem to support it; and xEdit wouldn't be 
   // able to load such data AFAIK.
   constexpr const bool allow_editing_papyrus = false;
}

FormDialogSoulGem::FormDialogSoulGem(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   this->ui.weight->setMaximum(std::numeric_limits<float>::max());
   this->ui.value->setMaximum(std::numeric_limits<int32_t>::max());

   auto _set_up_soul_picker = [](QComboBox* widget) {
      widget->clear();
      widget->addItem(tr("None",    "soul sizes"), (int)dovah::soul_size::none);
      widget->addItem(tr("Petty",   "soul sizes"), (int)dovah::soul_size::petty);
      widget->addItem(tr("Lesser",  "soul sizes"), (int)dovah::soul_size::lesser);
      widget->addItem(tr("Common",  "soul sizes"), (int)dovah::soul_size::common);
      widget->addItem(tr("Greater", "soul sizes"), (int)dovah::soul_size::greater);
      widget->addItem(tr("Grand",   "soul sizes"), (int)dovah::soul_size::grand);
   };
   _set_up_soul_picker(this->ui.soulInitial);
   _set_up_soul_picker(this->ui.soulMaximum);
   this->ui.linkedTo->setAllowNone(true);
   this->ui.linkedTo->setAllowedFormType(dovah::form_type::soul_gem);

   if constexpr (!allow_editing_papyrus) {
      this->ui.scriptListPane->setVisible(false);
   }

   this->load(); // this creates the working copy.
}
void FormDialogSoulGem::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   ui::bind(this->ui.weight, working.weight);
   ui::bind(this->ui.value,  working.value);
   ui::bind_inverse(this->ui.flagPlayable, this->record_flags(), loaded_form_type::form_flag::non_playable);
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.soundTake, working.take_sound, working);
   ui::bind(this->ui.soundDrop, working.drop_sound, working);

   this->ui.keywords->pullStubs(working.keywords.forms);
   if constexpr (allow_editing_papyrus) {
      this->ui.scriptListPane->setFormWorkingCopy(&working);
   }

   // Soul Gem options
   ui::bind(this->ui.soulInitial, working.initial_soul_size);
   ui::bind(this->ui.soulMaximum, working.maximum_soul_size);
   ui::bind(this->ui.linkedTo, working.linked_to, working);
}
void FormDialogSoulGem::_save_impl() {
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
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.keywords->commitStubs(working.keywords.forms, working);
   if constexpr (allow_editing_papyrus) {
      this->ui.scriptListPane->commit();
   }
}