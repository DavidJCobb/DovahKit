#include "./explosion.h"
#include <limits>
#include "dovah/data/all_base_form_types.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/enum_dropdown_configs/detection_loudness.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"

FormDialogExplosion::FormDialogExplosion(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.chainProjectile->setAllowedFormType(dovah::form_type::projectile);
   this->ui.spell->setAllowedFormTypes({ dovah::form_type::enchantment, dovah::form_type::spell });
   this->ui.light->setAllowedFormType(dovah::form_type::light);
   this->ui.imagespaceMod->setAllowedFormType(dovah::form_type::imagespace_modifier);
   this->ui.sound1->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.sound2->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.spawnObject->setAllowedFormTypes(QList<dovah::form_type>{ dovah::all_base_form_types.begin(), dovah::all_base_form_types.end() });

   ui::set_unsigned_range<float>(this->ui.damage);
   ui::set_unsigned_range<float>(this->ui.force); // TODO: are negative forces meaningful? would they pull objects inward?
   ui::set_unsigned_range<float>(this->ui.radius);
   ui::set_unsigned_range<float>(this->ui.imagespaceRadius);
   ui::set_range<float>(this->ui.verticalOffset);

   {
      auto* widget = this->ui.knockdown;
      widget->clear();
      widget->addItem(tr("Never"),      (int)loaded_form_type::knockdown_type::never);
      widget->addItem(tr("By Formula"), (int)loaded_form_type::knockdown_type::by_formula);
      widget->addItem(tr("NPCs Only"),  (int)loaded_form_type::knockdown_type::only_npcs);
      widget->addItem(tr("Always"),     (int)loaded_form_type::knockdown_type::always);
   }
   ui::enum_dropdown_configs::detection_loudness(this->ui.detectionSoundLevel);

   this->load(); // this creates the working copy.
}
void FormDialogExplosion::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.force, working.force);
   ui::bind(this->ui.flagPushSourceOnly, working.explosion_flags, loaded_form_type::explosion_flag::push_source_ref_only);
   ui::bind(this->ui.flagIgnoreLOS,      working.explosion_flags, loaded_form_type::explosion_flag::ignore_los_check);
   ui::bind(this->ui.damage, working.damage);
   ui::bind(this->ui.radius, working.radius);
   ui::bind(this->ui.verticalOffset, working.vertical_offset);
   ui::bind(this->ui.spell, working.enchantable.effect, working);
   ui::bind(this->ui.knockdown, working.knockdown);
   ui::bind(this->ui.detectionSoundLevel, working.loudness);
   ui::bind(this->ui.spawnObject, working.placed_object, working);
   ui::bind(this->ui.flagChain, working.explosion_flags, loaded_form_type::explosion_flag::chain);
   ui::bind(this->ui.chainProjectile, working.projectile, working);
   ui::bind(this->ui.imagespaceMod, working.imagespace_modifier, working);
   ui::bind(this->ui.imagespaceRadius, working.imagespace_radius);
   ui::bind(this->ui.flagIgnoreImagespaceSwap, working.explosion_flags, loaded_form_type::explosion_flag::ignore_imagespace_swap);
   ui::bind(this->ui.light, working.light, working);
   ui::bind(this->ui.sound1, working.sounds[0], working);
   ui::bind(this->ui.sound2, working.sounds[1], working);
   ui::bind(this->ui.impactDataSet, working.impact_data_set, working);
   ui::bind(this->ui.flagNoRumble,          working.explosion_flags, loaded_form_type::explosion_flag::no_controller_vibration);
   ui::bind(this->ui.flagAlwaysWorldOrient, working.explosion_flags, loaded_form_type::explosion_flag::use_world_orientation);
}
void FormDialogExplosion::_save_impl() {
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
}