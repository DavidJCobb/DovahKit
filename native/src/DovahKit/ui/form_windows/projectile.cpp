#include "./projectile.h"
#include <array>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"

FormDialogProjectile::FormDialogProjectile(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      size_t count = this->ui.type->count();
      for (size_t i = 0; i < count; ++i)
         this->ui.type->setItemData(i, (int)(1 << i)); // loaded_form_type::projectile_type
   }

   this->ui.light->setAllowedFormType(dovah::form_type::light);
   this->ui.defaultSource->setAllowedFormType(dovah::form_type::weapon);
   this->ui.decalData->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.flybySound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.muzzleFlashLight->setAllowedFormType(dovah::form_type::light);
   this->ui.explosionCountdownSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.collisionLayer->setAllowedFormType(dovah::form_type::collision_layer);
   this->ui.disarmSound->setAllowedFormType(dovah::form_type::sound_descriptor);

   for (auto* spinbox : std::array<QDoubleSpinBox*, 13>{
      this->ui.speed,
      this->ui.gravity,
      this->ui.range,
      this->ui.impactForce,
      this->ui.tracerChance,
      this->ui.fadeDuration,
      this->ui.coneSpread,
      this->ui.collisionRadius,
      this->ui.lifetime,
      this->ui.muzzleFlashDuration,
      this->ui.explosionTimer,
      this->ui.explosionProximity,
      this->ui.relaunchInterval,
   }) {
      spinbox->setRange(0, 10000);
   }

   this->ui.explosionTriggerImpact->setChecked(true);

   this->load(); // this creates the working copy.
}
void FormDialogProjectile::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   ui::bind(this->ui.type, working.type);
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.light, working.light, working);
   ui::bind(this->ui.defaultSource,  working.default_weapon_source, working);
   ui::bind(this->ui.decalData, working.decal, working);
   ui::bind(this->ui.flagSupersonic, working.flags, loaded_form_type::flag::supersonic);
   ui::bind(this->ui.flybySound, working.sounds.flyby, working);
   ui::bind(this->ui.detectionLoudness, working.loudness);
   ui::bind(this->ui.speed, working.speed);
   ui::bind(this->ui.gravity, working.gravity);
   ui::bind(this->ui.range, working.range);
   ui::bind(this->ui.impactForce, working.impact_force);
   ui::bind(this->ui.tracerChance, working.tracer_chance);
   ui::bind(this->ui.fadeDuration, working.fade_duration);
   ui::bind(this->ui.coneSpread, working.cone_spread);
   ui::bind(this->ui.collisionRadius, working.collision_radius);
   ui::bind(this->ui.lifetime, working.lifetime);

   ui::bind(this->ui.muzzleFlash, working.flags, loaded_form_type::flag::muzzle_flash);
   ui::bind(this->ui.muzzleFlashLight, working.muzzle_flash.light, working);
   ui::bind(this->ui.muzzleFlashEffect, working.muzzle_flash.model, working);
   ui::bind(this->ui.muzzleFlashDuration, working.muzzle_flash.duration);
   //
   ui::bind(this->ui.explosion, working.flags, loaded_form_type::flag::explosion);
   ui::bind(this->ui.explosionForm, working.explosion.form, working);
   ui::bind(this->ui.explosionTriggerAlt, working.flags, loaded_form_type::flag::alt_trigger);
   ui::bind(this->ui.explosionTimer, working.explosion.alt_trigger.timer);
   ui::bind(this->ui.explosionProximity, working.explosion.alt_trigger.proximity);
   ui::bind(this->ui.countdownSound, working.sounds.countdown, working);
   //
   ui::bind(this->ui.collisionLayer, working.collision_layer, working);
   ui::bind(this->ui.flagHitscan, working.flags, loaded_form_type::flag::hitscan);
   ui::bind(this->ui.flagCanDisarm, working.flags, loaded_form_type::flag::can_be_disabled);
   ui::bind(this->ui.disarmSound, working.sounds.disarm, working);
   ui::bind(this->ui.flagCanBePickedUp, working.flags, loaded_form_type::flag::can_be_taken);
   ui::bind(this->ui.flagCritPinsLimbs, working.flags, loaded_form_type::flag::pins_limbs);
   ui::bind(this->ui.flagPassThroughSmallTransparent, working.flags, loaded_form_type::flag::pass_through_small_transparent);
   ui::bind(this->ui.flagDisableCombatAimCorrection, working.flags, loaded_form_type::flag::disable_combat_aim_correction);
   ui::bind(this->ui.relaunchInterval, working.relaunch_interval);
}
void FormDialogProjectile::_save_impl() {
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
}