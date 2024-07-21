#include "./actor_base.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

#include "editor/subsystems/form_info_cache/core.h"
#include "dovah/forms/Outfit.h"

#include "widgets/widget-data/DKFormPickerCustomFilter.h"

namespace {
   constexpr const bool preview_enabled = false;

   // We don't currently have a renderer suitable for actor previews, and we don't have 
   // code to generate and export a head NIF and tintmask, so for now, disable all face 
   // editing widgets.
   constexpr const bool allow_customizing_face = false;

   constexpr const bool filtered_dialogue_browser_implemented = false;
}

namespace impl {
   class DKFormPickerExcludeSingleFormFilter final : public DKFormPickerCustomFilter {
      public:
         using DKFormPickerCustomFilter::DKFormPickerCustomFilter;
         
         virtual bool form_matches(const dovah::form_stub& stub) const noexcept override {
            return &stub != this->_exclude;
         };

         void set_exclusion(dovah::form_stub* exclude) {
            if (exclude == this->_exclude)
               return;
            auto* prior = this->_exclude;
            this->_exclude = exclude;
            if (prior)
               this->_refilter_form(*prior);
            if (exclude)
               this->_refilter_form(*exclude);
         }

      protected:
         dovah::form_stub* _exclude = nullptr;
   };
   class VoicetypePickerFilter final : public DKFormPickerCustomFilter {
      protected:
         using fic_type = dovahkit::subsystems::form_info_cache::core;

      public:
         VoicetypePickerFilter(QObject* parent) : DKFormPickerCustomFilter(parent) {
            auto& fic = fic_type::get();
            QObject::connect(&fic, &fic_type::cachedVoicetypeChanged, this, [this](dovah::form_stub* stub) {
               if (stub)
                  this->_refilter_form(*stub);
            });
         }

         virtual bool form_matches(const dovah::form_stub& stub) const noexcept override {
            auto& fic  = fic_type::get();
            auto* info = fic.get_voicetype_info(stub);
            if (!info)
               return true;
            return info->female == this->_female;
         };

         void set_female(bool female) {
            if (female == this->_female)
               return;
            this->_female = female;
            this->_refilter_all_forms();
         }

      protected:
         bool _female = false;
   };
}

FormDialogActorBase::FormDialogActorBase(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.previewOptionsContainer->setVisible(preview_enabled);
   this->ui.buttonShowFilteredDialogue->setVisible(filtered_dialogue_browser_implemented);

   this->_filters.exclude_self = new impl::DKFormPickerExcludeSingleFormFilter(this);

   #pragma region Not in any tab
      this->ui.templateActor->setAllowedFormTypes({ dovah::form_type::actor_base, dovah::form_type::leveled_character });
      QObject::connect(this->ui.templateActor, &DKFormPicker::formChanged, this, [this](dovah::form_stub* target) {
         this->ui.templateFlags->setEnabled(target != nullptr);
      });
   #pragma endregion
   
   #pragma region Traits tab
      this->ui.race->setAllowedFormType(dovah::form_type::race);
      this->ui.skin->setAllowedFormType(dovah::form_type::armor);
      ui::set_unsigned_range<float>(this->ui.height);
      this->ui.bodyWeight->setRange(0, 1);
      this->ui.farawaySkin->setAllowedFormType(dovah::form_type::armor);
      ui::set_unsigned_range<float>(this->ui.farawayDistance);
      this->ui.voicetype->setAllowedFormType(dovah::form_type::voicetype);
      {
         auto* filter = this->_filters.voicetype = new impl::VoicetypePickerFilter(this);
         this->ui.voicetype->setCustomFilter(filter); // limit voicetypes by sex, as the game itself does
      }
      this->ui.weaponList->setAllowedFormType(dovah::form_type::formlist);
      this->ui.deathItem->setAllowedFormType(dovah::form_type::leveled_item);
   #pragma endregion
   #pragma region Stats tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Factions tab
      this->ui.currentFactionForm->setAllowedFormType(dovah::form_type::faction);
      ui::set_range<int8_t>(this->ui.currentFactionRank);
   #pragma endregion
   #pragma region Relationships tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Keywords tab
      this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   #pragma endregion
   #pragma region AI Data tab
      static_assert(false, "TODO");
      ui::set_range<uint8_t>(this->ui.aiEnergy);
      this->ui.giftFilter->setAllowedFormType(dovah::form_type::formlist);

      static_assert(false, "TODO");
      ui::set_range<uint16_t>(this->ui.aggroRadiusWarn);
      ui::set_range<uint16_t>(this->ui.aggroRadiusWarnAttack);
      ui::set_range<uint16_t>(this->ui.aggroRadiusAttack);
   #pragma endregion
   #pragma region AI Packages tab
      static_assert(false, "TODO");

      this->ui.packageListDefault->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListSpectator->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListObserveCorpse->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListGuardWarn->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListCombat->setAllowedFormType(dovah::form_type::formlist);
   #pragma endregion
   #pragma region Inventory tab
      this->ui.outfitDefault->setAllowedFormType(dovah::form_type::outfit);
      this->ui.outfitSleep->setAllowedFormType(dovah::form_type::outfit);
      static_assert(false, "TODO: Make the DKFormListPane here read-only.");
      ui::set_range<uint8_t>(this->ui.gearedUpWeapons);

      QObject::connect(this->ui.outfitDefault, &DKFormPicker::formChanged, this, &FormDialogActorBase::_updateOutfitContentsView);
   #pragma endregion
   #pragma region Magic and Perks tab
      this->ui.spells->setAllowedFormTypes({
         dovah::form_type::spell,
         dovah::form_type::leveled_spell,
         dovah::form_type::shout,
      });
      this->ui.perks->setAllowedFormTypes({ dovah::form_type::perk });
   #pragma endregion
   #pragma region Sounds tab
      this->ui.inheritSoundsFrom->setAllowedFormType(dovah::form_type::actor_base);
      this->ui.inheritSoundsFrom->setCustomFilter(this->_filters.exclude_self); // don't let an actor inherit sounds from themselves

      this->ui.currentCreaSoundForm->setAllowedFormType(dovah::form_type::sound_descriptor);
   #pragma endregion
   #pragma region Attack Data tab
      this->ui.attackDataRace->setAllowedFormType(dovah::form_type::race);
      //
      ui::set_unsigned_range<float>(this->ui.attackDataSelDamageMult);
      ui::set_range<float>(this->ui.attackDataSelChance);
      ui::set_unsigned_range<float>(this->ui.attackDataSelStagger);
      ui::set_unsigned_range<float>(this->ui.attackDataSelRecoveryTime);
      ui::set_unsigned_range<float>(this->ui.attackDataSelStaminaMult);
      this->ui.attackDataSelAngleAttack->setRange(-360, 360);
      this->ui.attackDataSelAngleStrike->setRange(-360, 360);
      ui::set_unsigned_range<float>(this->ui.attackDataSelKnockdown);
      this->ui.attackDataSelSpell->setAllowedFormType(dovah::form_type::spell);
      this->ui.attackDataSelKeyword->setAllowedFormType(dovah::form_type::keyword);
   #pragma endregion
   #pragma region Face Parts tab
      if constexpr (!allow_customizing_face) {
         this->ui.tabFaceParts->setEnabled(false);
      }
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Morphs tab
      if constexpr (!allow_customizing_face) {
         this->ui.tabFaceMorphs->setEnabled(false);
      }
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Anim Preview tab
      if constexpr (preview_enabled) {
         this->ui.tabbox->setTabVisible(
            this->ui.tabbox->indexOf(this->ui.tabFaceAnimPreview),
            false
         );
      }
      static_assert(!preview_enabled, "If you enable the preview, then implement the facial animations preview!");
   #pragma endregion

   this->load(); // this creates the working copy.
}
void FormDialogActorBase::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(this->formStub());

   #pragma region Not in a tab
      ui::bind(this->ui.editorID, this->editor_id());
      this->ui.name->setText(editor.convert_localized_string(working.name));
      this->ui.shortName->setText(editor.convert_localized_string(working.short_name));

      ui::bind(this->ui.flagPreset, working.actor_flags, loaded_form_type::actor_flag::is_chargen_preset);
      ui::bind(this->ui.flagEssential, working.actor_flags, loaded_form_type::actor_flag::essential);
      ui::bind(this->ui.flagProtected, working.actor_flags, loaded_form_type::actor_flag::is_protected);
      ui::bind(this->ui.flagRespawn, working.actor_flags, loaded_form_type::actor_flag::respawn);
      ui::bind(this->ui.flagUnique, working.actor_flags, loaded_form_type::actor_flag::unique);
      ui::bind(this->ui.flagDoesntAffectStealthMeter, working.actor_flags, loaded_form_type::actor_flag::doesnt_affect_stealth_meter);
      //
      ui::bind(this->ui.flagSummonable, working.actor_flags, loaded_form_type::actor_flag::summonable);
      ui::bind(this->ui.flagGhost, working.actor_flags, loaded_form_type::actor_flag::ghost);
      ui::bind(this->ui.flagInvulnerable, working.actor_flags, loaded_form_type::actor_flag::invulnerable);
      ui::bind(this->ui.flagDoesntBleed, working.actor_flags, loaded_form_type::actor_flag::doesnt_bleed);
      ui::bind(this->ui.flagSimple, working.actor_flags, loaded_form_type::actor_flag::simple_actor);

      this->ui.destructionData->initializeFrom(working.destruction_data);
      this->ui.scriptListPane->setFormWorkingCopy(&working);

      #pragma region Template data
         ui::bind(this->ui.templateActor, working.template_data.actor, working);
         //
         ui::bind(this->ui.templateUseAIData, working.template_data.flags, loaded_form_type::template_flag::use_ai_data);
         ui::bind(this->ui.templateUseAIPackages, working.template_data.flags, loaded_form_type::template_flag::use_ai_packages);
         ui::bind(this->ui.templateUseAttackData, working.template_data.flags, loaded_form_type::template_flag::use_attack_data);
         ui::bind(this->ui.templateUseBaseData, working.template_data.flags, loaded_form_type::template_flag::use_base_data);
         ui::bind(this->ui.templateUseDefaultPackages, working.template_data.flags, loaded_form_type::template_flag::use_package_overrides);
         ui::bind(this->ui.templateUseFactions, working.template_data.flags, loaded_form_type::template_flag::use_factions);
         ui::bind(this->ui.templateUseInventory, working.template_data.flags, loaded_form_type::template_flag::use_inventory);
         ui::bind(this->ui.templateUseKeywords, working.template_data.flags, loaded_form_type::template_flag::use_keywords);
         ui::bind(this->ui.templateUseScripts, working.template_data.flags, loaded_form_type::template_flag::use_scripts);
         ui::bind(this->ui.templateUseSpellList, working.template_data.flags, loaded_form_type::template_flag::use_spells);
      #pragma endregion
   #pragma endregion

   #pragma region Traits tab
      ui::bind(this->ui.race, working.race,       working);
      ui::bind(this->ui.skin, working.worn_armor, working);
      QObject::connect(this->ui.sex, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         bool female = this->ui.sex->currentIndex() == 1;
         if (female)
            this->form->actor_flags |= loaded_form_type::actor_flag::female;
         else
            this->form->actor_flags &= ~loaded_form_type::actor_flag::female;

         this->_filters.voicetype->set_female(female);
      });
      ui::bind(this->ui.flagOppositeGenderAnims, working.template_data.flags, loaded_form_type::actor_flag::opposite_gender_animations);
      ui::bind(this->ui.height,     working.height);
      ui::bind(this->ui.bodyWeight, working.weight);
      ui::bind(this->ui.farawaySkin,     working.far_away.model, working);
      ui::bind(this->ui.farawayDistance, working.far_away.distance);
      ui::bind(this->ui.voicetype, working.voicetype, working);
      static_assert(false, "TODO: Weapon List");
      ui::bind(this->ui.dispositionBase, working.stats.disposition);
      ui::bind(this->ui.deathItem, working.death_item, working);
   #pragma endregion
   #pragma region Stats tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Factions tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Relationships tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Keywords tab
      for (auto& ref : working.keywords.forms) {
         this->ui.keywords->addStub(ref.get_form_stub());
      }
   #pragma endregion
   #pragma region AI Data tab
      static_assert(false, "TODO");
      ui::bind(this->ui.giftFilter, working.gift_filter, working);

      static_assert(false, "TODO");
      ui::bind(this->ui.aggroRadiusWarn,       working.ai.aggro.warn);
      ui::bind(this->ui.aggroRadiusWarnAttack, working.ai.aggro.warn_attack);
      ui::bind(this->ui.aggroRadiusAttack,     working.ai.aggro.attack);
   #pragma endregion
   #pragma region AI Packages tab
      static_assert(false, "TODO");

      ui::bind(this->ui.packageListDefault,       working.ai.default_package_list, working);
      ui::bind(this->ui.packageListSpectator,     working.ai.package_override_lists.spectator, working);
      ui::bind(this->ui.packageListObserveCorpse, working.ai.package_override_lists.observe_corpse, working);
      ui::bind(this->ui.packageListGuardWarn,     working.ai.package_override_lists.guard_warn, working);
      ui::bind(this->ui.packageListCombat,        working.ai.package_override_lists.combat, working);
   #pragma endregion
   #pragma region Inventory tab
      QObject::connect(this->ui.outfitDefault, &DKFormPicker::formChanged, this, &FormDialogActorBase::_updateOutfitContentsView);
      ui::bind(this->ui.outfitDefault, working.outfits.normal,   working);
      ui::bind(this->ui.outfitSleep,   working.outfits.sleeping, working);
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Magic and Perks tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Sounds tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Attack Data tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Parts tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Morphs tab
      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Anim Preview tab
      static_assert(false, "TODO");
   #pragma endregion

   static_assert(false, "TODO");
}
void FormDialogActorBase::_save_impl() {
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

   static_assert(false, "TODO");

   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.scriptListPane->commit();
}

void FormDialogActorBase::updatePreview() {
   static_assert(!preview_enabled, "The 3D preview is not yet implemented. Don't enable it until it's implemented!");
}
void FormDialogActorBase::_updateOutfitContentsView() {
   auto* widget = this->ui.outfitItemsPreview;
   widget->clear();

   auto* outfit = this->form->outfits.normal.get_form_stub();
   if (!outfit)
      return;
   auto  loaded = outfit->load().ptr_cast<dovah::loaded_forms::Outfit>();
   if (!loaded)
      return;

   for (auto& entry : loaded->contents) {
      widget->addStub(entry.get_form_stub());
   }
}
void FormDialogActorBase::_updateFromTemplate() {
   using template_flag = loaded_form_type::template_flag;

   auto* base  = this->form->template_data.actor.get_form_stub();
   auto  flags = this->form->template_data.flags;
   if (!base) {
      flags = 0;
   }
   this->ui.tabTraits->setEnabled(!(flags& template_flag::use_traits));
   this->ui.tabStats->setEnabled(!(flags & template_flag::use_stats));
   this->ui.tabFactions->setEnabled(!(flags & template_flag::use_factions));
   this->ui.tabSpells->setEnabled(!(flags & template_flag::use_spells));
   this->ui.tabAIData->setEnabled(!(flags & template_flag::use_ai_data));
   this->ui.packageTable->setEnabled(!(flags & template_flag::use_ai_packages));
   static_assert(false, "TODO: template_flag::use_animations");
   this->ui.tabInventory->setEnabled(!(flags & template_flag::use_inventory));
   this->ui.tabAttackData->setEnabled(!(flags & template_flag::use_attack_data));
   this->ui.tabKeywords->setEnabled(!(flags & template_flag::use_keywords));
   this->ui.defaultPackagesGroupbox->setEnabled(!(flags & template_flag::use_package_overrides));
   {  // Base data
      bool enable = !(flags & template_flag::use_base_data);
      this->ui.name->setEnabled(enable);
      this->ui.shortName->setEnabled(enable);
      this->ui.flagEssential->setEnabled(enable);
      this->ui.flagProtected->setEnabled(enable);
      this->ui.flagRespawn->setEnabled(enable);
      this->ui.flagSummonable->setEnabled(enable);
      this->ui.flagSimple->setEnabled(enable);
      this->ui.flagDoesntAffectStealthMeter->setEnabled(enable);
   }

   if (!base) {
      return;
   }
   auto base_loaded = base->load().ptr_cast<loaded_form_type>();
   if (!base_loaded)
      return;

   static_assert(false, "TODO: Based on flags, set the state of UI values to match the template actor.");
   if (flags & template_flag::use_keywords) {
      this->ui.keywords->clear();
      for (auto& ref : base_loaded->keywords.forms) {
         this->ui.keywords->addStub(ref.get_form_stub());
      }
   }
   if (flags & template_flag::use_package_overrides) {
      this->ui.packageListDefault->setFormStub(base_loaded->ai.default_package_list.get_form_stub());
      this->ui.packageListSpectator->setFormStub(base_loaded->ai.package_override_lists.spectator.get_form_stub());
      this->ui.packageListObserveCorpse->setFormStub(base_loaded->ai.package_override_lists.observe_corpse.get_form_stub());
      this->ui.packageListGuardWarn->setFormStub(base_loaded->ai.package_override_lists.guard_warn.get_form_stub());
      this->ui.packageListCombat->setFormStub(base_loaded->ai.package_override_lists.combat.get_form_stub());
   }
}