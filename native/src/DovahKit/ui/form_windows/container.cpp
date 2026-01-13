#include "./container.h"
#include <limits>
#include "dovah/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

FormDialogContainer::FormDialogContainer(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.soundOpen->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundClose->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->ui.weight->setMaximum(std::numeric_limits<float>::max());

   this->load(); // this creates the working copy.
}
void FormDialogContainer::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.weight, working.weight);
   ui::bind(this->ui.flagRandomAnimStart,     this->record_flags(),    loaded_form_type::form_flag::random_anim_start);
   ui::bind(this->ui.flagObstacle,            this->record_flags(),    loaded_form_type::form_flag::obstacle);
   ui::bind(this->ui.flagRespawns,            working.container_flags, loaded_form_type::container_flag::respawns);
   ui::bind(this->ui.flagShowOwner,           working.container_flags, loaded_form_type::container_flag::show_owner);
   ui::bind(this->ui.flagSoundsEvenWhenAnims, working.container_flags, loaded_form_type::container_flag::animation_allows_sounds);
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.navmeshGeneration, this->record_flags());
   ui::bind(this->ui.soundOpen,  working.open_sound,  working);
   ui::bind(this->ui.soundClose, working.close_sound, working);

   this->ui.inventory->initializeFrom(working.inventory);
   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogContainer::_save_impl() {
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

   this->ui.inventory->commitTo(working.inventory, working);
   this->ui.scriptListPane->commit();
}