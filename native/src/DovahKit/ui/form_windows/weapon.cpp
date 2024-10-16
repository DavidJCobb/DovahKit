#include "./weapon.h"
#include <limits>
#include "dovah/core.h"
#include "dovah/data/actor_values.h"
#include "dovah/data/skills.h"
#include "editor/helpers/skill_name_to_string.h"
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

      {
         auto* widget = this->ui.skill;
         widget->clear();
         for (size_t i = 0; i < dovah::skill_count; ++i) {
            widget->addItem(editor_helpers::skill_name_to_string((dovah::skill)i), (int)i);
         }
      }
      this->ui.equipType->setAllowedFormType(dovah::form_type::equip_slot);
      {
         auto* widget = this->ui.resist;
         widget->clear();
         widget->addItem(tr("None", "damage resistance AV"), -1);
         for (auto& info : dovah::all_actor_value_info) {
            if (info.type != dovah::actor_value_type::resistance)
               continue;
            widget->addItem(QString::fromLatin1(QByteArray(info.name.data(), info.name.size())), (int)info.index);
         }
      }
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
         {
            auto* widget = this->ui.embeddedAV;
            widget->clear();
            widget->addItem(tr("None", "embedded node AV"), -1);
            for (auto& info : dovah::all_actor_value_info) {
               if (info.type != dovah::actor_value_type::limb_condition)
                  continue;
               widget->addItem(QString::fromLatin1(QByteArray(info.name.data(), info.name.size())), (int)info.index);
            }
         }
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
      #pragma endregion

      #pragma region Rumble
         ui::set_unsigned_range<float>(this->ui.rumbleStrengthL);
         ui::set_unsigned_range<float>(this->ui.rumbleStrengthR);
         ui::set_unsigned_range<float>(this->ui.rumbleDuration);
         {
            auto* widget = this->ui.rumblePattern;
            widget->clear();
            widget->addItem("Constant", (int)loaded_form_type::rumble_pattern::constant);
            widget->addItem("Periodic Square", (int)loaded_form_type::rumble_pattern::periodic_square);
            widget->addItem("Periodic Triangle", (int)loaded_form_type::rumble_pattern::periodic_triangle);
            widget->addItem("Periodic Sawtooth", (int)loaded_form_type::rumble_pattern::periodic_sawtooth);
         }
      #pragma endregion
   #pragma endregion

   this->ui.tabbox->setCurrentIndex(0);

   this->load(); // this creates the working copy.
}
void FormDialogWeapon::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   #pragma region Base data
      ui::bind(this->ui.editorID, this->editor_id());
      this->ui.name->setText(editor.convert_localized_string(working.name));
      ui::bind(this->ui.enchantmentForm, working.enchantable.effect, working);
      ui::bind(this->ui.enchantmentCharge, working.enchantable.charge);
      ui::bind(this->ui.value, working.item_data.value);
      ui::bind(this->ui.templateForm, working.template_weapon, working);

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
      ui::bind(this->ui.skill, working.skill);
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
      this->ui.description->setPlainText(editor.convert_localized_string(working.description));
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
      #pragma endregion

      ui::bind(this->ui.rumbleStrengthL, working.rumble.left_motor);
      ui::bind(this->ui.rumbleStrengthR, working.rumble.right_motor);
      ui::bind(this->ui.rumbleDuration, working.rumble.duration);
      ui::bind(this->ui.rumblePattern, working.rumble.pattern);
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
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.name->text());
   editor.assign_localized_string(working.description, this->ui.description->toPlainText());
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.scriptListPane->commit();
}