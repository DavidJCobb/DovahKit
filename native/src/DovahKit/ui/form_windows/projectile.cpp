#include "./projectile.h"
#include <array>
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/enum_dropdown_configs/detection_loudness.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"

FormDialogProjectile::FormDialogProjectile(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      size_t count = this->ui.type->count();
      for (size_t i = 0; i < count; ++i)
         this->ui.type->setItemData(i, (int)(1 << i)); // loaded_form_type::projectile_type
   }

   ui::enum_dropdown_configs::detection_loudness(this->ui.detectionLoudness);

   this->ui.explosionForm->setAllowedFormType(dovah::form_type::explosion);
   this->ui.light->setAllowedFormType(dovah::form_type::light);
   this->ui.defaultWeaponSource->setAllowedFormType(dovah::form_type::weapon);
   this->ui.decalData->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.flybySound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.muzzleFlashLight->setAllowedFormType(dovah::form_type::light);
   this->ui.explosionCountdownSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.collisionLayer->setAllowedFormType(dovah::form_type::collision_layer);
   this->ui.disarmSound->setAllowedFormType(dovah::form_type::sound_descriptor);

   QObject::connect(this->ui.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormDialogProjectile::_update_type_related_enable_states);
   this->_update_type_related_enable_states();

   for (auto* spinbox : std::array{
      this->ui.speed,
      this->ui.gravity,
      this->ui.range,
      this->ui.impactForce,
      this->ui.fadeDuration,
      this->ui.coneSpread,
      this->ui.collisionRadius,
      this->ui.lifetime,
      this->ui.explosionTimer,
      this->ui.explosionProximity,
   }) {
      ui::set_unsigned_range<float>(spinbox);
   }
   this->ui.muzzleFlashDuration->setRange(0, 5);
   this->ui.relaunchInterval->setRange(0, 5);
   this->ui.tracerChance->setRange(0, 1);

   this->ui.explosionTriggerImpact->setChecked(true);

   this->load(); // this creates the working copy.
}
void FormDialogProjectile::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.type, working.type);
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.light, working.light, working);
   ui::bind(this->ui.defaultWeaponSource,  working.default_weapon_source, working);
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
   this->ui.muzzleFlashEffect->initializeFrom(working.muzzle_flash.model);
   ui::bind(this->ui.muzzleFlashDuration, working.muzzle_flash.duration);
   //
   ui::bind(this->ui.explosionSettings, working.flags, loaded_form_type::flag::explosion);
   ui::bind(this->ui.explosionForm, working.explosion.form, working);
   ui::bind(this->ui.explosionTriggerAlt, working.flags, loaded_form_type::flag::alt_trigger);
   ui::bind(this->ui.explosionTimer, working.explosion.alt_trigger.timer);
   ui::bind(this->ui.explosionProximity, working.explosion.alt_trigger.proximity);
   ui::bind(this->ui.explosionCountdownSound, working.sounds.countdown, working);
   //
   ui::bind(this->ui.collisionLayer, working.collision_layer, working);
   ui::bind(this->ui.flagHitscan, working.flags, loaded_form_type::flag::hitscan);
   ui::bind(this->ui.flagCanBeDisarmed, working.flags, loaded_form_type::flag::can_be_disabled);
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
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.muzzleFlashEffect->commitTo(working.muzzle_flash.model, working);
}

void FormDialogProjectile::_update_type_related_enable_states() {
   auto type = (loaded_form_type::projectile_type)this->ui.type->currentData().toInt();

   this->ui.coneSpread->setEnabled(type == loaded_form_type::projectile_type::cone);
   switch (type) {
      case loaded_form_type::projectile_type::barrier:
      case loaded_form_type::projectile_type::beam:
      case loaded_form_type::projectile_type::flame:
         this->ui.collisionRadius->setEnabled(false);
         break;
      default:
         this->ui.collisionRadius->setEnabled(true);
         break;
   }
   switch (type) {
      case loaded_form_type::projectile_type::beam:
      case loaded_form_type::projectile_type::flame:
      case loaded_form_type::projectile_type::lobber:
         this->ui.gravity->setEnabled(false);
         break;
      default:
         this->ui.gravity->setEnabled(true);
         break;
   }
   switch (type) {
      case loaded_form_type::projectile_type::arrow:
      case loaded_form_type::projectile_type::cone:
      case loaded_form_type::projectile_type::flame:
      case loaded_form_type::projectile_type::missile:
         this->ui.lifetime->setEnabled(false);
         break;
      default:
         this->ui.lifetime->setEnabled(true);
         break;
   }
   switch (type) {
      case loaded_form_type::projectile_type::lobber:
         this->ui.range->setEnabled(false);
         this->ui.flagHitscan->setEnabled(false);
         break;
      default:
         this->ui.range->setEnabled(true);
         break;
   }
   switch (type) {
      case loaded_form_type::projectile_type::lobber:
         this->ui.tracerChance->setEnabled(false);
         break;
      default:
         this->ui.tracerChance->setEnabled(true);
         break;
   }

   switch (type) {
      case loaded_form_type::projectile_type::beam:
      case loaded_form_type::projectile_type::flame:
         this->ui.flagCanBeDisarmed->setEnabled(false);
         this->ui.flagCanBePickedUp->setEnabled(false);
         this->ui.flagHitscan->setEnabled(false);
         break;
      case loaded_form_type::projectile_type::lobber:
         this->ui.flagHitscan->setEnabled(false);
         break;
      default:
         this->ui.flagHitscan->setEnabled(true);
         break;
   }
}