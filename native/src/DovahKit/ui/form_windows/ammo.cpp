#include "./ammo.h"
#include <limits>
#include "editor/subsystems/game_localized_strings/core.h"
#include "editor/core.h"
#include "ui/utils/bind.h"

FormDialogAmmo::FormDialogAmmo(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   this->ui.projectile->setAllowedFormType(dovah::form_type::projectile);

   this->ui.damage->setMaximum(std::numeric_limits<float>::max());
   this->ui.weight->setMaximum(std::numeric_limits<float>::max());
   this->ui.value->setMaximum(std::numeric_limits<int32_t>::max());

   {
      auto& editor = DovahKitCore::get();
      this->ui.weight->setEnabled(editor.get_current_game() != dovah::game::skyrim_classic);
      //
      // Account for the user converting across games:
      //
      QObject::connect(&editor, &DovahKitCore::dataSaveComplete, this, [this]() {
         this->ui.weight->setEnabled(DovahKitCore::get().get_current_game() != dovah::game::skyrim_classic);
      });
   }

   this->load(); // this creates the working copy.
}
void FormDialogAmmo::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.shortName, working.short_name);
   ui::bind(this->ui.projectile, working.projectile, working);
   ui::bind(this->ui.value,  working.value);
   ui::bind(this->ui.damage, working.damage);
   ui::bind(this->ui.weight, working.weight);
   ui::bind(this->ui.flagIgnoreDamageResist, working.ammo_flags, loaded_form_type::ammo_flag::ignores_normal_weapon_resist);
   ui::bind_inverse(this->ui.flagPlayable, working.ammo_flags, loaded_form_type::ammo_flag::non_playable); // There's a "non-playable" record flag, but Bethesda doesn't seem to use it.
   ui::bind_inverse(this->ui.flagCrossbowBolt, working.ammo_flags, loaded_form_type::ammo_flag::non_crossbow_bolt);
   this->ui.keywords->pullStubs(working.keywords.forms);
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.iconInventory, working.icon);
   ui::bind(this->ui.iconMessage, working.message_icon);
   ui::bind(this->ui.soundTake, working.take_sound, working);
   ui::bind(this->ui.soundDrop, working.drop_sound, working);
   this->ui.description->setPlainText(gls.convert_localized_string(working.description));
}
void FormDialogAmmo::_save_impl() {
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
   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());
}