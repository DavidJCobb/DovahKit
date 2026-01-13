#include "./movable_static.h"
#include "dovah/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

FormDialogMovableStatic::FormDialogMovableStatic(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.loopSound->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->load(); // this creates the working copy.
}
void FormDialogMovableStatic::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.loopSound, working.loop_sound, working);
   {  // Flags
      auto& record_flags = this->record_flags();
      ui::bind(this->ui.flagHasCurrents,     record_flags, loaded_form_type::form_flag::has_currents);
      ui::bind(this->ui.flagObstacle,        record_flags, loaded_form_type::form_flag::obstacle);
      ui::bind(this->ui.flagMustUpdateAnims, record_flags, loaded_form_type::form_flag::must_update_anims);
      ui::bind(this->ui.flagRandomAnimStart, record_flags, loaded_form_type::form_flag::random_anim_start);
      ui::bind_inverse(this->ui.flagOnLocalMap, record_flags, loaded_form_type::form_flag::hide_from_local_map);
   }
   ui::bind(this->ui.navmeshGeneration, this->record_flags());
}
void FormDialogMovableStatic::_save_impl() {
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
}