#include "./actor_base.h"
#include <limits>
#include <QKeyEvent>
#include <QMessageBox>
#include "helpers/string/strlen.h"
#include "dovah/core.h"
#include "dovah/exceptions/actor_base_template_is_cyclical.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"

#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/game_settings/core.h"
#include "editor/subsystems/message_log/core.h"
#include "editor/open_window_for_form.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/form_stubs/helpers/get_template_actor.h"
#include "dovah/forms/Class.h"
#include "dovah/forms/Outfit.h"
#include "dovah/forms/Race.h"
#include "dovah/utils/compute_classed_stat_points.h"

#include "widgets/widget-data/DKFormPickerCustomFilter.h"
#include "widgets/DKHeaderView.h"

#include "./actor_base/ActorBaseCreatureSoundsModel.h"
#include "./actor_base/ActorBaseFactionsModel.h"
#include "./actor_base/ActorBaseRelationshipsModel.h"
#include "./actor_base/ActorBaseSkillsModel.h"
#include "./actor_base/ActorBaseTintLayerModel.h"
#include "./actor_base/FaceComplexionPickerFilter.h"
#include "./actor_base/FaceHairColorPickerFilter.h"
#include "./actor_base/FaceTintColorPickerFilter.h" // TODO: DEPRECATED; DELETE
#include "./shared/FaceBaseHeadPartsModel.h"
#include "./shared/FaceExtraHeadPartsModel.h"
#include "./shared/HeadPartPickerFilter.h"

#include "ui/types/logging/log_item.h"

namespace {
   constexpr const bool preview_enabled = false;

   // We don't currently have a renderer suitable for actor previews, and we don't have 
   // code to generate and export a head NIF and tintmask, so for now, disable all face 
   // editing widgets.
   constexpr const bool allow_customizing_face =
      #if _DEBUG
         true ||
      #endif
      false
   ;

   constexpr const bool filtered_dialogue_browser_implemented = false;
}

namespace impl {
   class CrimeFactionPickerFilter final : public DKFormPickerCustomFilter {
      protected:
         using fic_type = dovahkit::subsystems::form_info_cache::core;

      public:
         CrimeFactionPickerFilter(QObject* parent) : DKFormPickerCustomFilter(parent) {
            auto& fic = fic_type::get();
            QObject::connect(&fic, &fic_type::cachedFactionChanged, this, [this](dovah::form_stub& stub) {
               this->_refilter_form(stub);
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
            QObject::connect(&fic, &fic_type::cachedVoicetypeChanged, this, [this](dovah::form_stub& stub) {
               this->_refilter_form(stub);
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

   this->ui.tabbox->setCurrentIndex(0);

   this->ui.previewOptionsContainer->setVisible(preview_enabled);
   this->ui.buttonShowFilteredDialogue->setEnabled(filtered_dialogue_browser_implemented);

   this->_filters.exclude_self = new impl::DKFormPickerExcludeSingleFormFilter(this);

   #pragma region Not in any tab
      this->ui.templateActor->setAllowedFormTypes({ dovah::form_type::actor_base, dovah::form_type::leveled_character });
      QObject::connect(this->ui.templateActor, &DKFormPicker::formChanged, this, [this](dovah::form_stub* target) {
         this->ui.buttonEditTemplateActor->setEnabled(target != nullptr);
         this->ui.templateFlags->setEnabled(target != nullptr);
      });
      QObject::connect(this->ui.buttonEditTemplateActor, &QPushButton::clicked, this, [this]() {
         if (!this->form)
            return;
         auto* stub = this->form->template_data.actor.get_form_stub();
         if (!stub)
            return;
         open_edit_dialog_for_form(*stub);
      });
   #pragma endregion
   
   #pragma region Traits tab
      this->ui.race->setAllowedFormType(dovah::form_type::race);
      this->ui.skin->setAllowedFormType(dovah::form_type::armor);
      {
         auto* widget = this->ui.sex;
         widget->clear();
         widget->addItem(tr("Female"), (int)dovah::sex::female);
         widget->addItem(tr("Male"), (int)dovah::sex::male);
      }
      ui::set_unsigned_range<float>(this->ui.height);
      this->ui.bodyWeight->setRange(0, 1);
      this->ui.farawaySkin->setAllowedFormType(dovah::form_type::armor);
      ui::set_unsigned_range<float>(this->ui.farawayDistance);
      this->ui.voicetype->setAllowedFormType(dovah::form_type::voicetype);
      {
         auto* filter = this->_filters.voicetype = new impl::VoicetypePickerFilter(this);
         this->ui.voicetype->setCustomFilter(filter); // limit voicetypes by sex, as the game itself does
      }
      this->ui.deathItem->setAllowedFormType(dovah::form_type::leveled_item);

      ui::set_unsigned_range<decltype(decltype(loaded_form_type::stats)::disposition)>(this->ui.dispositionBase);
   #pragma endregion
   #pragma region Stats tab
      {
         using stats_struct = decltype(loaded_form_type::stats);

         ui::set_range<decltype(stats_struct::level)>(this->ui.level);
         ui::set_range<decltype(stats_struct::calc_min_level)>(this->ui.levelCalcMin);
         ui::set_range<decltype(stats_struct::calc_max_level)>(this->ui.levelCalcMax);
         QObject::connect(this->ui.level,           QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormDialogActorBase::_on_effective_level_changed);
         QObject::connect(this->ui.flagPCLevelMult, &QCheckBox::toggled, this, &FormDialogActorBase::_set_pc_level_mult);

         this->ui.speedPercentage->setRange(0, 32767);
         this->ui.bleedoutOverrideThreshold->setRange(0, 32767);

         this->ui.statsClass->setAllowedFormType(dovah::form_type::combat_class);
         this->ui.statsClass->setAllowNone(false);
         ui::set_range<loaded_form_type::attribute_value_type>(this->ui.statsHealthBase);
         ui::set_range<loaded_form_type::attribute_value_type>(this->ui.statsMagickaBase);
         ui::set_range<loaded_form_type::attribute_value_type>(this->ui.statsStaminaBase);
         ui::set_range<loaded_form_type::attribute_offset_type>(this->ui.statsHealthOffset);
         ui::set_range<loaded_form_type::attribute_offset_type>(this->ui.statsMagickaOffset);
         ui::set_range<loaded_form_type::attribute_offset_type>(this->ui.statsStaminaOffset);
         ui::set_range<loaded_form_type::attribute_value_type>(this->ui.statsHealthCalcFinal);
         ui::set_range<loaded_form_type::attribute_value_type>(this->ui.statsMagickaCalcFinal);
         ui::set_range<loaded_form_type::attribute_value_type>(this->ui.statsStaminaCalcFinal);
         ui::set_range<loaded_form_type::skill_offset_type>(this->ui.currentSkillOffset);
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

            ui::typical_tableview_config(widget);
            widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

            auto* sel_model = widget->selectionModel();
            QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, sel_model, model]() {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty()) {
                  this->ui.skillEditLayout->setEnabled(false);
                  return;
               }
               auto skill_opt = model->skillAtRow(rows[0].row());
               if (!skill_opt.has_value()) {
                  this->ui.skillEditLayout->setEnabled(false);
                  return;
               }
               this->ui.skillEditLayout->setEnabled(true);
               auto skill = skill_opt.value();

               const auto blocker = QSignalBlocker(this->ui.currentSkillOffset);
               this->ui.currentSkillOffset->setValue(this->form->stats.skills.offsets.list[(size_t)skill]);
            });
            QObject::connect(this->ui.currentSkillOffset, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, sel_model, model](int v) {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               auto skill_opt = model->skillAtRow(rows[0].row());
               if (!skill_opt.has_value())
                  return;
               auto skill = skill_opt.value();

               this->form->stats.skills.offsets.list[(size_t)skill] = v;
               this->_recalc_stats();
            });

            this->ui.skillEditLayout->setEnabled(false);
         }
         QObject::connect(this->ui.flagAutoCalcStats, &QCheckBox::toggled, this, [this](bool checked) {
            if (this->form) {
               auto& dst  = this->form->actor_flags;
               auto  flag = loaded_form_type::actor_flag::auto_calc_stats;
               if (checked)
                  dst |= flag;
               else
                  dst &= ~flag;
            }
            this->ui.skillEditLayout->setVisible(!checked);
            this->_models.skills->setOffsetsUsed(!checked);
            this->_recalc_stats();
         });
      }
   #pragma endregion
   #pragma region Factions tab
      this->_filters.crime_faction = new impl::CrimeFactionPickerFilter(this);
      {
         auto* widget = this->ui.factionsTable;
         auto* model  = this->_models.factions = new ActorBaseFactionsModel(this);
         widget->setModel(model);
         this->_filters.crime_faction->setModel(model);

         ui::typical_tableview_config(widget);
         widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(widget, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(ActorBaseFactionsModel::Column::Faction, 1, 0);
            header.setColumnFlex(ActorBaseFactionsModel::Column::Rank,    0, 0, metrics.boundingRect("99999").width() * 1.5F + 4);
         });

         auto* sel_model = widget->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &FormDialogActorBase::_pull_faction_to_ui);
         
         QObject::connect(this->ui.buttonFactionAdd, &QPushButton::clicked, this, [this, model, sel_model]() {
            auto qmi = model->create();
            if (qmi.isValid()) {
               auto col = model->columnCount({});

               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(col - 1);
               sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.buttonFactionRemove, &QPushButton::clicked, this, [this, model, sel_model]() {
            size_t row;
            {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               row = rows[0].row();
            }
            model->deleteItems(row, 1);
         });
      }
      this->ui.currentFactionForm->setAllowedFormType(dovah::form_type::faction);
      ui::set_range<int8_t>(this->ui.currentFactionRank);

      QObject::connect(this->ui.currentFactionForm, &DKFormPicker::formChanged, this, &FormDialogActorBase::_push_faction_from_ui);
      QObject::connect(this->ui.currentFactionRank, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormDialogActorBase::_push_faction_from_ui);

      this->ui.crimeFaction->setAllowedFormType(dovah::form_type::faction);
      this->ui.crimeFaction->setCustomFilter(this->_filters.crime_faction);
   #pragma endregion
   #pragma region Relationships tab
      {
         auto* widget = this->ui.relationshipsTable;
         auto* model  = this->_models.relationships = new ActorBaseRelationshipsModel(this);
         widget->setModel(model);

         ui::typical_tableview_config(widget);
         widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

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
      this->ui.packages->setAllowedFormTypes({ dovah::form_type::package });

      this->ui.packageListDefault->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListSpectator->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListObserveCorpse->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListGuardWarn->setAllowedFormType(dovah::form_type::formlist);
      this->ui.packageListCombat->setAllowedFormType(dovah::form_type::formlist);
   #pragma endregion
   #pragma region Inventory tab
      this->ui.outfitDefault->setAllowedFormType(dovah::form_type::outfit);
      this->ui.outfitSleep->setAllowedFormType(dovah::form_type::outfit);
      this->ui.outfitItemsPreview->setAllowDuplicates(true);
      this->ui.outfitItemsPreview->setReadOnly(true);
      ui::set_range<uint8_t>(this->ui.gearedUpWeapons);

      QObject::connect(this->ui.outfitDefault, &DKFormPicker::formChanged, this, &FormDialogActorBase::_update_outfit_contents_view);
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
      //
      QObject::connect(this->ui.inheritSoundsFrom, &DKFormPicker::formChanged, this, &FormDialogActorBase::_set_creature_sound_inherit_actor);

      {
         auto* widget = this->ui.creatureSoundsTable;
         auto* model  = this->_models.creature_sounds = new ActorBaseCreatureSoundsModel(this);
         widget->setModel(model);
         //
         ui::typical_tableview_config(widget);
         widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(widget, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(ActorBaseCreatureSoundsModel::Column::Form,   1, 0);
            header.setColumnFlex(ActorBaseCreatureSoundsModel::Column::Chance, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
            header.setColumnFlex(ActorBaseCreatureSoundsModel::Column::Type,   0, 0, metrics.boundingRect("Conscious Loop").width() * 1.5F + 4);
         });

         auto* sel_model = widget->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &FormDialogActorBase::_pull_creature_sound_to_ui);

         QObject::connect(this->ui.buttonCreaSoundAdd, &QPushButton::clicked, this, [this, model, sel_model]() {
            auto qmi = model->create();
            if (qmi.isValid()) {
               auto col = model->columnCount({});

               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(col - 1);
               sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.buttonCreaSoundRemove, &QPushButton::clicked, this, [this, model, sel_model]() {
            size_t row;
            {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               row = rows[0].row();
            }
            model->deleteItems(row, 1);
         });
      }

      ui::item_indices_to_data(this->ui.currentCreaSoundType);
      this->ui.currentCreaSoundChance->setRange(0, 100);
      this->ui.currentCreaSoundForm->setAllowedFormType(dovah::form_type::sound_descriptor);

      QObject::connect(this->ui.currentCreaSoundType,   QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FormDialogActorBase::_push_creature_sound_from_ui);
      QObject::connect(this->ui.currentCreaSoundChance, QOverload<int>::of(&QSpinBox::valueChanged), this, &FormDialogActorBase::_push_creature_sound_from_ui);
      QObject::connect(this->ui.currentCreaSoundForm,   &DKFormPicker::formChanged, this, &FormDialogActorBase::_push_creature_sound_from_ui);
   #pragma endregion
   #pragma region Attack Data tab
      //
      // Just the one premade widget. It'll set itself up for us.
      //
   #pragma endregion
   #pragma region Face Parts tab
      if constexpr (!allow_customizing_face) {
         this->ui.tabFaceParts->setEnabled(false);
      }

      this->_filters.face.base_head_part = new HeadPartPickerFilter(this);
      this->ui.baseHeadPartPicker->setAllowedFormType(dovah::form_type::head_part);
      this->ui.baseHeadPartPicker->setCustomFilter(this->_filters.face.base_head_part);

      this->_filters.face.complexion = new impl::FaceComplexionPickerFilter(this);
      this->ui.faceComplexion->setAllowedFormType(dovah::form_type::texture_set);
      this->ui.faceComplexion->setCustomFilter(this->_filters.face.complexion);

      this->_filters.face.hair_color = new impl::FaceHairColorPickerFilter(this);
      this->ui.hairColor->setAllowedFormType(dovah::form_type::color);
      this->ui.hairColor->setCustomFilter(this->_filters.face.hair_color);

      {
         auto* view  = this->ui.faceTintLayerTable;
         auto* model = this->_models.face_tints = new ActorBaseTintLayerModel(this);
         view->setModel(model);

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(ActorBaseTintLayerModel::Column::TexturePath, 1, 0);
            header.setColumnFlex(ActorBaseTintLayerModel::Column::Color, 0, 0, metrics.boundingRect("   ").width() * 1.5F + 4);
            header.setColumnFlex(ActorBaseTintLayerModel::Column::Alpha, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
         });
         QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormDialogActorBase::_pull_tint_layer_to_ui);

         QObject::connect(this->ui.faceTintColorUseCustom, &QRadioButton::toggled, this, [this](bool checked) {
            if (!checked)
               return;
            this->ui.faceTintColorPreset->setEnabled(false);
            this->ui.faceTintColorCustom->setEnabled(true);
            this->_push_tint_layer_from_ui();
         });
         QObject::connect(this->ui.faceTintColorUsePreset, &QRadioButton::toggled, this, [this](bool checked) {
            if (!checked)
               return;
            this->ui.faceTintColorPreset->setEnabled(true);
            this->ui.faceTintColorCustom->setEnabled(false);
            this->_push_tint_layer_from_ui();
         });
         QObject::connect(this->ui.faceTintColorCustom, &DKColorPickerButton::colorChanged, this, &FormDialogActorBase::_push_tint_layer_from_ui);
         QObject::connect(this->ui.faceTintColorPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FormDialogActorBase::_push_tint_layer_from_ui);
         QObject::connect(this->ui.faceTintColorInterpolation, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormDialogActorBase::_push_tint_layer_from_ui);
         QObject::connect(this->ui.faceTintColorInterpolationSlider, &DKFloatSlider::valueChanged, this, [this](float value) {
            this->ui.faceTintColorInterpolation->setValue(value);
         });
      }
      
      {  // Base Head Parts table
         auto* view  = this->ui.baseHeadPartsTable;
         auto* model = this->_models.head_parts_base = new FaceBaseHeadPartsModel(this);
         view->setModel(model);
         view->setAcceptDrops(true);
         view->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
         view->setDragDropOverwriteMode(false);
         view->setDropIndicatorShown(true);

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(0, 0, 0, metrics.boundingRect("Facial Hair").width() * 1.5F + 4);
            header.setColumnFlex(1, 1, 0);
         });

         auto* sel_model = view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, view, model, sel_model]() {
            auto* picker = this->ui.baseHeadPartPicker;

            std::optional<FaceBaseHeadPartsModel::Slot> slot;
            {
               auto rows = sel_model->selectedRows();
               if (!rows.isEmpty())
                  slot = model->slotAt(rows[0].row());
            }
            if (!slot.has_value()) {
               picker->setEnabled(false);
               return;
            }

            picker->setEnabled(true);
            auto* stub = model->headPartFor(slot.value());

            auto blocker = QSignalBlocker(picker);
            this->_filters.face.base_head_part->setRequiredType(FaceBaseHeadPartsModel::slotToType(slot.value()));
            picker->setFormStub(stub);
         });

         QObject::connect(this->ui.baseHeadPartPicker, &DKFormPicker::formChanged, this, [this, sel_model, model](dovah::form_stub* stub) {
            auto* picker = this->ui.baseHeadPartPicker;

            std::optional<FaceBaseHeadPartsModel::Slot> slot;
            {
               auto rows = sel_model->selectedRows();
               if (!rows.isEmpty())
                  slot = model->slotAt(rows[0].row());
            }
            if (!slot.has_value())
               return;

            model->setHeadPartFor(slot.value(), stub);
         });
      }

      {  // Additional Head Parts table
         auto* view  = this->ui.additionalHeadParts;
         auto* model = this->_models.head_parts_extra = new FaceExtraHeadPartsModel(this);
         view->setModel(model);
         view->setAcceptDrops(true);
         view->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
         view->setDragDropOverwriteMode(false);
         view->setDropIndicatorShown(true);

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(0, 0, 0, metrics.boundingRect("Facial Hair").width() * 1.5F + 4);
            header.setColumnFlex(1, 1, 0);
         });

         // Handle the Del key for removing head parts from the list.
         view->installEventFilter(this);
      }
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

   //
   // Bit unusual to do anything in the constructor but after loading, but:
   //
   {
      auto& gss = dovahkit::subsystems::game_settings::core::get();
      QObject::connect(&gss, &std::decay_t<decltype(gss)>::settingValueChanged, this, &FormDialogActorBase::_on_game_setting_changed);
   }
   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formModified, this, &FormDialogActorBase::_on_other_form_modified);
   }
}

void FormDialogActorBase::_on_game_setting_changed(const char* name) {
   //
   // Update auto-calc'd stats if any GMSTs used in the calculations are changed.
   //
   constexpr const auto relevant_settings = std::array{
      "iAVDSkillStart",
      "iAVDSkillsLevelUp",
      "iAVDAutoCalcSkillMax",
      "iAVDhmsLevelUp",
      "fPCHealthLevelBonus",
      "fNPCHealthLevelBonus",
   };
   
   bool found = true;
   for (auto* item : relevant_settings) {
      if (_strnicmp(name, item, cobb::strlen(item)) != 0) {
         found = true;
         break;
      }
   }
   if (found)
      this->_recalc_stats();
}
void FormDialogActorBase::_on_other_form_modified(dovah::form_stub* stub) {
   auto& working = *this->form;
   if (stub->form_type == dovah::form_type::actor_base || stub->form_type == dovah::form_type::leveled_character) {
      //
      // Detect changes to our template actor (or their template actor (or...)).
      //
      auto* current = dovah::form_stub_helpers::get_template_actor(this->formStub());
      for (; current; current = dovah::form_stub_helpers::get_template_actor(current)) {
         if (stub == current) {
            this->_update_from_template_actor();
            break;
         }
      }
   }
   if (stub->form_type == dovah::form_type::actor_base) {
      if (stub == working.creature_sounds.inherits_from()) {
         this->_set_creature_sound_inherit_actor(stub);
      }
   } else {
      if (stub == working.race.get_form_stub()) {
         this->_set_race(stub); // not redundant; should refresh everything
      } else if (stub == working.stats.combat_class.get_form_stub()) {
         this->_recalc_stats();
      }
   }
}

void FormDialogActorBase::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   //
   // Corrections:
   //
   if (!working.race) {
      write_form_ref(working.race, editor.get_form_of_probable_type(dovah::form_type::race, dovah::hardcoded_form_ids::DefaultRace));
   }
   {
      bool is_unique = working.actor_flags & loaded_form_type::actor_flag::unique;
      if (is_unique) {
         dovah::form_stub* leveled_base = this->_is_templated_from_leveled_base();
         if (leveled_base) {
            ui::types::log_item item;
            item.context = ui::types::log_item_context::form_load;
            item.type    = ui::types::log_item_type::warning;
            item.text    =
               tr("ActorBase %1 is flagged as Unique, but it pulls pulls template data "
                  "(directly or indirectly) from Leveled Character %2. The Unique flag "
                  "will be removed."
               )
               .arg(QString::fromStdString(this->formStub()->editorID))
               .arg(QString::fromStdString(leveled_base->editorID));
            dovahkit::subsystems::message_log::core::get().addLogItem(item);
         }
      }
   }

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
         this->ui.templateActor->setFormStub(working.template_data.actor.get_form_stub());
         QObject::connect(this->ui.templateActor, &DKFormPicker::formChanged, this, &FormDialogActorBase::_set_template_actor);
         //
         ui::bind(this->ui.templateUseAIData, working.template_data.flags, loaded_form_type::template_flag::use_ai_data);
         ui::bind(this->ui.templateUseAIPackages, working.template_data.flags, loaded_form_type::template_flag::use_ai_packages);
         ui::bind(this->ui.templateUseAttackData, working.template_data.flags, loaded_form_type::template_flag::use_attack_data);
         ui::bind(this->ui.templateUseBaseData, working.template_data.flags, loaded_form_type::template_flag::use_base_data);
         ui::bind(this->ui.templateUsePackageFormLists, working.template_data.flags, loaded_form_type::template_flag::use_package_overrides);
         ui::bind(this->ui.templateUseFactions, working.template_data.flags, loaded_form_type::template_flag::use_factions);
         ui::bind(this->ui.templateUseInventory, working.template_data.flags, loaded_form_type::template_flag::use_inventory);
         ui::bind(this->ui.templateUseKeywords, working.template_data.flags, loaded_form_type::template_flag::use_keywords);
         ui::bind(this->ui.templateUseScripts, working.template_data.flags, loaded_form_type::template_flag::use_scripts);
         ui::bind(this->ui.templateUseSpellList, working.template_data.flags, loaded_form_type::template_flag::use_spells);
         ui::bind(this->ui.templateUseStats, working.template_data.flags, loaded_form_type::template_flag::use_stats);
         ui::bind(this->ui.templateUseTraits, working.template_data.flags, loaded_form_type::template_flag::use_traits);
         //
         // This next signal has to be registered here, to ensure it runs after the flag is changed.
         //
         auto checkboxes = std::array{
            this->ui.templateUseTraits,
            this->ui.templateUseStats,
            this->ui.templateUseScripts,
            this->ui.templateUseFactions,
            this->ui.templateUseAIData,
            this->ui.templateUseAIPackages,
            this->ui.templateUsePackageFormLists,
            this->ui.templateUseAttackData,
            this->ui.templateUseSpellList,
            this->ui.templateUseInventory,
            this->ui.templateUseBaseData,
            this->ui.templateUseKeywords,
         };
         for (auto* checkbox : checkboxes) {
            QObject::connect(checkbox, &QCheckBox::stateChanged, this, &FormDialogActorBase::_update_from_template_actor);
         }
      #pragma endregion
   #pragma endregion

   #pragma region Traits tab
      QObject::connect(this->ui.race, &DKFormPicker::formChanged, this, &FormDialogActorBase::_set_race);
      ui::bind(this->ui.skin, working.skin, working);
      QObject::connect(this->ui.sex, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         auto sex = this->ui.sex->currentData().toInt();
         this->_set_sex((dovah::sex)sex);
      });
      ui::bind(this->ui.flagOppositeGenderAnims, working.template_data.flags, loaded_form_type::actor_flag::opposite_gender_animations);
      ui::bind(this->ui.height,     working.height);
      ui::bind(this->ui.bodyWeight, working.weight);
      ui::bind(this->ui.farawaySkin,     working.far_away.model, working);
      ui::bind(this->ui.farawayDistance, working.far_away.distance);
      ui::bind(this->ui.voicetype, working.voicetype, working);
      ui::bind(this->ui.dispositionBase, working.stats.disposition);
      ui::bind(this->ui.deathItem, working.death_item, working);
   #pragma endregion
   #pragma region Stats tab
      QObject::connect(this->ui.level, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
         auto& working = *this->form;
         if (working.actor_flags & loaded_form_type::actor_flag::pc_level_mult) {
            working.stats.level = value * 1000;
         } else {
            working.stats.level = value;
            this->_recalc_stats();
         }
      });
      ui::bind(this->ui.levelCalcMin, working.stats.calc_min_level);
      ui::bind(this->ui.levelCalcMax, working.stats.calc_max_level);

      // DO NOT bind this; we already registered a handler for it, and that handler ensures that we do things 
      // in the proper order (i.e. change the actor flag and then recalc stats).
      //ui::bind(this->ui.flagPCLevelMult, working.actor_flags, loaded_form_type::actor_flag::pc_level_mult);
      this->ui.flagPCLevelMult->setChecked(working.actor_flags & loaded_form_type::actor_flag::pc_level_mult);

      ui::bind(this->ui.speedPercentage, working.stats.speed_mult);
      ui::bind(this->ui.flagBleedoutOverride, working.actor_flags, loaded_form_type::actor_flag::bleedout_override);
      ui::bind(this->ui.bleedoutOverrideThreshold, working.stats.bleedout_threshold);

      ui::bind(this->ui.statsClass, working.stats.combat_class, working);

      // DO NOT bind this; we already registered a handler for it, and that handler ensures that we do things 
      // in the proper order (i.e. change the actor flag and then recalc stats).
      //ui::bind(this->ui.flagAutoCalcStats, working.actor_flags, loaded_form_type::actor_flag::auto_calc_stats);
      this->ui.flagAutoCalcStats->setChecked(working.actor_flags & loaded_form_type::actor_flag::auto_calc_stats);
      //
      {  // Attributes
         auto spinbox_change_signal = QOverload<int>::of(&QSpinBox::valueChanged);
         QObject::connect(this->ui.statsHealthOffset,  spinbox_change_signal, this, &FormDialogActorBase::_recalc_stats);
         QObject::connect(this->ui.statsMagickaOffset, spinbox_change_signal, this, &FormDialogActorBase::_recalc_stats);
         QObject::connect(this->ui.statsStaminaOffset, spinbox_change_signal, this, &FormDialogActorBase::_recalc_stats);
         QObject::connect(this->ui.statsClass, &DKFormPicker::formChanged, this, &FormDialogActorBase::_recalc_stats);
         // Recalculating attributes on-race-change is done in the `_set_race` method.
      }
      this->_models.skills->setAllData(
         working.stats.skills.offsets.list,
         working.stats.skills.calculated.list
      );
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
      ui::bind(this->ui.crimeFaction, working.crime_faction, working);
   #pragma endregion
   #pragma region Relationships tab
      this->_models.relationships->setFocusActor(
         this->formStub(),
         (working.actor_flags & loaded_form_type::actor_flag::female) ? dovah::sex::female : dovah::sex::male
      );
   #pragma endregion
   #pragma region Keywords tab
      this->ui.keywords->pullStubs(working.keywords.forms);
   #pragma endregion
   #pragma region AI Data tab
      ui::bind(this->ui.mood, working.ai.mood);
      ui::bind(this->ui.aiEnergy, working.ai.energy_level);
      ui::bind(this->ui.giftFilter, working.gift_filter, working);

      ui::bind(this->ui.aggression, working.ai.aggression);
      ui::bind(this->ui.confidence, working.ai.confidence);
      ui::bind(this->ui.assistance, working.ai.assistance);
      ui::bind(this->ui.morality,   working.ai.morality);
      ui::bind(this->ui.aggroRadii, working.ai.aggro.use_radius);
      ui::bind(this->ui.aggroRadiusWarn,       working.ai.aggro.warn);
      ui::bind(this->ui.aggroRadiusWarnAttack, working.ai.aggro.warn_attack);
      ui::bind(this->ui.aggroRadiusAttack,     working.ai.aggro.attack);
   #pragma endregion
   #pragma region AI Packages tab
      this->ui.packages->pullStubs(working.ai.package_list);

      ui::bind(this->ui.packageListDefault,       working.ai.default_package_list, working);
      ui::bind(this->ui.packageListSpectator,     working.ai.package_override_lists.spectator, working);
      ui::bind(this->ui.packageListObserveCorpse, working.ai.package_override_lists.observe_corpse, working);
      ui::bind(this->ui.packageListGuardWarn,     working.ai.package_override_lists.guard_warn, working);
      ui::bind(this->ui.packageListCombat,        working.ai.package_override_lists.combat, working);
   #pragma endregion
   #pragma region Inventory tab
      QObject::connect(this->ui.outfitDefault, &DKFormPicker::formChanged, this, &FormDialogActorBase::_update_outfit_contents_view);
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
      this->_set_creature_sound_inherit_actor(working.creature_sounds.inherits_from());
   #pragma endregion
   #pragma region Attack Data tab
      this->ui.attackData->initializeFrom(working.attack_data);
   #pragma endregion
   #pragma region Face Parts tab
      ui::bind(this->ui.faceComplexion, working.facegen.complexion, working);
      ui::bind(this->ui.hairColor,      working.facegen.hair_color, working);
      //
      // The rest is done below, after updating from the template actor.
      //
   #pragma endregion
   #pragma region Face Morphs tab
      {
         auto _handle = [](DKFloatSlider* widget, float& value) {
            widget->setRange(-1, 1);
            widget->setTickInterval(0.2);
            ui::bind(widget, value);
         };

         auto& values = working.facegen.morphs.sliders;

         _handle(this->ui.faceMorphBrowDepth,  values.brows.depth);
         _handle(this->ui.faceMorphBrowHeight, values.brows.height);
         _handle(this->ui.faceMorphBrowWidth,  values.brows.width);
         
         {
            auto* widget = this->ui.faceMorphMouthIndex;
            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget](int index) {
               auto& dst = this->form->facegen.morphs.indices.lips;
               if (index < 0 && widget->count() == 0) {
                  dst = -1;
                  return;
               }
               auto i = widget->currentData().toInt();
               dst = i;
            });
         }
         _handle(this->ui.faceMorphMouthHeight, values.mouth.height);
         _handle(this->ui.faceMorphMouthDepth,  values.mouth.depth);

         _handle(this->ui.faceMorphChinDepth,  values.chin.depth);
         _handle(this->ui.faceMorphChinLength, values.chin.height);
         _handle(this->ui.faceMorphChinWidth,  values.chin.width);

         _handle(this->ui.faceMorphJawDepth,  values.jaw.depth);
         _handle(this->ui.faceMorphJawHeight, values.jaw.height);
         _handle(this->ui.faceMorphJawWidth,  values.jaw.width);

         _handle(this->ui.faceMorphCheekbonesHeight, values.cheeks.height);
         _handle(this->ui.faceMorphCheekbonesWidth,  values.cheeks.width);

         {
            auto* widget = this->ui.faceMorphEyesIndex;
            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget](int index) {
               auto& dst = this->form->facegen.morphs.indices.eyes;
               if (index < 0 && widget->count() == 0) {
                  dst = -1;
                  return;
               }
               auto i = widget->currentData().toInt();
               dst = i;
            });
         }
         _handle(this->ui.faceMorphEyesDepth,  values.eyes.depth);
         _handle(this->ui.faceMorphEyesHeight, values.eyes.height);
         _handle(this->ui.faceMorphEyesWidth,  values.eyes.width);

         {
            auto* widget = this->ui.faceMorphNoseIndex;
            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget](int index) {
               auto& dst = this->form->facegen.morphs.indices.nose;
               if (index < 0 && widget->count() == 0) {
                  dst = -1;
                  return;
               }
               auto i = widget->currentData().toInt();
               dst = i;
            });
         }
         _handle(this->ui.faceMorphNoseHeight, values.nose.height);
         _handle(this->ui.faceMorphNoseLength, values.nose.length);

         // This one is in the range [0, 1] rather than [-1, 1], I believe.
         this->ui.faceMorphVampire->setTickInterval(0.2);
         ui::bind(this->ui.faceMorphVampire, values.vampire_morph);
      }
   #pragma endregion
   #pragma region Face Anim Preview tab
      static_assert(!preview_enabled, "TODO");
   #pragma endregion

   this->_update_from_template_actor();
   this->_pull_headparts_to_ui();
   {
      bool female = (working.actor_flags & loaded_form_type::actor_flag::female) != 0;

      this->_models.face_tints->importLayerStates(working);
      this->_set_sex(female ? dovah::sex::female : dovah::sex::male);
   }
   this->_set_race(working.race.get_form_stub());
   this->_set_pc_level_mult(this->ui.flagPCLevelMult->isChecked());
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
   
   editor.assign_localized_string(working.name,       this->ui.name->text());
   editor.assign_localized_string(working.short_name, this->ui.shortName->text());
   this->ui.destructionData->commitTo(working.destruction_data, working);
   this->ui.scriptListPane->commit();

   if (this->_is_templated_from_leveled_base() != nullptr) {
      //
      // Actors that are templated on LeveledActors can't be Unique. We clear the 
      // checkbox and grey it out when you set one, but we don't actually clear the 
      // flag during save time (so that if you change the template actor to one that 
      // isn't a LeveledActor, we can re-check the checkbox).
      //
      working.actor_flags &= ~loaded_form_type::actor_flag::unique;
   }

   #pragma region Tabs
      #pragma region Traits
         //
         // All data is bound, widget-to-field, and live-updated.
         //
      #pragma endregion
      #pragma region Stats
         //
         // All data is bound, widget-to-field, and live-updated.
         //
      #pragma endregion
      #pragma region Factions
         {
            auto* model = this->_models.factions;
            auto& dst   = working.faction_memberships;
            for (auto& item : dst)
               item.faction.set(working, nullptr);
            dst.clear();

            size_t size = model->rowCount();
            for (size_t i = 0; i < size; ++i) {
               auto* src_item = model->item(i);
               if (!src_item)
                  break;
               if (!src_item->faction)
                  continue;
               auto& dst_item = dst.emplace_back();
               dst_item.rank = src_item->rank;
               write_form_ref(dst_item.faction, src_item->faction);
            }
         }
      #pragma endregion
      #pragma region Relationships
         //
         // Nothing to save. This tab just exists to let you view relationship forms 
         // that refer to this actor.
         //
      #pragma endregion
      #pragma region Keywords
         this->ui.keywords->commitStubs(working.keywords.forms, working);
      #pragma endregion
      #pragma region AI Data
         //
         // All data is bound, widget-to-field, and live-updated.
         //
      #pragma endregion
      #pragma region AI Packages
         this->ui.packages->commitStubs(working.ai.package_list, working);
      #pragma endregion
      #pragma region Inventory
         this->ui.inventory->commitTo(working.inventory, working);
      #pragma endregion
      #pragma region Magic and Perks
         this->ui.perks->commitStubs(working.perks, working);
         this->ui.spells->commitStubs(working.spells.forms, working);
      #pragma endregion
      #pragma region Sounds
      {
         auto& dst = working.creature_sounds;
         if (auto* stub = this->ui.inheritSoundsFrom->formStub()) {
            dst.set_inherits_from(*this->form, stub);
         } else {
            std::vector<dovah::loaded_forms::structs::actor_creature_sounds::entry> entries;
            {
               auto*  model = this->_models.creature_sounds;
               size_t size  = model->rowCount();
               entries.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  auto* data = model->item(i);
                  if (!data)
                     continue;
                  entries[i] = *data;
               }
            }
            dst.replace_sounds(*this->form, entries);
         }
      }
      #pragma endregion
      #pragma region Attack Data
         this->ui.attackData->commitTo(working.attack_data, working);
      #pragma endregion
      #pragma region Face Parts
         this->_models.face_tints->exportLayerStates(working);
         {  // HeadParts list
            std::vector<dovah::form_stub*> head_parts;
            {
               auto* model = this->_models.head_parts_base;
               if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Brows))
                  head_parts.push_back(stub);
               if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Eyes))
                  head_parts.push_back(stub);
               if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Face))
                  head_parts.push_back(stub);
               if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::FacialHair))
                  head_parts.push_back(stub);
               if (auto* stub = model->headPartFor(FaceBaseHeadPartsModel::Slot::Hair))
                  head_parts.push_back(stub);
            }
            {
               auto*  model = this->_models.head_parts_extra;
               size_t size  = model->rowCount();
               head_parts.reserve(head_parts.size() + size);
               for (size_t i = 0; i < size; ++i) {
                  if (auto* stub = model->headPart(i))
                     head_parts.push_back(stub);
               }
            }

            auto& dst_list = working.facegen.head_parts;
            dovah::clear_form_reference_list(dst_list, working);
            for (auto* stub : head_parts) {
               auto& form_use = dst_list.emplace_back();
               write_form_ref(form_use, stub);
            }
         }
      #pragma endregion
      #pragma region Face Morphs
         //
         // All data is bound, widget-to-field, and live-updated.
         //
      #pragma endregion
   #pragma endregion
}

void FormDialogActorBase::updatePreview() {
   static_assert(!preview_enabled, "The 3D preview is not yet implemented. Don't enable it until it's implemented!");
}
void FormDialogActorBase::_update_outfit_contents_view() {
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

dovah::form_stub* FormDialogActorBase::_is_templated_from_leveled_base() const {
   std::vector<dovah::form_stub*> seen;

   dovah::form_stub* current = this->form->template_data.actor.get_form_stub();
   for(; current; current = dovah::form_stub_helpers::get_template_actor(current)) {
      if (std::find(seen.begin(), seen.end(), current) != seen.end())
         break;
      seen.push_back(current);

      if (current->form_type == dovah::form_type::leveled_character)
         return current;
   }
   return nullptr;
}
void FormDialogActorBase::_set_template_actor(dovah::form_stub* desired) {
   if (desired) {
      switch (desired->form_type) {
         case dovah::form_type::actor_base:
         case dovah::form_type::leveled_character:
            break;
         default:
            desired = nullptr;
            break;
      }
   }
   if (this->form->template_data.actor.get_form_stub() == desired)
      return;
   if (desired) {
      //
      // Guard against cyclical references. (NOTE: We don't care about incidental 
      // cyclical references occurring elsewhere; we only care about cyclical 
      // references that would come to exist specifically as a result of setting 
      // this template relationship right here. In practice, we only care about 
      // cycles that loop through this very ActorBase that we're editing.)
      //
      bool would_create_a_new_cycle = false;
      {
         std::vector<dovah::form_stub*> seen;
         dovah::form_stub* current = desired;
         for (; current; current = dovah::form_stub_helpers::get_template_actor(current)) {
            if (current == this->formStub()) {
               would_create_a_new_cycle = true;
               break;
            }
            if (std::find(seen.begin(), seen.end(), current) != seen.end()) {
               break;
            }
            seen.push_back(current);
         }
      }
      if (would_create_a_new_cycle) {
         QString text = tr(
            "You have selected ActorBase %1 as this ActorBase's template, but doing so would "
            "form a cyclical reference. The selected ActorBase ultimately inherits from this "
            "ActorBase.\n\n"
            "A cyclical reference is a situation where A inherits from B, who inherits from "
            "C, who inherits from A: if you follow the chain, it loops back around on itself "
            "like a set of Penrose stairs or a Möbius strip.\n\n"
            "Skyrim knows how to avoid getting stuck in an infinite loop, but the practical "
            "consequence of this cyclical reference is that the data that each involved actor "
            "will inherit will depend on what order Skyrim happens to process the template "
            "relationships in, rather than being consistent.\n\n"
            "This selection will not be used.",
            "cyclical template actor warning"
         )
            .arg(QString::fromStdString(desired->editorID))
         ;
         QMessageBox::critical(this, tr("Error"), text);

         auto* prior = this->form->template_data.actor.get_form_stub();
         if (prior != desired) {
            const auto blocker = QSignalBlocker(this->ui.templateActor);
            this->ui.templateActor->setFormStub(this->form->template_data.actor.get_form_stub());
         }
         return;
      }
   }
   write_form_ref(this->form->template_data.actor, desired);
   this->_update_from_template_actor();
}
void FormDialogActorBase::_update_from_template_actor() {
   using template_flag = loaded_form_type::template_flag;

   auto* base  = this->form->template_data.actor.get_form_stub();
   auto  flags = this->form->template_data.flags;
   if (!base) {
      flags = 0;
   }
   {  // Traits flag
      bool enable = !(flags & template_flag::use_traits);
      this->ui.tabTraits->setEnabled(enable);
      this->ui.tabSounds->setEnabled(enable);
      this->ui.tabFaceParts->setEnabled(enable);
      this->ui.tabFaceMorphs->setEnabled(enable);
   }
   this->ui.tabStats->setEnabled(!(flags & template_flag::use_stats));
   this->ui.tabFactions->setEnabled(!(flags & template_flag::use_factions));
   this->ui.tabSpells->setEnabled(!(flags & template_flag::use_spells));
   this->ui.tabAIData->setEnabled(!(flags & template_flag::use_ai_data));
   this->ui.packages->setEnabled(!(flags & template_flag::use_ai_packages));
   this->ui.tabInventory->setEnabled(!(flags & template_flag::use_inventory));
   this->ui.defaultPackagesGroupbox->setEnabled(!(flags & template_flag::use_package_overrides));
   this->ui.tabAttackData->setEnabled(!(flags & template_flag::use_attack_data));
   this->ui.tabKeywords->setEnabled(!(flags & template_flag::use_keywords));
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

   {
      auto* widget        = this->ui.flagUnique;
      bool  can_be_unique = true;
      if (base) {
         can_be_unique = this->_is_templated_from_leveled_base() == nullptr;
      }
      widget->setEnabled(can_be_unique);

      const auto blocker = QSignalBlocker(widget);
      if (can_be_unique) {
         widget->setChecked(this->form->actor_flags & loaded_form_type::actor_flag::unique);
      } else {
         widget->setChecked(false);
      }
   }

   if (!base) {
      return;
   }

   auto& working = *this->form;
   try {
      working.copy_data_from_template_actor();
   } catch (const dovah::exceptions::actor_base_template_is_cyclical& e) {
      return;
   }
   for (size_t i = 0; i < 13; ++i) {
      auto mask = (template_flag::type)(1 << i);
      if (working.template_data.flags & mask)
         this->_push_data_to_ui(mask);
   }
}

void FormDialogActorBase::_push_data_to_ui(loaded_form_type::template_flag::type flag) {
   using actor_flag    = loaded_form_type::actor_flag;
   using template_flag = loaded_form_type::template_flag;

   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   switch (flag) {
      case template_flag::use_traits:
         {
            // Must set these first, so the various formpicker filters get updated.
            this->_set_sex(working.actor_flags & actor_flag::female ? dovah::sex::female : dovah::sex::male);
            this->_set_race(working.race.get_form_stub());

            { // Traits
               const auto blockers = std::array{
                  QSignalBlocker(this->ui.skin),
                  QSignalBlocker(this->ui.height),
                  QSignalBlocker(this->ui.bodyWeight),
                  QSignalBlocker(this->ui.farawaySkin),
                  QSignalBlocker(this->ui.farawayDistance),
                  QSignalBlocker(this->ui.voicetype),
                  QSignalBlocker(this->ui.dispositionBase),
                  QSignalBlocker(this->ui.deathItem),
                  QSignalBlocker(this->ui.flagOppositeGenderAnims),
               };
               this->ui.skin->setFormStub(working.skin.get_form_stub());
               this->ui.height->setValue(working.height);
               this->ui.bodyWeight->setValue(working.weight);
               this->ui.farawaySkin->setFormStub(working.far_away.model.get_form_stub());
               this->ui.farawayDistance->setValue(working.far_away.distance);
               this->ui.voicetype->setFormStub(working.voicetype.get_form_stub());
               this->ui.dispositionBase->setValue(working.stats.disposition);
               this->ui.deathItem->setFormStub(working.death_item.get_form_stub());
               this->ui.flagOppositeGenderAnims->setChecked(working.actor_flags & actor_flag::opposite_gender_animations);
            }
            { // Sounds
               const auto blockers = std::array{
                  QSignalBlocker(this->ui.soundLevel),
                  QSignalBlocker(this->ui.inheritSoundsFrom),
               };
               this->ui.soundLevel->setCurrentIndex(this->ui.soundLevel->findData((int)working.sound_level));
               this->ui.inheritSoundsFrom->setFormStub(working.creature_sounds.inherits_from());

               this->_creature_sound_inheritance_changed();
            }
            { // Face Parts
               this->ui.faceComplexion->setFormStub(working.facegen.complexion.get_form_stub());
               this->ui.hairColor->setFormStub(working.facegen.hair_color.get_form_stub());

               this->_models.face_tints->importLayerStates(working);
               this->_models.face_tints->setSex(this->_current_sex());
               this->_pull_headparts_to_ui();
            }
            { // Face Morphs
               const auto blockers = std::array{
                  QSignalBlocker(this->ui.faceMorphBrowDepth),
                  QSignalBlocker(this->ui.faceMorphBrowHeight),
                  QSignalBlocker(this->ui.faceMorphBrowWidth),
                  //
                  QSignalBlocker(this->ui.faceMorphMouthIndex),
                  QSignalBlocker(this->ui.faceMorphMouthDepth),
                  QSignalBlocker(this->ui.faceMorphMouthHeight),
                  //
                  QSignalBlocker(this->ui.faceMorphChinDepth),
                  QSignalBlocker(this->ui.faceMorphChinLength),
                  QSignalBlocker(this->ui.faceMorphChinWidth),
                  //
                  QSignalBlocker(this->ui.faceMorphJawDepth),
                  QSignalBlocker(this->ui.faceMorphJawHeight),
                  QSignalBlocker(this->ui.faceMorphJawWidth),
                  //
                  QSignalBlocker(this->ui.faceMorphCheekbonesHeight),
                  QSignalBlocker(this->ui.faceMorphCheekbonesWidth),
                  //
                  QSignalBlocker(this->ui.faceMorphEyesIndex),
                  QSignalBlocker(this->ui.faceMorphEyesDepth),
                  QSignalBlocker(this->ui.faceMorphEyesHeight),
                  QSignalBlocker(this->ui.faceMorphEyesWidth),
                  //
                  QSignalBlocker(this->ui.faceMorphNoseIndex),
                  QSignalBlocker(this->ui.faceMorphNoseHeight),
                  QSignalBlocker(this->ui.faceMorphNoseLength),
                  //
                  QSignalBlocker(this->ui.faceMorphVampire),
               };

               const auto& morphs = working.facegen.morphs.sliders;
               this->ui.faceMorphBrowDepth->setValue(morphs.brows.depth);
               this->ui.faceMorphBrowHeight->setValue(morphs.brows.height);
               this->ui.faceMorphBrowWidth->setValue(morphs.brows.width);
               //
               this->ui.faceMorphMouthIndex->setCurrentIndex(this->ui.faceMorphMouthIndex->findData(working.facegen.morphs.indices.lips));
               this->ui.faceMorphMouthDepth->setValue(morphs.mouth.depth);
               this->ui.faceMorphMouthHeight->setValue(morphs.mouth.height);
               //
               this->ui.faceMorphChinDepth->setValue(morphs.chin.depth);
               this->ui.faceMorphChinLength->setValue(morphs.chin.height);
               this->ui.faceMorphChinWidth->setValue(morphs.chin.width);
               //
               this->ui.faceMorphJawDepth->setValue(morphs.jaw.depth);
               this->ui.faceMorphJawHeight->setValue(morphs.jaw.height);
               this->ui.faceMorphJawWidth->setValue(morphs.jaw.width);
               //
               this->ui.faceMorphCheekbonesHeight->setValue(morphs.cheeks.height);
               this->ui.faceMorphCheekbonesWidth->setValue(morphs.cheeks.width);
               //
               this->ui.faceMorphEyesIndex->setCurrentIndex(this->ui.faceMorphEyesIndex->findData(working.facegen.morphs.indices.eyes));
               this->ui.faceMorphEyesDepth->setValue(morphs.eyes.depth);
               this->ui.faceMorphEyesHeight->setValue(morphs.eyes.height);
               this->ui.faceMorphEyesWidth->setValue(morphs.eyes.width);
               //
               this->ui.faceMorphNoseIndex->setCurrentIndex(this->ui.faceMorphNoseIndex->findData(working.facegen.morphs.indices.nose));
               this->ui.faceMorphNoseHeight->setValue(morphs.nose.height);
               this->ui.faceMorphNoseLength->setValue(morphs.nose.length);
               //
               this->ui.faceMorphVampire->setValue(morphs.vampire_morph);
            }
         }
         break;
      case template_flag::use_stats:
         {
            const auto blockers = std::array{
               //QSignalBlocker(this->ui.flagPCLevelMult), // handled by _set_pc_level_mult
               QSignalBlocker(this->ui.flagAutoCalcStats),
               //QSignalBlocker(this->ui.level),           // handled by _set_pc_level_mult
               QSignalBlocker(this->ui.levelCalcMin),
               QSignalBlocker(this->ui.levelCalcMax),
               QSignalBlocker(this->ui.statsHealthOffset),
               QSignalBlocker(this->ui.statsMagickaOffset),
               QSignalBlocker(this->ui.statsStaminaOffset),
               QSignalBlocker(this->ui.speedPercentage),
               QSignalBlocker(this->ui.flagBleedoutOverride),
               QSignalBlocker(this->ui.bleedoutOverrideThreshold),
               QSignalBlocker(this->ui.statsClass),
            };
            this->ui.flagAutoCalcStats->setChecked(working.actor_flags & actor_flag::auto_calc_stats);
            this->ui.levelCalcMin->setValue(working.stats.calc_min_level);
            this->ui.levelCalcMax->setValue(working.stats.calc_max_level);
            this->ui.statsHealthOffset->setValue(working.stats.attributes.offsets.health);
            this->ui.statsMagickaOffset->setValue(working.stats.attributes.offsets.magicka);
            this->ui.statsStaminaOffset->setValue(working.stats.attributes.offsets.stamina);
            this->ui.speedPercentage->setValue(working.stats.speed_mult);
            this->ui.flagBleedoutOverride->setChecked(working.actor_flags & actor_flag::bleedout_override);
            this->ui.bleedoutOverrideThreshold->setValue(working.stats.bleedout_threshold);
            this->ui.statsClass->setFormStub(working.stats.combat_class.get_form_stub());
         }
         this->_set_pc_level_mult(working.actor_flags & actor_flag::pc_level_mult); // update UI
         break;
      case template_flag::use_factions:
         {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.currentFactionForm),
               QSignalBlocker(this->ui.currentFactionRank),
               QSignalBlocker(this->ui.crimeFaction),
            };
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
            this->ui.crimeFaction->setFormStub(working.crime_faction.get_form_stub());
         }
         break;
      case template_flag::use_spells:
         this->ui.spells->pullStubs(working.spells.forms);
         this->ui.perks->pullStubs(working.perks);
         break;
      case template_flag::use_ai_data:
         {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.mood),
               QSignalBlocker(this->ui.aiEnergy),
               QSignalBlocker(this->ui.giftFilter),
               QSignalBlocker(this->ui.aggression),
               QSignalBlocker(this->ui.confidence),
               QSignalBlocker(this->ui.assistance),
               QSignalBlocker(this->ui.morality),
               QSignalBlocker(this->ui.aggroRadii),
               QSignalBlocker(this->ui.aggroRadiusWarn),
               QSignalBlocker(this->ui.aggroRadiusWarnAttack),
               QSignalBlocker(this->ui.aggroRadiusAttack),
            };
            this->ui.mood->setCurrentIndex(this->ui.mood->findData((int)working.ai.mood));
            this->ui.aiEnergy->setValue(working.ai.energy_level);
            this->ui.giftFilter->setFormStub(working.gift_filter.get_form_stub());
            this->ui.aggression->setCurrentIndex(this->ui.aggression->findData((int)working.ai.aggression));
            this->ui.confidence->setCurrentIndex(this->ui.confidence->findData((int)working.ai.confidence));
            this->ui.assistance->setCurrentIndex(this->ui.assistance->findData((int)working.ai.assistance));
            this->ui.morality->setCurrentIndex(this->ui.morality->findData((int)working.ai.morality));
            this->ui.aggroRadii->setChecked(working.ai.aggro.use_radius);
            this->ui.aggroRadiusWarn->setValue(working.ai.aggro.warn);
            this->ui.aggroRadiusWarnAttack->setValue(working.ai.aggro.warn_attack);
            this->ui.aggroRadiusAttack->setValue(working.ai.aggro.attack);
         }
         break;
      case template_flag::use_ai_packages:
         this->ui.packages->pullStubs(working.ai.package_list);
         break;
      case template_flag::use_animations:
         //
         // None?
         //
         break;
      case template_flag::use_base_data:
         {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.name),
               QSignalBlocker(this->ui.shortName),
               QSignalBlocker(this->ui.flagEssential),
               QSignalBlocker(this->ui.flagProtected),
               QSignalBlocker(this->ui.flagRespawn),
               QSignalBlocker(this->ui.flagSummonable),
               QSignalBlocker(this->ui.flagSimple),
               QSignalBlocker(this->ui.flagDoesntAffectStealthMeter),
            };

            this->ui.name->setText(editor.convert_localized_string(working.name));
            this->ui.shortName->setText(editor.convert_localized_string(working.short_name));
            //
            auto flags = working.actor_flags;
            this->ui.flagEssential->setChecked(flags & actor_flag::essential);
            this->ui.flagProtected->setChecked(flags & actor_flag::is_protected);
            this->ui.flagRespawn->setChecked(flags & actor_flag::respawn);
            this->ui.flagSummonable->setChecked(flags & actor_flag::summonable);
            this->ui.flagSimple->setChecked(flags & actor_flag::simple_actor);
            this->ui.flagDoesntAffectStealthMeter->setChecked(flags & actor_flag::doesnt_affect_stealth_meter);
         }
         break;
      case template_flag::use_inventory:
         {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.outfitDefault),
               QSignalBlocker(this->ui.outfitSleep),
               QSignalBlocker(this->ui.gearedUpWeapons),
            };
            this->ui.outfitDefault->setFormStub(working.outfits.normal.get_form_stub());
            this->ui.outfitSleep->setFormStub(working.outfits.sleeping.get_form_stub());
            this->_update_outfit_contents_view();
            this->ui.inventory->initializeFrom(working.inventory);
            this->ui.gearedUpWeapons->setValue(working.geared_up_weapons);
         }
         break;
      case template_flag::use_scripts:
         //
         // Doesn't copy anything at edit time; the copying happens during play.
         //
         break;
      case template_flag::use_package_overrides:
         {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.packageListDefault),
               QSignalBlocker(this->ui.packageListSpectator),
               QSignalBlocker(this->ui.packageListObserveCorpse),
               QSignalBlocker(this->ui.packageListGuardWarn),
               QSignalBlocker(this->ui.packageListCombat),
            };
            this->ui.packageListDefault->setFormStub(working.ai.default_package_list.get_form_stub());
            this->ui.packageListSpectator->setFormStub(working.ai.package_override_lists.spectator.get_form_stub());
            this->ui.packageListObserveCorpse->setFormStub(working.ai.package_override_lists.observe_corpse.get_form_stub());
            this->ui.packageListGuardWarn->setFormStub(working.ai.package_override_lists.guard_warn.get_form_stub());
            this->ui.packageListCombat->setFormStub(working.ai.package_override_lists.combat.get_form_stub());
         }
         break;
      case template_flag::use_attack_data:
         this->ui.attackData->initializeFrom(working.attack_data);
         break;
      case template_flag::use_keywords:
         this->ui.keywords->pullStubs(working.keywords.forms);
         break;
   }
}

unsigned int FormDialogActorBase::_get_effective_level() const {
   unsigned int level = this->ui.level->value();
   if (this->ui.flagPCLevelMult->isChecked()) {
      level = this->ui.levelCalcMin->value();
      if (level < 1)
         level = 1;
   }
   return level;
}
void FormDialogActorBase::_on_effective_level_changed() {
   auto level = this->_get_effective_level();
   this->_recalc_stats();
}

void FormDialogActorBase::_recalc_stats() {
   auto level = _get_effective_level();

   dovah::loaded_form_ptr<dovah::loaded_forms::Class> loaded_class;
   dovah::loaded_form_ptr<dovah::loaded_forms::Race>  loaded_race;
   if (auto* stub = this->form->stats.combat_class.get_form_stub())
      loaded_class = stub->load().ptr_cast<dovah::loaded_forms::Class>();
   if (auto* stub = this->form->race.get_form_stub())
      loaded_race = stub->load().ptr_cast<dovah::loaded_forms::Race>();

   float health_bonus = 0;
   {
      auto& gss = dovahkit::subsystems::game_settings::core::get();

      dovahkit::subsystems::game_settings::game_setting_value variant;
      if (this->formStub()->formID == dovah::hardcoded_form_ids::Player) {
         health_bonus = 0;
         variant = gss.get_setting_value("fPCHealthLevelBonus");
      } else {
         health_bonus = 5;
         variant = gss.get_setting_value("fNPCHealthLevelBonus");
      }
      if (std::holds_alternative<float>(variant))
         health_bonus = std::get<float>(variant);

      health_bonus *= (level - 1);
   }
   
   auto stats = dovah::compute_classed_stat_points(
      this->formStub()->get_owning_load_order(),
      loaded_class,
      loaded_race,
      level
   );
   stats.attribute_points.base.health       += health_bonus;
   stats.attribute_points.calculated.health += health_bonus;

   auto& working = *this->form;
   {
      working.stats.attributes.offsets.h = this->ui.statsHealthOffset->value();
      working.stats.attributes.offsets.m = this->ui.statsMagickaOffset->value();
      working.stats.attributes.offsets.s = this->ui.statsStaminaOffset->value();
   }

   bool auto_calc = working.actor_flags & (loaded_form_type::actor_flag::auto_calc_stats | loaded_form_type::actor_flag::pc_level_mult);

   auto _set_stats_from = [&working, auto_calc](
      const dovah::classed_stat_points::attribute_trio& attributes,
      const std::array<dovah::classed_stat_points::value_type, dovah::skill_count>& skills
   ) {
      using source_type = dovah::classed_stat_points::value_type;
      {
         auto& stat_set = working.stats.attributes;
         auto& offsets  = stat_set.offsets.list;
         auto& dst      = stat_set.calculated.list;
         using destination_type = std::decay_t<decltype(dst)>::value_type;

         for (size_t i = 0; i < dst.size(); ++i) {
            auto value = attributes.list[i] + offsets[i];
            dst[i] = std::min<source_type>(std::numeric_limits<destination_type>::max(), value);
         }
      }
      {
         auto& stat_set = working.stats.skills;
         auto& offsets  = stat_set.offsets.list;
         auto& dst      = stat_set.calculated.list;
         using destination_type = std::decay_t<decltype(dst)>::value_type;

         for (size_t i = 0; i < dst.size(); ++i) {
            auto value = skills[i];
            if (!auto_calc) // skill offsets are not used when Auto-Calc Stats is active
               value += offsets[i];
            dst[i] = std::min<source_type>(std::numeric_limits<destination_type>::max(), value);
         }
      }
   };

   if (auto_calc) {
      _set_stats_from(stats.attribute_points.calculated, stats.skill_points.calculated);
   } else {
      _set_stats_from(stats.attribute_points.base, stats.skill_points.base);
   }

   this->ui.statsHealthBase->setValue(stats.attribute_points.base.h);
   this->ui.statsMagickaBase->setValue(stats.attribute_points.base.m);
   this->ui.statsStaminaBase->setValue(stats.attribute_points.base.s);
   this->ui.statsHealthCalcFinal->setValue(working.stats.attributes.calculated.list[0]);
   this->ui.statsMagickaCalcFinal->setValue(working.stats.attributes.calculated.list[1]);
   this->ui.statsStaminaCalcFinal->setValue(working.stats.attributes.calculated.list[2]);
   this->_models.skills->setAllData(
      working.stats.skills.offsets.list,
      working.stats.skills.calculated.list
   );
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
void FormDialogActorBase::_push_faction_from_ui() {
   auto* widget    = this->ui.factionsTable;
   auto* model     = this->_models.factions;
   auto* sel_model = widget->selectionModel();

   const ActorBaseFactionsModelNode* node = nullptr;
   size_t row;
   {
      auto rows = sel_model->selectedRows();
      if (!rows.isEmpty()) {
         row  = rows[0].row();
         node = model->item(row);
      }
   }
   if (!node)
      return;

   auto overwrite = *node;
   overwrite.faction = this->ui.currentFactionForm->formStub();
   overwrite.rank    = this->ui.currentFactionRank->value();
   model->overwrite(row, overwrite);
}


void FormDialogActorBase::_set_creature_sound_inherit_actor(dovah::form_stub* stub) {
   {
      auto* const widget  = this->ui.inheritSoundsFrom;
      const auto  blocker = QSignalBlocker(widget);

      auto* prior = widget->formStub();
      widget->setFormStub(stub);
      if (stub) {
         //
         // If `stub` isn't something we can legally inherit from (i.e. if the 
         // filter we've attached to that DKFormPicker rejects it), then use 
         // the prior stub.
         //
         auto* selected = widget->formStub();
         if (stub != selected) {
            widget->setFormStub(prior);
            stub = prior;
         }
      }
   }
   this->form->creature_sounds.set_inherits_from(*this->form, stub);
   this->_creature_sound_inheritance_changed();
}
void FormDialogActorBase::_creature_sound_inheritance_changed() {
   auto* picker       = this->ui.inheritSoundsFrom;
   auto* inherit_from = picker->formStub();

   this->form->creature_sounds.set_inherits_from(*this->form, inherit_from);

   std::vector<dovah::loaded_forms::structs::actor_creature_sounds::entry> entries;

   {
      bool enable = inherit_from == nullptr;

      this->ui.currentCreaSoundType->setEnabled(enable);
      this->ui.currentCreaSoundChance->setEnabled(enable);
      this->ui.currentCreaSoundForm->setEnabled(enable);
      this->ui.buttonCreaSoundAdd->setEnabled(enable);
      this->ui.buttonCreaSoundRemove->setEnabled(enable);
   }

   if (inherit_from) {
      auto loaded = inherit_from->load().ptr_cast<loaded_form_type>();
      if (loaded) {
         entries = loaded->creature_sounds.sounds();
      }
   } else {
      entries = this->form->creature_sounds.sounds();
   }
   if (!entries.empty()) {
      std::vector<ActorBaseCreatureSoundsModelNode> nodes;
      for (auto& src : entries) {
         auto& dst = nodes.emplace_back();
         dst.chance = src.chance;
         dst.type   = src.type;
         dst.sound  = src.sound;
      }

      auto* model = (ActorBaseCreatureSoundsModel*) this->ui.creatureSoundsTable->model();
      model->overwriteAllItems(nodes);
   }
   this->_pull_creature_sound_to_ui();
}
void FormDialogActorBase::_pull_creature_sound_to_ui() {
   const ActorBaseCreatureSoundsModelNode* src = nullptr;
   {
      auto* widget    = this->ui.creatureSoundsTable;
      auto* model     = (ActorBaseCreatureSoundsModel*)widget->model();
      auto* sel_model = widget->selectionModel();
      //
      auto rows = sel_model->selectedRows();
      if (!rows.isEmpty())
         src = model->item(rows[0].row());
   }

   bool enable = src != nullptr && this->ui.inheritSoundsFrom->formStub() == nullptr;

   this->ui.currentCreaSoundType->setEnabled(enable);
   this->ui.currentCreaSoundChance->setEnabled(enable);
   this->ui.currentCreaSoundForm->setEnabled(enable);
   if (!src) {
      return;
   }

   const auto blockers = std::array{
      QSignalBlocker(this->ui.currentCreaSoundType),
      QSignalBlocker(this->ui.currentCreaSoundChance),
      QSignalBlocker(this->ui.currentCreaSoundForm),
   };
   {
      auto i = this->ui.currentCreaSoundType->findData((int)src->type);
      this->ui.currentCreaSoundType->setCurrentIndex(i);
   }
   this->ui.currentCreaSoundChance->setValue(src->chance);
   this->ui.currentCreaSoundForm->setFormStub(src->sound);
}
void FormDialogActorBase::_push_creature_sound_from_ui() {
   if (this->ui.inheritSoundsFrom->formStub()) {
      return;
   }

   auto* widget = this->ui.creatureSoundsTable;
   auto* model  = (ActorBaseCreatureSoundsModel*)widget->model();

   size_t row;
   {
      auto* sel_model = widget->selectionModel();
      //
      auto rows = sel_model->selectedRows();
      if (rows.isEmpty())
         return;
      row = rows[0].row();
   }

   ActorBaseCreatureSoundsModelNode node;
   node.type   = (decltype(ActorBaseCreatureSoundsModelNode::type)) this->ui.currentCreaSoundType->currentData().toInt();
   node.chance = this->ui.currentCreaSoundChance->value();
   node.sound  = this->ui.currentCreaSoundForm->formStub();
   model->overwrite(row, node);
}

void FormDialogActorBase::_pull_tint_layer_to_ui() {
   auto* view      = this->ui.faceTintLayerTable;
   auto* model     = qobject_cast<ActorBaseTintLayerModel*>(view->model());
   auto* sel_model = view->selectionModel();
   assert(model     != nullptr);
   assert(sel_model != nullptr);

   std::optional<ActorBaseTintLayerModel::TintLayer> layer_opt;
   uint16_t layer_index;
   {
      auto rows = sel_model->selectedRows();
      if (!rows.isEmpty()) {
         auto index_opt = model->layerIndex(rows[0].row());
         if (index_opt.has_value()) {
            layer_index = index_opt.value();
            layer_opt   = model->layerDefinitionByIndex(layer_index);
         }
      }
   }
   if (layer_opt.has_value()) {
      const auto& layer_dfn = layer_opt.value();

      this->ui.faceTintColorGroupbox->setEnabled(true);

      const auto blockers = std::array{
         QSignalBlocker(this->ui.faceTintColorCustom),
         QSignalBlocker(this->ui.faceTintColorPreset),
         QSignalBlocker(this->ui.faceTintColorUseCustom),
         QSignalBlocker(this->ui.faceTintColorUsePreset),
         QSignalBlocker(this->ui.faceTintColorInterpolation),
         QSignalBlocker(this->ui.faceTintColorInterpolationSlider),
      };

      bool changes_required = false;
      bool has_custom_color = false;

      this->ui.faceTintColorPreset->clear();
      for (auto& preset : layer_dfn.presets) {
         QString text = QString("%1 at opacity %2");
         if (auto* stub = preset.color.form) {
            text = text.arg(stub->get_editor_id());
         } else {
            text = text.arg(tr("NONE"));
         }
         text = text.arg(preset.alpha);

         this->ui.faceTintColorPreset->addItem(text, (int)preset.index);
      }
      
      auto* state = model->layerStateByIndex(layer_index);
      if (state) {
         has_custom_color = state->preset_index == dovah::index_of_no_face_tint;
         if (!has_custom_color) {
            has_custom_color = true;
            if (auto* preset = layer_dfn.preset_by_index(state->preset_index)) {
               if (preset->color.form) {
                  auto* widget = this->ui.faceTintColorPreset;
                  int   i      = widget->findData((int)state->preset_index);
                  if (i >= 0) {
                     widget->setCurrentIndex(i);
                     this->ui.faceTintColorCustom->setColor(preset->color.cached);
                     has_custom_color = false;
                  }
               }
            }
            changes_required = has_custom_color;
         }
         if (has_custom_color) {
            this->ui.faceTintColorCustom->setColor(state->color);
         }
         this->ui.faceTintColorInterpolation->setValue((float)state->alpha / 100);
         this->ui.faceTintColorInterpolationSlider->setValue((float)state->alpha / 100);
      } else {
         float alpha = 0.0F;

         this->ui.faceTintColorPreset->setCurrentIndex(-1);
         if (!layer_dfn.presets.empty()) {
            auto& preset = layer_dfn.presets[0];
            this->ui.faceTintColorPreset->setCurrentIndex(0);
            this->ui.faceTintColorCustom->setColor(preset.color.cached);
         }
         if (auto* stub = layer_dfn.default_color) {
            auto& list = layer_dfn.presets;
            auto  size = list.size();
            for (size_t i = 0; i < size; ++i) {
               auto& preset = list[i];
               if (preset.color.form == stub) {
                  this->ui.faceTintColorPreset->setCurrentIndex(i);
                  this->ui.faceTintColorCustom->setColor(preset.color.cached);
                  alpha = preset.alpha;
                  break;
               }
            }
         }
         this->ui.faceTintColorInterpolation->setValue(alpha);
         this->ui.faceTintColorInterpolationSlider->setValue(alpha);
      }
      this->ui.faceTintColorUseCustom->setChecked(has_custom_color);
      this->ui.faceTintColorUsePreset->setChecked(!has_custom_color);
      this->ui.faceTintColorCustom->setEnabled(has_custom_color);
      this->ui.faceTintColorPreset->setEnabled(!has_custom_color);

      if (changes_required) {
         this->_push_tint_layer_from_ui();
      }
   } else {
      this->ui.faceTintColorGroupbox->setEnabled(false);
   }
}
void FormDialogActorBase::_push_tint_layer_from_ui() {
   auto* view      = this->ui.faceTintLayerTable;
   auto* model     = qobject_cast<ActorBaseTintLayerModel*>(view->model());
   auto* sel_model = view->selectionModel();
   assert(model     != nullptr);
   assert(sel_model != nullptr);

   uint16_t layer_index;
   {
      auto rows = sel_model->selectedRows();
      if (rows.isEmpty())
         return;
      auto index_opt = model->layerIndex(rows[0].row());
      if (!index_opt.has_value())
         return;
      layer_index = index_opt.value();
   }

   ActorBaseTintLayerModel::TintLayerActorState state;
   state.layer_index = layer_index;
   state.alpha       = this->ui.faceTintColorInterpolation->value() * 100;
   state.color       = this->ui.faceTintColorCustom->color();
   if (this->ui.faceTintColorUsePreset->isChecked()) {
      state.preset_index = this->ui.faceTintColorPreset->currentData().toInt();
   } else {
      state.preset_index = dovah::index_of_no_face_tint;
   }
   model->importLayerState(state);
}

dovah::sex FormDialogActorBase::_current_sex() const {
   return (dovah::sex)this->ui.sex->currentData().toInt();
}

void FormDialogActorBase::_set_pc_level_mult(bool flag) {
   bool prior = this->form->actor_flags & loaded_form_type::actor_flag::pc_level_mult;
   if (flag) {
      this->form->actor_flags |= loaded_form_type::actor_flag::pc_level_mult;
   } else {
      this->form->actor_flags &= ~loaded_form_type::actor_flag::pc_level_mult;
   }
   
   {
      auto* const widget  = this->ui.flagPCLevelMult;
      const auto  blocker = QSignalBlocker(widget);
      widget->setChecked(flag);
   }
   {
      auto* const widget  = this->ui.level;
      const auto  blocker = QSignalBlocker(widget);
      if (flag) {
         widget->setDecimals(3);
         widget->setRange(0.001, 32.767);
         if (flag != prior) {
            this->form->stats.level = 1000;
         }
         widget->setValue((float)this->form->stats.level / 1000);
      } else {
         widget->setDecimals(0);
         widget->setRange(1, 32767);
         if (flag != prior) {
            this->form->stats.level = this->form->stats.calc_min_level;
         }
         widget->setValue(this->form->stats.level);
      }
   }

   this->ui.flagAutoCalcStats->setEnabled(!flag);
   if (flag) {
      this->ui.flagAutoCalcStats->setChecked(true);
   }

   if (flag != prior)
      this->_on_effective_level_changed();
}
void FormDialogActorBase::_set_race(dovah::form_stub* race) {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.baseHeadPartPicker),
      QSignalBlocker(this->ui.race),
      QSignalBlocker(this->ui.sex),
   };

   bool changed = true;
   if (this->form)
      changed = this->form->race != race;

   this->ui.race->setFormStub(race);
   if (this->form) {
      write_form_ref(this->form->race, race);
   }

   this->_filters.face.base_head_part->setRequiredRace(race);
   this->_filters.face.complexion->setRequiredRace(race);
   this->_filters.face.hair_color->setRequiredRace(race);

   if (!race) {
      return;
   }

   auto loaded = race->load().ptr_cast<dovah::loaded_forms::Race>();
   this->_recalc_stats();
   {  // Face Parts
      if (loaded) {
         //
         // Tint layers:
         //
         this->_models.face_tints->importLayerDefinitions(*loaded);
         this->_pull_tint_layer_to_ui();
         //
         // Head parts:
         //
         this->_models.head_parts_base->filterForRace(race);
         this->_models.head_parts_extra->filterForRace(race);
      }
   }
   {  // Indexed face morphs
      if (loaded) {
         size_t prior_eyes = 0;
         size_t prior_lips = 0;
         size_t prior_nose = 0;
         if (!changed) {
            prior_eyes = this->form->facegen.morphs.indices.eyes;
            prior_lips = this->form->facegen.morphs.indices.lips;
            prior_nose = this->form->facegen.morphs.indices.nose;
         }
         this->ui.faceMorphEyesIndex->clear();
         this->ui.faceMorphMouthIndex->clear();
         this->ui.faceMorphNoseIndex->clear();

         auto& src = loaded->by_sex[this->_current_sex()].head_data.morphs;
         auto& gss = dovahkit::subsystems::game_settings::core::get();

         {
            size_t morph_count = 10;
            {
               auto variant = gss.get_setting_value("iEyeMorphCount");
               if (std::holds_alternative<int32_t>(variant))
                  morph_count = std::get<int32_t>(variant);
            }
            auto* widget = this->ui.faceMorphEyesIndex;
            for (size_t i = 0; i < morph_count; ++i) {
               bool enabled = src.eyes.test(i);
               if (enabled)
                  widget->addItem(QString("EyeType%1").arg(i), i);
            }
            if (prior_eyes != -1) {
               auto i = widget->findData(prior_eyes);
               if (i >= 0)
                  widget->setCurrentIndex(i);
               else
                  widget->setCurrentIndex(0);
            }
         }
         {
            size_t morph_count = 10;
            {
               auto variant = gss.get_setting_value("iLipMorphCount");
               if (std::holds_alternative<int32_t>(variant))
                  morph_count = std::get<int32_t>(variant);
            }
            auto* widget = this->ui.faceMorphMouthIndex;
            for (size_t i = 0; i < morph_count; ++i) {
               bool enabled = src.mouths.test(i);
               if (enabled)
                  widget->addItem(QString("LipType%1").arg(i), i);
            }
            if (prior_lips != -1) {
               auto i = widget->findData(prior_lips);
               if (i >= 0)
                  widget->setCurrentIndex(i);
               else
                  widget->setCurrentIndex(0);
            }
         }
         {
            size_t morph_count = 10;
            {
               auto variant = gss.get_setting_value("iNoseMorphCount");
               if (std::holds_alternative<int32_t>(variant))
                  morph_count = std::get<int32_t>(variant);
            }
            auto* widget = this->ui.faceMorphNoseIndex;
            for (size_t i = 0; i < morph_count; ++i) {
               bool enabled = src.noses.test(i);
               if (enabled)
                  widget->addItem(QString("NoseType%1").arg(i), i);
            }
            if (prior_nose != -1) {
               auto i = widget->findData(prior_nose);
               if (i >= 0)
                  widget->setCurrentIndex(i);
               else
                  widget->setCurrentIndex(0);
            }
         }
      } else {
         this->ui.faceMorphEyesIndex->clear();
         this->ui.faceMorphMouthIndex->clear();
         this->ui.faceMorphNoseIndex->clear();
      }
   }
   {  // Tab and preview-option visibility
      bool show = false;
      if (loaded) {
         show = loaded->race_flags & dovah::loaded_forms::Race::race_flag::facegen_head;
      }
      this->ui.tabbox->setTabVisible(this->ui.tabbox->indexOf(this->ui.tabFaceMorphs), show);
      this->ui.tabbox->setTabVisible(this->ui.tabbox->indexOf(this->ui.tabFaceParts), show);
      if constexpr (preview_enabled) {
         this->ui.tabbox->setTabVisible(this->ui.tabbox->indexOf(this->ui.tabFaceAnimPreview), show);

         this->ui.previewTypeHead->setEnabled(show);
         if (!show)
            this->ui.previewTypeFull->setChecked(true);
      } else {
         this->ui.tabbox->setTabVisible(this->ui.tabbox->indexOf(this->ui.tabFaceAnimPreview), false);
      }
   }
}
void FormDialogActorBase::_set_sex(dovah::sex s) {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.baseHeadPartPicker),
      QSignalBlocker(this->ui.faceComplexion),
      QSignalBlocker(this->ui.hairColor),
      QSignalBlocker(this->ui.sex),
   };
   
   {
      auto* widget = this->ui.sex;
      auto  i      = widget->findData((int)s);
      if (i >= 0)
         widget->setCurrentIndex(i);
   }
   if (this->form) {
      auto& flags = this->form->actor_flags;
      if (s == dovah::sex::female)
         this->form->actor_flags |= loaded_form_type::actor_flag::female;
      else
         this->form->actor_flags &= ~loaded_form_type::actor_flag::female;
   }
   this->_models.relationships->setFocusActor(this->formStub(), s);

   this->_filters.voicetype->set_female(s == dovah::sex::female);
   {  // Face Parts
      this->_filters.face.base_head_part->setRequiredSex(s);
      this->_filters.face.complexion->setRequiredSex(s);
      this->_filters.face.hair_color->setRequiredSex(s);

      this->_models.face_tints->setSex(s);
      this->_pull_tint_layer_to_ui();

      this->_models.head_parts_base->filterForSex(s);
      this->_models.head_parts_extra->filterForSex(s);
   }
}

void FormDialogActorBase::_pull_headparts_to_ui() {
   auto* model_base  = this->_models.head_parts_base;
   auto* model_extra = this->_models.head_parts_extra;

   auto& fic = dovahkit::subsystems::form_info_cache::core::get();

   using base_slot = FaceBaseHeadPartsModel::Slot;

   std::vector<dovah::form_stub*> extra_parts;

   // Used to strip out duplicates on load.
   auto _head_part_already_seen = [&extra_parts, model_base](const dovah::form_stub* stub) {
      if (stub == model_base->headPartFor(base_slot::Brows))
         return true;
      if (stub == model_base->headPartFor(base_slot::Eyes))
         return true;
      if (stub == model_base->headPartFor(base_slot::Face))
         return true;
      if (stub == model_base->headPartFor(base_slot::FacialHair))
         return true;
      if (stub == model_base->headPartFor(base_slot::Hair))
         return true;

      auto it = std::find(extra_parts.begin(), extra_parts.end(), stub);
      if (it != extra_parts.end())
         return true;

      return false;
   };

   for (auto& form_use : this->form->facegen.head_parts) {
      auto* stub = form_use.get_form_stub();
      if (stub && stub->form_type == dovah::form_type::head_part) {
         if (_head_part_already_seen(stub))
            continue;
         auto* info = fic.get_head_part_info(*stub);
         if (!info) {
            extra_parts.push_back(stub);
            continue;
         }
         switch (info->type) {
            case dovah::head_part_type::eyebrows:
               model_base->setHeadPartFor(base_slot::Brows, stub);
               continue;
            case dovah::head_part_type::eyes:
               model_base->setHeadPartFor(base_slot::Eyes, stub);
               continue;
            case dovah::head_part_type::face:
               model_base->setHeadPartFor(base_slot::Face, stub);
               continue;
            case dovah::head_part_type::facial_hair:
               model_base->setHeadPartFor(base_slot::FacialHair, stub);
               continue;
            case dovah::head_part_type::hair:
               model_base->setHeadPartFor(base_slot::Hair, stub);
               continue;
         }
         extra_parts.push_back(stub);
      }
   }
   model_extra->replaceAllHeadParts(extra_parts);
}

/*virtual*/ bool FormDialogActorBase::eventFilter(QObject* object, QEvent* event) /*override*/ {
   auto* widget = this->ui.additionalHeadParts;
   if (object == widget) {
      if (event->type() == QEvent::Type::KeyPress) {
         auto* casted = (QKeyEvent*)event;
         if (casted->key() == Qt::Key_Delete) {

            auto* sel_model = widget->selectionModel();
            auto* model     = this->_models.head_parts_extra;
            {
               auto rows = sel_model->selectedRows();
               if (!rows.isEmpty()) {
                  auto  row  = rows[0].row();
                  auto* stub = model->headPart(row);
                  if (stub)
                     model->removeHeadPart(*stub);
               }
            }

            return true;
         }
         return false;
      }
   }
   return false;
}