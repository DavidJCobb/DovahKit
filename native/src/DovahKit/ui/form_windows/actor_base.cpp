#include "./actor_base.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"

#include "editor/subsystems/form_info_cache/core.h"
#include "editor/open_window_for_form.h"
#include "dovah/forms/Outfit.h"

#include "widgets/widget-data/DKFormPickerCustomFilter.h"

#include "./actor_base/ActorBaseFactionsModel.h"
#include "./actor_base/ActorBaseRelationshipsModel.h"
#include "./actor_base/ActorBaseSkillsModel.h"
#include "./actor_base/FaceComplexionPickerFilter.h"
#include "./actor_base/FaceHairColorPickerFilter.h"
#include "./actor_base/FaceTintColorPickerFilter.h"

namespace {
   constexpr const bool preview_enabled = false;

   // We don't currently have a renderer suitable for actor previews, and we don't have 
   // code to generate and export a head NIF and tintmask, so for now, disable all face 
   // editing widgets.
   constexpr const bool allow_customizing_face = false;

   constexpr const bool filtered_dialogue_browser_implemented = false;
}

namespace impl {
   class CrimeFactionPickerFilter final : public DKFormPickerCustomFilter {
      protected:
         using fic_type = dovahkit::subsystems::form_info_cache::core;

      public:
         CrimeFactionPickerFilter(QObject* parent) : DKFormPickerCustomFilter(parent) {
            auto& fic = fic_type::get();
            QObject::connect(&fic, &fic_type::cachedFactionChanged, this, [this](dovah::form_stub* stub) {
               if (stub)
                  this->_refilter_form(*stub);
            });
         }
         
         virtual bool form_matches(const dovah::form_stub& stub) const noexcept override {
            if (!this->_model)
               return true;

            if (!this->_model->containsFaction(&stub))
               return false;
            
            auto& fic  = fic_type::get();
            auto* info = fic.get_faction_info(stub);
            if (!info)
               return false;

            return info->tracks_crime;
         };

         void setModel(ActorBaseFactionsModel* model) {
            if (this->_model == model)
               return;

            if (this->_model) {
               QObject::disconnect(this->_model, nullptr, this, nullptr);
            }
            this->_model = model;
            if (model) {
               QObject::connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& qmi) {
                  if (qmi.column() != ActorBaseFactionsModel::Column::Faction)
                     return;
                  this->_refilter_all_forms();
               });
               QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
                  this->_refilter_all_forms();
               });
               QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() {
                  this->_refilter_all_forms();
               });
               QObject::connect(model, &QAbstractItemModel::modelReset, this, [this]() {
                  this->_refilter_all_forms();
               });
            }
            this->_refilter_all_forms();
         }

      protected:
         ActorBaseFactionsModel* _model = nullptr;
   };
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

      ui::set_unsigned_range<decltype(decltype(loaded_form_type::stats)::disposition)>(this->ui.dispositionBase);
   #pragma endregion
   #pragma region Stats tab
      {
         using stats_struct = decltype(loaded_form_type::stats);

         ui::set_range<decltype(stats_struct::level)>(this->ui.level);
         ui::set_range<decltype(stats_struct::calc_min_level)>(this->ui.levelCalcMin);
         ui::set_range<decltype(stats_struct::calc_max_level)>(this->ui.levelCalcMax);
         QObject::connect(this->ui.flagPCLevelMult, &QCheckBox::toggled, this, [this](bool checked) {
            if (checked)
               this->ui.flagAutoCalcStats->setChecked(true);
         });

         this->ui.speedPercentage->setRange(0, 32767);
         this->ui.bleedoutOverrideThreshold->setRange(0, 32767);

         this->ui.statsClass->setAllowedFormType(dovah::form_type::combat_class);
         static_assert(false, "TODO: Attributes");
         ui::set_range<decltype(decltype(stats_struct::base)::health)>(this->ui.statsHealthBase);
         ui::set_range<decltype(decltype(stats_struct::base)::magicka)>(this->ui.statsMagickaBase);
         ui::set_range<decltype(decltype(stats_struct::base)::stamina)>(this->ui.statsStaminaBase);
         ui::set_range<decltype(decltype(stats_struct::offsets)::health)>(this->ui.statsHealthOffset);
         ui::set_range<decltype(decltype(stats_struct::offsets)::magicka)>(this->ui.statsMagickaOffset);
         ui::set_range<decltype(decltype(stats_struct::offsets)::stamina)>(this->ui.statsStaminaOffset);
         this->ui.statsHealthBase->setReadOnly(true);
         this->ui.statsMagickaBase->setReadOnly(true);
         this->ui.statsStaminaBase->setReadOnly(true);
         this->ui.statsHealthCalcFinal->setReadOnly(true);
         this->ui.statsMagickaCalcFinal->setReadOnly(true);
         this->ui.statsStaminaCalcFinal->setReadOnly(true);
         {
            auto* widget = this->ui.statsTable;
            auto* model  = this->_models.skills = new ActorBaseSkillsModel(this);
            widget->setModel(model);

            ui::set_range<uint8_t>(this->ui.currentSkillOffset);

            {  // Level
               QObject::connect(this->ui.flagPCLevelMult, &QCheckBox::toggled, this, [this](bool checked) {
                  if (checked)
                     this->_models.skills->setLevel(1);
                  else
                     this->_models.skills->setLevel(this->ui.level->value());
               });
               QObject::connect(this->ui.level, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
                  if (!this->ui.flagPCLevelMult->isChecked())
                     this->_models.skills->setLevel(v);
               });
            }
            QObject::connect(this->ui.flagAutoCalcStats, &QCheckBox::toggled, this, [this](bool checked) {
               this->ui.skillEditLayout->setVisible(!checked);
               this->_models.skills->setOffsetsUsed(!checked);
            });
            QObject::connect(this->ui.race, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
               this->_models.skills->setRace(stub);
            });
            QObject::connect(this->ui.statsClass, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
               this->_models.skills->setClass(stub);
            });

            auto* sel_model = widget->selectionModel();
            QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, sel_model, model]() {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty()) {
                  this->ui.skillEditLayout->setEnabled(false);
                  return;
               }
               this->ui.skillEditLayout->setEnabled(true);

               const auto blocker = QSignalBlocker(this->ui.currentSkillOffset);
               this->ui.currentSkillOffset->setValue(model->offsetOf((dovah::skill)rows[0].row()));
            });
            QObject::connect(this->ui.currentSkillOffset, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, sel_model, model](int v) {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty()) {
                  this->ui.skillEditLayout->setEnabled(false);
                  return;
               }
               model->setSkillOffset((dovah::skill)rows[0].row(), v);
            });

            this->ui.skillEditLayout->setEnabled(false);
         }
      }
   #pragma endregion
   #pragma region Factions tab
      this->_filters.crime_faction = new impl::CrimeFactionPickerFilter(this);
      {
         auto* widget = this->ui.factionsTable;
         auto* model  = this->_models.factions = new ActorBaseFactionsModel(this);
         widget->setModel(model);
         this->_filters.crime_faction->setModel(model);
      }
      this->ui.currentFactionForm->setAllowedFormType(dovah::form_type::faction);
      this->ui.currentFactionForm->setCustomFilter(this->_filters.crime_faction);
      ui::set_range<int8_t>(this->ui.currentFactionRank);

      QObject::connect(this->ui.currentFactionForm, &DKFormPicker::formChanged, this, &FormDialogActorBase::_push_faction_from_ui);
      QObject::connect(this->ui.currentFactionRank, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormDialogActorBase::_push_faction_from_ui);
   #pragma endregion
   #pragma region Relationships tab
      {
         auto* widget = this->ui.relationshipsTable;
         auto* model  = this->_models.relationships = new ActorBaseRelationshipsModel(this);
         widget->setModel(model);

         {  // Context menu
            auto& menu = this->_context_menus.relationships;

            auto* action_edit = new QAction(tr("Edit..."), this);
            QObject::connect(action_edit, &QAction::triggered, this, [this]() {
               auto* sel_model = this->ui.relationshipsTable->selectionModel();
               size_t row;
               {
                  auto rows = sel_model->selectedRows();
                  if (rows.isEmpty())
                     return;
                  row = rows[0].row();
               }
               auto* relationship = this->_models.relationships->relationshipAt(row);
               if (!relationship)
                  return;
               open_edit_dialog_for_form(*relationship);
            });
            menu.addAction(action_edit);

            widget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
            QObject::connect(widget, &QWidget::customContextMenuRequested, this, [this, widget, action_edit, &menu](const QPoint& pos) {
               auto& menu = this->_context_menus.relationships;

               bool any_selection = false;
               if (auto* sel_model = this->ui.relationshipsTable->selectionModel()) {
                  auto rows = sel_model->selectedRows();
                  if (!rows.isEmpty())
                     any_selection = true;
               }

               action_edit->setEnabled(any_selection);

               menu.exec(widget->mapToGlobal(pos));
            });
         }
      }
   #pragma endregion
   #pragma region Keywords tab
      this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   #pragma endregion
   #pragma region AI Data tab
      ui::item_indices_to_data(this->ui.mood);
      ui::set_range<uint8_t>(this->ui.aiEnergy);
      this->ui.giftFilter->setAllowedFormType(dovah::form_type::formlist);

      ui::item_indices_to_data(this->ui.aggression);
      ui::item_indices_to_data(this->ui.confidence);
      ui::item_indices_to_data(this->ui.assistance);
      ui::item_indices_to_data(this->ui.morality);
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
      this->ui.outfitItemsPreview->setReadOnly(true);
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
      ui::item_indices_to_data(this->ui.soundLevel);
      this->ui.inheritSoundsFrom->setAllowedFormType(dovah::form_type::actor_base);
      this->ui.inheritSoundsFrom->setAllowNone(true);
      this->ui.inheritSoundsFrom->setCustomFilter(this->_filters.exclude_self); // don't let an actor inherit sounds from themselves

      static_assert(false, "TODO: list model");

      ui::item_indices_to_data(this->ui.currentCreaSoundType);
      this->ui.currentCreaSoundChance->setRange(0, 100);
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

      this->_filters.face.complexion = new impl::FaceComplexionPickerFilter(this);
      this->ui.faceComplexion->setCustomFilter(this->_filters.face.complexion);

      this->_filters.face.hair_color = new impl::FaceHairColorPickerFilter(this);
      this->ui.hairColor->setCustomFilter(this->_filters.face.hair_color);

      this->_filters.face.tint_color = new impl::FaceTintColorPickerFilter(this);
      this->ui.faceTintColorPreset->setCustomFilter(this->_filters.face.tint_color);

      QObject::connect(this->ui.race, &DKFormPicker::formChanged, this, [this](dovah::form_stub* race) {
         this->_filters.face.complexion->setRequiredRace(race);
         this->_filters.face.hair_color->setRequiredRace(race);
         this->_filters.face.tint_color->setRequiredRace(race);
      });
      QObject::connect(this->ui.sex, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
         auto sex = (i == 1) ? dovah::sex::female : dovah::sex::male;

         this->_filters.face.complexion->setRequiredSex(sex);
         this->_filters.face.hair_color->setRequiredSex(sex);
         this->_filters.face.tint_color->setRequiredSex(sex);
      });

      {
         auto* view      = this->ui.faceTintLayerTable;
         auto* sel_model = view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this]() {
            uint16_t tint_index = static_assert(false, "TODO");

            this->_filters.face.tint_color->setFaceTintIndex(tint_index);
         });
      }

      static_assert(false, "TODO");
   #pragma endregion
   #pragma region Face Morphs tab
      if constexpr (!allow_customizing_face) {
         this->ui.tabFaceMorphs->setEnabled(false);
      }
      //
      // We set the morph information when we bind the widgets to the form, not here.
      // Easier that way just because there's no floating-point QSlider; keep all the 
      // values we use as hacks to adjust it all in one place.
      //
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
         this->_models.relationships->setFocusActor(this->formStub(), female ? dovah::sex::female : dovah::sex::male);
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
      ui::bind(this->ui.level,        working.stats.level);
      ui::bind(this->ui.levelCalcMin, working.stats.calc_min_level);
      ui::bind(this->ui.levelCalcMax, working.stats.calc_max_level);
      ui::bind(this->ui.flagPCLevelMult, working.actor_flags, loaded_form_type::actor_flag::pc_level_mult);

      ui::bind(this->ui.speedPercentage, working.stats.speed_mult);
      ui::bind(this->ui.flagBleedoutOverride, working.actor_flags, loaded_form_type::actor_flag::bleedout_override);
      ui::bind(this->ui.bleedoutOverrideThreshold, working.stats.bleedout_threshold);

      ui::bind(this->ui.statsClass, working.stats.combat_class, working);
      ui::bind(this->ui.flagAutoCalcStats, working.actor_flags, loaded_form_type::actor_flag::auto_calc_stats);
      static_assert(false, "TODO: Attributes");

      {
         for (size_t i = 0; i < dovah::skill_count; ++i)
            this->_models.skills->setSkillOffset((dovah::skill)i, working.stats.offsets.skills.list[i]);
      }
   #pragma endregion
   #pragma region Factions tab
      {
         auto* model = this->_models.factions;
         model->clear();

         std::vector<ActorBaseFactionsModelNode> nodes;
         for (const auto& entry : working.faction_memberships) {
            auto& dst = nodes.emplace_back();
            dst.faction = entry.faction.get_form_stub();
            dst.rank    = entry.rank;
         }
         model->overwriteAllItems(nodes);
      }
      this->_pull_faction_to_ui();
   #pragma endregion
   #pragma region Relationships tab
      this->_models.relationships->setFocusActor(
         this->formStub(),
         (working.actor_flags & loaded_form_type::actor_flag::female) ? dovah::sex::female : dovah::sex::male
      );
   #pragma endregion
   #pragma region Keywords tab
      for (auto& ref : working.keywords.forms) {
         this->ui.keywords->addStub(ref.get_form_stub());
      }
   #pragma endregion
   #pragma region AI Data tab
      ui::bind(this->ui.mood, working.ai.mood);
      ui::bind(this->ui.aiEnergy, working.ai.energy_level);
      ui::bind(this->ui.giftFilter, working.gift_filter, working);

      ui::bind(this->ui.aggression, working.ai.aggression);
      ui::bind(this->ui.confidence, working.ai.confidence);
      ui::bind(this->ui.assistance, working.ai.assistance);
      ui::bind(this->ui.morality, working.ai.morality);
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
      ui::bind(this->ui.gearedUpWeapons, working.geared_up_weapons);

      this->ui.inventory->initializeFrom(working.inventory);
   #pragma endregion
   #pragma region Magic and Perks tab
      this->ui.spells->pullStubs(working.spells.forms);
      this->ui.perks->pullStubs(working.perks);
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
      {
         auto _handle = [](QSlider* widget, float& value) {
            widget->setRange(-1000, 1000);
            widget->setTickInterval(200);
            ui::bind(widget, value, 1000);
         };

         _handle(this->ui.faceMorphBrowDepth,  working.face.morphs.brows.depth);
         _handle(this->ui.faceMorphBrowHeight, working.face.morphs.brows.height);
         _handle(this->ui.faceMorphBrowWidth,  working.face.morphs.brows.width);

         static_assert(false, "TODO: Mouth Type (morph index from the actor's race)");
         _handle(this->ui.faceMorphMouthHeight, working.face.morphs.mouth.height);
         _handle(this->ui.faceMorphMouthDepth,  working.face.morphs.mouth.depth);

         _handle(this->ui.faceMorphChinDepth,  working.face.morphs.chin.depth);
         _handle(this->ui.faceMorphChinLength, working.face.morphs.chin.height);
         _handle(this->ui.faceMorphChinWidth,  working.face.morphs.chin.width);

         _handle(this->ui.faceMorphJawDepth,  working.face.morphs.jaw.depth);
         _handle(this->ui.faceMorphJawHeight, working.face.morphs.jaw.height);
         _handle(this->ui.faceMorphJawWidth,  working.face.morphs.jaw.width);

         _handle(this->ui.faceMorphCheekbonesHeight, working.face.morphs.cheeks.height);
         _handle(this->ui.faceMorphCheekbonesWidth,  working.face.morphs.cheeks.width);

         static_assert(false, "TODO: Eyes Type (morph index from the actor's race)");
         _handle(this->ui.faceMorphEyesDepth,  working.face.morphs.eyes.depth);
         _handle(this->ui.faceMorphEyesHeight, working.face.morphs.eyes.height);
         _handle(this->ui.faceMorphEyesWidth,  working.face.morphs.eyes.width);

         static_assert(false, "TODO: Nose Type (morph index from the actor's race)");
         _handle(this->ui.faceMorphNoseHeight, working.face.morphs.nose.height);
         _handle(this->ui.faceMorphNoseLength, working.face.morphs.nose.length);
      }
   #pragma endregion
   #pragma region Face Anim Preview tab
      static_assert(!preview_enabled, "TODO");
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
   this->ui.destructionData->commitTo(working.destruction_data, working);

   static_assert(false, "TODO");

   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.spells->commitStubs(working.spells.forms, working);
   this->ui.perks->commitStubs(working.perks, working);
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

void FormDialogActorBase::_pull_faction_to_ui() {
   auto* widget    = this->ui.factionsTable;
   auto* model     = this->_models.factions;
   auto* sel_model = widget->selectionModel();

   const ActorBaseFactionsModelNode* node = nullptr;
   {
      auto rows = sel_model->selectedRows();
      if (!rows.isEmpty())
         node = model->item(rows[0].row());
   }

   const auto blockers = std::array{
      QSignalBlocker(this->ui.currentFactionForm),
      QSignalBlocker(this->ui.currentFactionRank),
   };

   this->ui.currentFactionForm->setEnabled(node != nullptr);
   this->ui.currentFactionRank->setEnabled(node != nullptr);
   if (!node) {
      this->ui.currentFactionForm->setAllowNone(true);
      this->ui.currentFactionForm->setFormStub(nullptr);
      this->ui.currentFactionRank->setValue(-1);
      return;
   }
   this->ui.currentFactionForm->setAllowNone(false);
   this->ui.currentFactionForm->setFormStub(node->faction);
   this->ui.currentFactionRank->setValue(node->rank);
}
void FormDialogActorBase::_push_faction_from_ui();