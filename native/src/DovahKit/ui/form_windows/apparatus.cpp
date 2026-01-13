#include "./apparatus.h"
#include <limits>
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"
#include "dovah/forms/MiscItem.h" // "non-playable" flag

FormDialogApparatus::FormDialogApparatus(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* widget = this->ui.quality;
      widget->clear();
      widget->addItem(tr("Novice"), (int)dovah::skill_level::novice);
      widget->addItem(tr("Apprentice"), (int)dovah::skill_level::apprentice);
      widget->addItem(tr("Journeyman"), (int)dovah::skill_level::journeyman);
      widget->addItem(tr("Expert"), (int)dovah::skill_level::expert);
      widget->addItem(tr("Master"), (int)dovah::skill_level::master);
   }
   
   this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->ui.weight->setMaximum(std::numeric_limits<float>::max());
   this->ui.value->setMaximum(std::numeric_limits<int32_t>::max());

   this->load(); // this creates the working copy.
}
void FormDialogApparatus::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.weight, working.weight);
   ui::bind(this->ui.value,  working.value);
   ui::bind_inverse(this->ui.flagPlayable, this->record_flags(), dovah::loaded_forms::MiscItem::form_flag::non_playable);
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.soundTake, working.sounds.take, working);
   ui::bind(this->ui.soundDrop, working.sounds.drop, working);

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogApparatus::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.scriptListPane->commit();
}