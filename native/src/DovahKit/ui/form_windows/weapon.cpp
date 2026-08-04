#include "./weapon.h"
#include <limits>
#include "dovah/data/actor_values.h"
#include "dovah/data/skills.h"
#include "editor/helpers/skill_name_to_string.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/enum_dropdown_configs/actor_value_index.h"
#include "ui/utils/enum_dropdown_configs/detection_loudness.h"
#include "ui/utils/enum_dropdown_configs/resistance_av_index.h"
#include "ui/utils/enum_dropdown_configs/skill.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogWeapon::FormDialogWeapon(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   #pragma region Base data
      this->ui.enchantmentForm->setAllowedFormTypes({ dovah::form_type::enchantment, dovah::form_type::spell });
      ui::set_range<decltype(dovah::loaded_forms::components::enchantable::charge)>(this->ui.enchantmentCharge);
      ui::set_range<int32_t>(this->ui.value); // TODO: we need a QSpinBox that can hold a uint32_t, not just an int32_t (int)
      this->ui.templateForm->setAllowedFormType(dovah::form_type::weapon);
   #pragma endregion

   #pragma region Game data
      ui::set_unsigned_range<decltype(loaded_form_type::damage)>(this->ui.damage);
      this->ui.ironsightFOV->setRange(0, 179.9F);
      ui::set_range<decltype(loaded_form_type::projectile_count)>(this->ui.projectileCount);
      ui::set_unsigned_range<float>(this->ui.reach);
      ui::set_unsigned_range<float>(this->ui.speed);
      ui::set_unsigned_range<float>(this->ui.stagger);
      ui::set_unsigned_range<float>(this->ui.weight);

      ui::enum_dropdown_configs::skill(this->ui.skill, true);
      this->ui.equipType->setAllowedFormType(dovah::form_type::equip_slot);
      ui::enum_dropdown_configs::resistance_av_index(this->ui.resist);
      {
         auto* widget = this->ui.onHitCalc;
         widget->clear();
         widget->addItem(tr("Normal formula behavior"), (int)loaded_form_type::hit_gore::normal);
         widget->addItem(tr("Dismember only"), (int)loaded_form_type::hit_gore::dismember_only);
         widget->addItem(tr("Explode only"), (int)loaded_form_type::hit_gore::explode_only);
         widget->addItem(tr("Never dismember or explode"), (int)loaded_form_type::hit_gore::no_dismember_or_explode);
      }
      ui::set_range<decltype(loaded_form_type::base_vats_hit_chance)>(this->ui.baseVATSChance);

      #pragma region Combat AI ranges
         ui::set_unsigned_range<float>(this->ui.combatAIRangeMin);
         ui::set_unsigned_range<float>(this->ui.combatAIRangeMax);
      #pragma endregion

      this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

      #pragma region Critical hits
         ui::set_range<float>(this->ui.critChanceMult);
         ui::set_range<uint16_t>(this->ui.critDamage);
         this->ui.critEffect->setAllowedFormType(dovah::form_type::spell);
      #pragma endregion

      #pragma region Embedded weapon
         ui::enum_dropdown_configs::actor_value_index<ui::enum_dropdown_configs::actor_value_index_options{
            .allow_none = true,
            .sorted     = true,
         }>(
            this->ui.embeddedAV,
            [](const dovah::actor_value_info& av_info) {
               return av_info.type == dovah::actor_value_type::limb_condition;
            }
         );
      #pragma endregion
   #pragma endregion

   #pragma region Art and Sound
      this->ui.firstPersonModel->setAllowedFormType(dovah::form_type::statik);
      this->ui.impactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
      this->ui.impactDataSetBlockBash->setAllowedFormType(dovah::form_type::impact_data_set);
      this->ui.alternateBlockMaterial->setAllowedFormType(dovah::form_type::material_type);

      #pragma region Animation
         {
            auto* widget = this->ui.weaponType;
            widget->clear();
            widget->addItem(tr("Battleaxe/Warhammer (2H)"), (int)dovah::weapon_type::battleaxe);
            widget->addItem(tr("Bow"), (int)dovah::weapon_type::bow);
            widget->addItem(tr("Crossbow"), (int)dovah::weapon_type::crossbow);
            widget->addItem(tr("Dagger"), (int)dovah::weapon_type::one_hand_dagger);
            widget->addItem(tr("Greatsword (2H)"), (int)dovah::weapon_type::greatsword);
            widget->addItem(tr("Hand-to-Hand"), (int)dovah::weapon_type::hand_to_hand_melee);
            widget->addItem(tr("Staff"), (int)dovah::weapon_type::staff);
            widget->addItem(tr("Sword (1H)"), (int)dovah::weapon_type::one_hand_sword);
            widget->addItem(tr("Mace"), (int)dovah::weapon_type::one_hand_mace);
            widget->addItem(tr("War Axe (1H)"), (int)dovah::weapon_type::one_hand_axe);
            widget->model()->sort(0, Qt::SortOrder::AscendingOrder);
         }
         {
            auto* widget = this->ui.attackAnim;
            widget->clear();
            widget->addItem("DEFAULT", (int)loaded_form_type::legacy_attack_animation::DEFAULT);
            widget->addItem("AttackLeft", (int)loaded_form_type::legacy_attack_animation::AttackLeft);
            widget->addItem("AttackRight", (int)loaded_form_type::legacy_attack_animation::AttackRight);
            widget->addItem("Attack3", (int)loaded_form_type::legacy_attack_animation::Attack3);
            widget->addItem("Attack4", (int)loaded_form_type::legacy_attack_animation::Attack4);
            widget->addItem("Attack5", (int)loaded_form_type::legacy_attack_animation::Attack5);
            widget->addItem("Attack6", (int)loaded_form_type::legacy_attack_animation::Attack6);
            widget->addItem("Attack7", (int)loaded_form_type::legacy_attack_animation::Attack7);
            widget->addItem("Attack8", (int)loaded_form_type::legacy_attack_animation::Attack8);
            widget->addItem("AttackLoop", (int)loaded_form_type::legacy_attack_animation::AttackLoop);
            widget->addItem("AttackSpin", (int)loaded_form_type::legacy_attack_animation::AttackSpin);
            widget->addItem("AttackSpin2", (int)loaded_form_type::legacy_attack_animation::AttackSpin2);
            widget->addItem("PlaceMine", (int)loaded_form_type::legacy_attack_animation::PlaceMine);
            widget->addItem("PlaceMine2", (int)loaded_form_type::legacy_attack_animation::PlaceMine2);
            widget->addItem("AttackThrow", (int)loaded_form_type::legacy_attack_animation::AttackThrow);
            widget->addItem("AttackThrow2", (int)loaded_form_type::legacy_attack_animation::AttackThrow2);
            widget->addItem("AttackThrow3", (int)loaded_form_type::legacy_attack_animation::AttackThrow3);
            widget->addItem("AttackThrow4", (int)loaded_form_type::legacy_attack_animation::AttackThrow4);
            widget->addItem("AttackThrow5", (int)loaded_form_type::legacy_attack_animation::AttackThrow5);
         }
         ui::set_unsigned_range<float>(this->ui.animAttackMult);
      #pragma endregion

      this->ui.scopeEffect->setAllowedFormType(dovah::form_type::spell);

      #pragma region Sounds
         this->ui.soundAttack->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundAttack2D->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundAttackFail->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundAttackLoop->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundIdle->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundEquip->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundUnequip->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundTake->setAllowedFormType(dovah::form_type::sound_descriptor);
         this->ui.soundDrop->setAllowedFormType(dovah::form_type::sound_descriptor);
         ui::enum_dropdown_configs::detection_loudness(this->ui.detectionSoundLevel);
      #pragma endregion

      #pragma region Rumble
         ui::set_unsigned_range<float>(this->ui.rumbleStrengthL);
         ui::set_unsigned_range<float>(this->ui.rumbleStrengthR);
         ui::set_unsigned_range<float>(this->ui.rumbleDuration);
      #pragma endregion
   #pragma endregion

   this->ui.tabbox->setCurrentIndex(0);

   this->load(); // this creates the working copy.
}
void FormDialogWeapon::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   #pragma region Base data
      ui::bind(this->ui.editorID, this->editor_id());
      this->ui.name->setText(gls.convert_localized_string(working.name));
      ui::bind(this->ui.enchantmentForm, working.enchantable.effect, working);
      ui::bind(this->ui.enchantmentCharge, working.enchantable.charge);
      ui::bind(this->ui.value, working.item_data.value);
      {
         auto* widget = this->ui.templateForm;
         ui::bind(widget, working.template_weapon, working);
         QObject::connect(widget, &DKFormPicker::formChanged, this, &FormDialogWeapon::_update_from_template_form);
         this->_update_from_template_form();
      }
      this->ui.scriptListPane->setFormWorkingCopy(&working);
   #pragma endregion

   #pragma region Game data
      ui::bind(this->ui.weight, working.item_data.weight);
      ui::bind(this->ui.damage, working.damage);
      ui::bind(this->ui.reach, working.reach);
      ui::bind(this->ui.projectileCount, working.projectile_count);
      ui::bind(this->ui.speed, working.speed);
      ui::bind(this->ui.ironsightFOV, working.ironsight_fov);
      ui::bind(this->ui.stagger, working.stagger);
      {
         auto* widget = this->ui.skill;
         {
            int i = -1;
            if (working.skill.has_value())
               i = widget->findData((int)working.skill.value());
            if (i < 0)
               i = widget->findData(-1);
            widget->setCurrentIndex(i);
         }
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget]() {
            auto s = widget->currentData().toInt();
            if (s == -1) {
               this->form->skill = {};
            } else {
               this->form->skill = (dovah::skill)s;
            }
         });
      }
      ui::bind(this->ui.equipType, working.equip_type, working);
      {
         auto* widget = this->ui.resist;
         widget->setCurrentIndex(widget->findData((int)working.resist_av));
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget]() {
            this->form->resist_av = widget->currentData().toInt();
         });
      }
      ui::bind(this->ui.onHitCalc, working.hit_gore_behavior);
      ui::bind(this->ui.baseVATSChance, working.base_vats_hit_chance);
      ui::bind(this->ui.combatAIRangeMin, working.ai_ranges.minimum);
      ui::bind(this->ui.combatAIRangeMax, working.ai_ranges.maximum);
      ui::bind(this->ui.flagFixedAIRange, working.flags.fixed_ai_range);
      ui::bind(this->ui.flagBurstShot, working.flags.burst_shot);
      ui::bind(this->ui.flagLongBursts, working.flags.long_bursts);
      this->ui.keywords->pullStubs(working.keywords.forms);

      ui::bind(this->ui.flagNPCsUseAmmo, working.flags.npcs_use_ammo);
      ui::bind(this->ui.flagNoJam, working.flags.never_jams_after_reload);
      ui::bind(this->ui.flagIgnoresNormalResist, working.flags.ignores_normal_weapon_resist);
      ui::bind(this->ui.flagMinorCrime, working.flags.minor_crime);
      ui::bind(this->ui.flagAutomatic, working.flags.automatic);
      ui::bind(this->ui.flagHideBackpack, working.flags.hide_backpack);
      ui::bind(this->ui.flagCantDrop, working.flags.cant_drop);
      ui::bind_inverse(this->ui.flagPlayable, working.flags.non_playable);
      ui::bind(this->ui.flagNotUsedInNormalCombat, working.flags.not_used_in_normal_combat);
      ui::bind(this->ui.flagNonHostile, working.flags.non_hostile);
      ui::bind(this->ui.flagBound, working.flags.bound_weapon);
      //
      ui::bind(this->ui.critChanceMult, working.crit_data.chance_mult);
      ui::bind(this->ui.critDamage, working.crit_data.added_damage);
      ui::bind(this->ui.critEffect, working.crit_data.spell_to_apply, working);
      ui::bind(this->ui.flagCritEffectOnlyOnDeath, working.crit_data.apply_spell_only_on_target_death);
      //
      ui::bind(this->ui.embedded, working.flags.embedded);
      ui::bind(this->ui.embeddedToNode, working.embedded.node);
      {
         auto* widget = this->ui.embeddedAV;
         widget->setCurrentIndex(widget->findData((int)working.embedded.actor_value));
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget]() {
            this->form->embedded.actor_value = widget->currentData().toInt();
         });
      }
      //
      this->ui.description->setPlainText(gls.convert_localized_string(working.description));
   #pragma endregion

   #pragma region Art and Sound
      this->ui.model->initializeFrom(working.model);
      ui::bind(this->ui.firstPersonModel, working.first_person_model, working);
      ui::bind(this->ui.iconInventory, working.item_data.icons.inventory);
      ui::bind(this->ui.iconMessage, working.item_data.icons.message);
      this->ui.destructionData->initializeFrom(working.destruction_data);
      ui::bind(this->ui.impactDataSet, working.impact_data_set, working);
      ui::bind(this->ui.impactDataSetBlockBash, working.block_bash.impact_data_set, working);
      ui::bind(this->ui.alternateBlockMaterial, working.block_bash.alternate_material, working);
      ui::bind(this->ui.weaponType, working.type);
      ui::bind(this->ui.attackAnim, working.animation.legacy_anim);
      ui::bind(this->ui.animAttackMult, working.animation.attack_mult);
      ui::bind(this->ui.animShotsPerSec, working.fire_rate);
      ui::bind(this->ui.flagNoFirstPersonISAnims, working.flags.no_first_person_ironsight_anim);
      ui::bind(this->ui.flagNoThirdPersonISAnims, working.flags.no_third_person_ironsight_anim);
      ui::bind(this->ui.scopeGroupbox, working.flags.has_scope);
      this->ui.scopeTargetNIF->initializeFrom(working.scope_model);
      ui::bind(this->ui.scopeEffect, working.scope_shader, working);
      
      #pragma region Sounds
         ui::bind(this->ui.soundAttack, working.sounds.attack, working);
         ui::bind(this->ui.soundAttack2D, working.sounds.attack_2D, working);
         ui::bind(this->ui.soundAttackFail, working.sounds.attack_fail, working);
         ui::bind(this->ui.soundAttackLoop, working.sounds.attack_loop, working);
         ui::bind(this->ui.soundIdle, working.sounds.idle, working);
         ui::bind(this->ui.soundEquip, working.sounds.equip, working);
         ui::bind(this->ui.soundUnequip, working.sounds.unequip, working);
         ui::bind(this->ui.detectionSoundLevel, working.loudness);
      #pragma endregion

      ui::bind(this->ui.rumbleStrengthL, working.rumble.left_motor);
      ui::bind(this->ui.rumbleStrengthR, working.rumble.right_motor);
      ui::bind(this->ui.rumbleDuration, working.rumble.duration);
      ui::bind(this->ui.flagAlternateRumble, working.flags.rumble_alternate);
   #pragma endregion
}
void FormDialogWeapon::_save_impl() {
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
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.scriptListPane->commit();
}

void FormDialogWeapon::_pull_templatable_data_to_ui() {
   auto _pull_enum = [](QComboBox* widget, auto enumeration) {
      auto i = widget->findData((int)enumeration);
      if (i < 0)
         i = widget->findData(-1);
      widget->setCurrentIndex(i);
   };

   const auto& working = *this->form;
   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   
   const auto blockers = std::array{
      QSignalBlocker(this->ui.weight),
      QSignalBlocker(this->ui.damage),
      QSignalBlocker(this->ui.reach),
      QSignalBlocker(this->ui.projectileCount),
      QSignalBlocker(this->ui.speed),
      QSignalBlocker(this->ui.ironsightFOV),
      QSignalBlocker(this->ui.stagger),
      QSignalBlocker(this->ui.skill),
      QSignalBlocker(this->ui.equipType),
      QSignalBlocker(this->ui.resist),
      QSignalBlocker(this->ui.onHitCalc),
      QSignalBlocker(this->ui.baseVATSChance),
      QSignalBlocker(this->ui.combatAIRangeMin),
      QSignalBlocker(this->ui.combatAIRangeMax),
      QSignalBlocker(this->ui.flagFixedAIRange),
      QSignalBlocker(this->ui.flagBurstShot),
      QSignalBlocker(this->ui.flagLongBursts),
      QSignalBlocker(this->ui.keywords),
      QSignalBlocker(this->ui.flagNPCsUseAmmo),
      QSignalBlocker(this->ui.flagNoJam),
      QSignalBlocker(this->ui.flagIgnoresNormalResist),
      QSignalBlocker(this->ui.flagMinorCrime),
      QSignalBlocker(this->ui.flagAutomatic),
      QSignalBlocker(this->ui.flagHideBackpack),
      QSignalBlocker(this->ui.flagCantDrop),
      QSignalBlocker(this->ui.flagPlayable),
      QSignalBlocker(this->ui.flagNotUsedInNormalCombat),
      QSignalBlocker(this->ui.flagPlayerOnly),
      QSignalBlocker(this->ui.flagNonHostile),
      QSignalBlocker(this->ui.flagBound),
      QSignalBlocker(this->ui.critChanceMult),
      QSignalBlocker(this->ui.critDamage),
      QSignalBlocker(this->ui.critEffect),
      QSignalBlocker(this->ui.flagCritEffectOnlyOnDeath),
      QSignalBlocker(this->ui.embedded),
      QSignalBlocker(this->ui.embeddedAV),
      QSignalBlocker(this->ui.embeddedToNode),
      QSignalBlocker(this->ui.description),
      //
      // Art and Sound:
      //
      QSignalBlocker(this->ui.model),
      QSignalBlocker(this->ui.firstPersonModel),
      QSignalBlocker(this->ui.iconInventory),
      QSignalBlocker(this->ui.iconMessage),
      QSignalBlocker(this->ui.destructionData),
      QSignalBlocker(this->ui.impactDataSet),
      QSignalBlocker(this->ui.impactDataSetBlockBash),
      QSignalBlocker(this->ui.alternateBlockMaterial),
      QSignalBlocker(this->ui.weaponType),
      QSignalBlocker(this->ui.attackAnim),
      QSignalBlocker(this->ui.animAttackMult),
      QSignalBlocker(this->ui.flagNoFirstPersonISAnims),
      QSignalBlocker(this->ui.flagNoThirdPersonISAnims),
      QSignalBlocker(this->ui.scopeGroupbox),
      QSignalBlocker(this->ui.scopeTargetNIF),
      QSignalBlocker(this->ui.scopeEffect),
      //
      QSignalBlocker(this->ui.soundAttack),
      QSignalBlocker(this->ui.soundAttack2D),
      QSignalBlocker(this->ui.soundAttackLoop),
      QSignalBlocker(this->ui.soundAttackFail),
      QSignalBlocker(this->ui.soundIdle),
      QSignalBlocker(this->ui.soundEquip),
      QSignalBlocker(this->ui.soundUnequip),
      QSignalBlocker(this->ui.soundTake),
      QSignalBlocker(this->ui.soundDrop),
      QSignalBlocker(this->ui.detectionSoundLevel),
      QSignalBlocker(this->ui.rumbleStrengthL),
      QSignalBlocker(this->ui.rumbleStrengthR),
      QSignalBlocker(this->ui.rumbleDuration),
      QSignalBlocker(this->ui.flagAlternateRumble),
   };
   #pragma region Game Data
      this->ui.weight->setValue(working.item_data.weight);
      this->ui.damage->setValue(working.damage);
      this->ui.reach->setValue(working.reach);
      this->ui.projectileCount->setValue(working.projectile_count);
      this->ui.speed->setValue(working.speed);
      this->ui.ironsightFOV->setValue(working.ironsight_fov);
      this->ui.stagger->setValue(working.stagger);
      {
         auto* widget = this->ui.skill;
         int   i      = -1;
         if (working.skill.has_value())
            i = widget->findData((int)working.skill.value());
         if (i < 0)
            i = widget->findData(-1);
         widget->setCurrentIndex(i);
      }
      this->ui.equipType->setFormStub(working.equip_type.get_form_stub());
      _pull_enum(this->ui.resist, working.resist_av);
      _pull_enum(this->ui.onHitCalc, working.hit_gore_behavior);
      this->ui.baseVATSChance->setValue(working.base_vats_hit_chance);
      this->ui.combatAIRangeMin->setValue(working.ai_ranges.minimum);
      this->ui.combatAIRangeMax->setValue(working.ai_ranges.maximum);
      this->ui.flagFixedAIRange->setChecked(working.flags.fixed_ai_range);
      this->ui.flagBurstShot->setChecked(working.flags.burst_shot);
      this->ui.flagLongBursts->setChecked(working.flags.long_bursts);
      this->ui.keywords->pullStubs(working.keywords.forms);
      this->ui.flagNPCsUseAmmo->setChecked(working.flags.npcs_use_ammo);
      this->ui.flagNoJam->setChecked(working.flags.never_jams_after_reload);
      this->ui.flagIgnoresNormalResist->setChecked(working.flags.ignores_normal_weapon_resist);
      this->ui.flagMinorCrime->setChecked(working.flags.minor_crime);
      this->ui.flagAutomatic->setChecked(working.flags.automatic);
      this->ui.flagHideBackpack->setChecked(working.flags.hide_backpack);
      this->ui.flagCantDrop->setChecked(working.flags.cant_drop);
      this->ui.flagPlayable->setChecked(!working.flags.non_playable);
      this->ui.flagNotUsedInNormalCombat->setChecked(working.flags.not_used_in_normal_combat);
      this->ui.flagPlayerOnly->setChecked(working.flags.player_only);
      this->ui.flagNonHostile->setChecked(working.flags.non_hostile);
      this->ui.flagBound->setChecked(working.flags.bound_weapon);
      this->ui.critChanceMult->setValue(working.crit_data.chance_mult);
      this->ui.critDamage->setValue(working.crit_data.added_damage);
      this->ui.critEffect->setFormStub(working.crit_data.spell_to_apply.get_form_stub());
      this->ui.flagCritEffectOnlyOnDeath->setChecked(working.crit_data.apply_spell_only_on_target_death);
      this->ui.embedded->setChecked(working.flags.embedded);
      _pull_enum(this->ui.embeddedAV, working.embedded.actor_value);
      this->ui.embeddedToNode->setText(QString::fromStdString(working.embedded.node));
      this->ui.description->setPlainText(gls.convert_localized_string(working.description));
   #pragma endregion
   #pragma region Art and Sound
      this->ui.model->initializeFrom(working.model);
      this->ui.firstPersonModel->setFormStub(working.first_person_model.get_form_stub());
      this->ui.iconInventory->setValue(ui::types::game_file_path(QString::fromStdString(working.item_data.icons.inventory)));
      this->ui.iconMessage->setValue(ui::types::game_file_path(QString::fromStdString(working.item_data.icons.message)));
      this->ui.destructionData->initializeFrom(working.destruction_data);
      this->ui.impactDataSet->setFormStub(working.impact_data_set.get_form_stub());
      this->ui.impactDataSetBlockBash->setFormStub(working.block_bash.impact_data_set.get_form_stub());
      this->ui.alternateBlockMaterial->setFormStub(working.block_bash.alternate_material.get_form_stub());
      _pull_enum(this->ui.weaponType, working.type);
      _pull_enum(this->ui.attackAnim, working.animation.legacy_anim);
      this->ui.animAttackMult->setValue(working.animation.attack_mult);
      this->ui.animShotsPerSec->setValue(working.fire_rate);
      this->ui.flagNoFirstPersonISAnims->setChecked(working.flags.no_first_person_ironsight_anim);
      this->ui.flagNoThirdPersonISAnims->setChecked(working.flags.no_third_person_ironsight_anim);
      this->ui.scopeGroupbox->setChecked(working.flags.has_scope);
      this->ui.scopeTargetNIF->initializeFrom(working.scope_model);
      this->ui.scopeEffect->setFormStub(working.scope_shader.get_form_stub());
      //
      this->ui.soundAttack->setFormStub(working.sounds.attack.get_form_stub());
      this->ui.soundAttack2D->setFormStub(working.sounds.attack_2D.get_form_stub());
      this->ui.soundAttackLoop->setFormStub(working.sounds.attack_loop.get_form_stub());
      this->ui.soundAttackFail->setFormStub(working.sounds.attack_fail.get_form_stub());
      this->ui.soundIdle->setFormStub(working.sounds.idle.get_form_stub());
      this->ui.soundEquip->setFormStub(working.sounds.equip.get_form_stub());
      this->ui.soundUnequip->setFormStub(working.sounds.unequip.get_form_stub());
      this->ui.soundTake->setFormStub(working.item_data.sounds.take.get_form_stub());
      this->ui.soundDrop->setFormStub(working.item_data.sounds.drop.get_form_stub());
      _pull_enum(this->ui.detectionSoundLevel, working.loudness);
      this->ui.rumbleStrengthL->setValue(working.rumble.left_motor);
      this->ui.rumbleStrengthR->setValue(working.rumble.right_motor);
      this->ui.rumbleDuration->setValue(working.rumble.duration);
      this->ui.flagAlternateRumble->setChecked(working.flags.rumble_alternate);
   #pragma endregion
}
void FormDialogWeapon::_update_from_template_form() {
   if (!this->form)
      return;
   auto& working = *this->form;

   auto* direct_template_form = working.template_weapon.get_form_stub();
   {
      bool enable = direct_template_form == nullptr;
      this->ui.tabGameData->setEnabled(enable);
      this->ui.tabArt->setEnabled(enable);
      if (!direct_template_form)
         return;
   }
   //
   // Resolve transitive template relationships:
   //
   dovah::form_stub* effective_template_form = nullptr;
   {
      std::vector<dovah::form_stub*> seen;
      seen.push_back(&working.stub);
      [&seen, &effective_template_form](this auto&& recurse, dovah::form_stub* current) -> void {
         {
            auto it = std::find(seen.begin(), seen.end(), current);
            if (it != seen.end()) {
               //
               // Cyclical reference. Clear the template-form pointer and abort.
               //
               effective_template_form = nullptr;
               return;
            }
         }
         if (current->form_type != dovah::form_type::weapon) {
            //
            // Invalid relationship. The game would just clear the pointer, so we should use 
            // the last-seen template form and stop here.
            //
            return;
         }
         auto loaded = current->load().ptr_cast<loaded_form_type>();
         if (!loaded) {
            return;
         }
         effective_template_form = current;
         auto* stub = loaded->template_weapon.get_form_stub();
         if (stub)
            recurse(stub);
      }(direct_template_form);
   }
   if (!effective_template_form) {
      return;
   }
   //
   // Copy values and fields.
   //
   this->form->copy_data_from_template_weapon(*effective_template_form);
   this->_pull_templatable_data_to_ui();
}