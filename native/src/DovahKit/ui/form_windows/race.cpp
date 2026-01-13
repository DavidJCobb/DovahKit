#include "./race.h"
#include <limits>
#include "helpers/math/rotation/unit_conversion.h"
#include "dovah/data/face_fx/default_facegen_race_phonemes.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/data/skills.h"
#include "dovah/forms/MovementType.h"
#include "dovah/core.h"
#include "editor/helpers/face_fx_phoneme_name.h"
#include "editor/helpers/skill_name_to_string.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/head_part.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/enum_dropdown_configs/skill.h"
#include "ui/utils/bind.h"
#include "ui/utils/get_selection.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"

#include "./shared/FaceBaseHeadPartsModel.h"
#include "./shared/FaceExtraHeadPartsModel.h"
#include "./shared/FormPickerFromFormListPaneFilter.h"
#include "./shared/HeadPartPickerFilter.h"
#include "./race/RaceAvailableFaceMorphsModel.h"
#include "./race/RaceBaseMovementDefaultsModel.h"
#include "./race/RaceBipedObjectSlotsModel.h"
#include "./race/RaceEquipSlotsModel.h"
#include "./race/RaceEquipTypesModel.h"
#include "./race/RacePhonemeMorphsModel.h"
#include "./race/RacePresetActorFormFilter.h"
#include "./race/RaceTintDefaultColorPickerFilter.h"
#include "./race/RaceTintLayerModel.h"

namespace {
   constexpr const bool allow_del_key_on_tint_layer_list = false;
}

FormDialogRace::FormDialogRace(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   auto& editor = DovahKitCore::get();

   {
      auto& dst = this->_subwidgets.sex_tabboxes;
      dst.body       = this->ui.bodyBySex;
      dst.face_data  = this->ui.faceBySex;
      dst.face_tints = this->ui.tintsBySex;

      for (auto* widget : dst.all) {
         widget->setCurrentIndex(0);
      }
   }
   this->ui.tabbox->setCurrentIndex(0);

   #pragma region General tab
      // Inherit
      this->ui.morphRace->setAllowedFormType(dovah::form_type::race);
      this->ui.armorRace->setAllowedFormType(dovah::form_type::race);

      // Miscellaneous
      {
         auto* widget = this->ui.creatureSize;
         widget->clear();
         widget->addItem(tr("Small"),      (int)loaded_form_type::creature_size::small);
         widget->addItem(tr("Medium"),     (int)loaded_form_type::creature_size::medium);
         widget->addItem(tr("Large"),      (int)loaded_form_type::creature_size::large);
         widget->addItem(tr("Very Large"), (int)loaded_form_type::creature_size::extra_large);
      }
      ui::set_unsigned_range<float>(this->ui.flightRadius);
      ui::set_unsigned_range<float>(this->ui.baseCarryCapacity);

      QObject::connect(this->ui.flagFaceGenHead, &QCheckBox::toggled, this, [this](bool checked) {
         auto* tabbox    = this->ui.tabbox;
         auto* tab_parts = this->ui.tabFace;
         auto* tab_tints = this->ui.tabTints;

         tabbox->setTabVisible(tabbox->indexOf(tab_parts), checked);
         tabbox->setTabVisible(tabbox->indexOf(tab_tints), checked);
      });
      {  // Update visible/enable states.
         auto* widget  = this->ui.flagFaceGenHead;
         bool  checked = widget->isChecked();
         emit widget->toggled(checked);
      }
      
      #pragma region Skill boost widgets
         this->_subwidgets.skills.which = {
            this->ui.skillBonus01Skill,
            this->ui.skillBonus02Skill,
            this->ui.skillBonus03Skill,
            this->ui.skillBonus04Skill,
            this->ui.skillBonus05Skill,
            this->ui.skillBonus06Skill,
            this->ui.skillBonus07Skill,
         };
         this->_subwidgets.skills.boost = {
            this->ui.skillBonus01Value,
            this->ui.skillBonus02Value,
            this->ui.skillBonus03Value,
            this->ui.skillBonus04Value,
            this->ui.skillBonus05Value,
            this->ui.skillBonus06Value,
            this->ui.skillBonus07Value,
         };
         for (auto* widget : this->_subwidgets.skills.which) {
            ui::enum_dropdown_configs::skill(widget, true, false);
         }
         for (auto* widget : this->_subwidgets.skills.boost) {
            ui::set_range<loaded_form_type::skill_boost_value_type>(widget);
         }
      #pragma endregion

      // Attributes
      ui::set_unsigned_range<float>(this->ui.attrBaseH);
      ui::set_unsigned_range<float>(this->ui.attrBaseM);
      ui::set_unsigned_range<float>(this->ui.attrBaseS);
      ui::set_unsigned_range<float>(this->ui.attrRegenH);
      ui::set_unsigned_range<float>(this->ui.attrRegenM);
      ui::set_unsigned_range<float>(this->ui.attrRegenS);

      // Spells
      this->ui.abilities->setAllowedFormTypes({ dovah::form_type::spell, dovah::form_type::leveled_spell, dovah::form_type::shout });

      ui::set_range<float>(this->ui.mountOffsetX);
      ui::set_range<float>(this->ui.mountOffsetY);
      ui::set_range<float>(this->ui.mountOffsetZ);
      ui::set_range<float>(this->ui.dismountOffsetX);
      ui::set_range<float>(this->ui.dismountOffsetY);
      ui::set_range<float>(this->ui.dismountOffsetZ);
      ui::set_range<float>(this->ui.mountCamOffsetX);
      ui::set_range<float>(this->ui.mountCamOffsetY);
      ui::set_range<float>(this->ui.mountCamOffsetZ);
   #pragma endregion
   #pragma region Keywords tab
      this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   #pragma endregion
   #pragma region Body tab
      ui::set_unsigned_range<float>(this->ui.mass);
      this->ui.bodyPartData->setAllowedFormType(dovah::form_type::body_part_data);
      this->ui.skin->setAllowedFormType(dovah::form_type::armor);

      ui::set_unsigned_range<float>(this->ui.heightMultF);
      ui::set_unsigned_range<float>(this->ui.heightMultM);
      this->ui.bodyWeightF->setRange(0, 100);
      this->ui.bodyWeightM->setRange(0, 100);
      this->ui.decapArmorF->setAllowedFormType(dovah::form_type::armor);
      this->ui.decapArmorM->setAllowedFormType(dovah::form_type::armor);
      this->ui.voicetypeF->setAllowedFormType(dovah::form_type::voicetype);
      this->ui.voicetypeM->setAllowedFormType(dovah::form_type::voicetype);
      this->ui.voicetypeF->setDefaultForm(editor.get_form_of_probable_type(dovah::form_type::voicetype, dovah::hardcoded_form_ids::AdultFemaleVoice1));
      this->ui.voicetypeM->setDefaultForm(editor.get_form_of_probable_type(dovah::form_type::voicetype, dovah::hardcoded_form_ids::AdultMaleVoice1));
      this->ui.voicetypeF->setAllowNone(false);
      this->ui.voicetypeM->setAllowNone(false);

      this->_update_slot_dropdowns();
      {  // Biped object slot table
         auto* view  = this->ui.bipedSlotDefinitions;
         auto* model = this->_models.biped_objects = new RaceBipedObjectSlotsModel(this);
         view->setModel(model);
         auto* sel_model = view->selectionModel();

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(view, [model](DKHeaderView& header, const QFontMetrics& metrics) {
            auto col_header = model->headerData(RaceBipedObjectSlotsModel::Column::IsFirstPerson, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            header.setColumnFlex(RaceBipedObjectSlotsModel::Column::Name,          1, 0);
            header.setColumnFlex(RaceBipedObjectSlotsModel::Column::IsFirstPerson, 0, 0, metrics.boundingRect(col_header).width() * 1.5F + 4);
         });

         auto* name_edit = this->ui.bipedObjectSlotNameEdit;
         name_edit->setEnabled(false);
         name_edit->setMaxLength(loaded_form_type::max_biped_object_name_length);
         QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, model, name_edit](const QItemSelection& sel) {
            const auto blocker = QSignalBlocker(name_edit);
            if (sel.empty() || sel[0].isEmpty()) {
               name_edit->setEnabled(false);
               name_edit->setText("");
               return;
            }
            auto qmi = QModelIndex(sel[0].topLeft()).siblingAtColumn(RaceBipedObjectSlotsModel::Column::Name);
            name_edit->setEnabled(true);
            name_edit->setText(model->data(qmi, Qt::DisplayRole).toString());
         });
         QObject::connect(name_edit, &QLineEdit::textChanged, this, [this, model, sel_model](QString text) {
            auto sel = sel_model->selectedRows();
            if (sel.empty())
               return;
            auto qmi = sel[0].siblingAtColumn(RaceBipedObjectSlotsModel::Column::Name);
            model->setData(qmi, text, Qt::UserRole);
         });
      }
   #pragma endregion
   #pragma region Blood tab
      this->ui.impactMaterialType->setAllowedFormType(dovah::form_type::material_type);
      this->ui.decapBloodArt->setAllowedFormType(dovah::form_type::art_object);
      this->ui.impactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
      this->ui.soundOpen->setAllowedFormType(dovah::form_type::sound_descriptor);
      this->ui.soundClose->setAllowedFormType(dovah::form_type::sound_descriptor);
   #pragma endregion
   #pragma region Text tab
      //
      // No setup needed at this time.
      //
   #pragma endregion
   #pragma region Movement Details tab
      ui::set_unsigned_range<float>(this->ui.accelerationRate);
      ui::set_unsigned_range<float>(this->ui.decelerationRate);
      ui::set_unsigned_range<float>(this->ui.angularAccelerationRate);
      ui::set_unsigned_range<float>(this->ui.angularTolerance);
      {  // Base Movement Defaults
         auto* view  = this->ui.movementTypeList;
         auto* model = this->_models.base_movement_types = new RaceBaseMovementDefaultsModel(this);
         view->setModel(model);
         auto* sel_model = view->selectionModel();
         auto* picker    = this->ui.movementTypePicker;

         picker->setAllowedFormType(dovah::form_type::movement_type);

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(RaceBaseMovementDefaultsModel::Column::Type, 0, 0, metrics.boundingRect("   ").width() * 1.5F + 4);
            header.setColumnFlex(RaceBaseMovementDefaultsModel::Column::Form, 1, 0, metrics.boundingRect("SomeCoolMovementType").width() * 1.5F + 4);
         });
         QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, model, picker](const QItemSelection& selection) {
            if (selection.isEmpty() || selection[0].isEmpty()) {
               picker->setEnabled(false);
               return;
            }
            picker->setEnabled(true);

            auto  qmi  = selection[0].topLeft();
            auto* stub = model->form(qmi.row());

            const auto blocker = QSignalBlocker(picker);
            picker->setFormStub(stub);
         });
         QObject::connect(picker, &DKFormPicker::formChanged, this, [this, model, sel_model](dovah::form_stub* stub) {
            auto sel = sel_model->selectedRows();
            if (sel.isEmpty())
               return;
            model->setForm(sel[0].row(), stub);
         });
         picker->setEnabled(false);
      }
      {  // Movement Data Overrides
         this->_subwidgets.movement_data_override = {
            .form_pane = this->ui.movementTypeOverrideForms,
            .values = {
               this->ui.movementTypeOverrideWalkLeft,
               this->ui.movementTypeOverrideRunLeft,
               this->ui.movementTypeOverrideWalkRight,
               this->ui.movementTypeOverrideRunRight,
               this->ui.movementTypeOverrideWalkForward,
               this->ui.movementTypeOverrideRunForward,
               this->ui.movementTypeOverrideWalkBack,
               this->ui.movementTypeOverrideRunBack,
               this->ui.movementTypeOverrideWalkRotate,
               this->ui.movementTypeOverrideRunRotate,
               nullptr
            }
         };

         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideWalkForward);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideWalkBack);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideWalkLeft);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideWalkRight);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideRunForward);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideRunBack);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideRunLeft);
         ui::set_unsigned_range<float>(this->ui.movementTypeOverrideRunRight);
         this->ui.movementTypeOverrideWalkRotate->setRange(0, 360);
         this->ui.movementTypeOverrideRunRotate->setRange(0, 360);

         auto* listview = this->ui.movementTypeOverrideForms;
         listview->setAllowedFormTypes({ dovah::form_type::movement_type });
         listview->setAllowDuplicates(false);
         listview->setAllowMultiSelect(false);
         QObject::connect(listview, &DKFormListPane::selectedFormsChanged, this, [this](const std::vector<dovah::form_stub*>& stubs) {
            if (stubs.empty()) {
               this->_pull_movement_type_overrides_to_ui(nullptr);
               return;
            }
            this->_pull_movement_type_overrides_to_ui(stubs[0]);
         });
         QObject::connect(listview, &DKFormListPane::formsAdded, this, [this, listview]() {
            if (this->_state.filling_movement_type_list)
               return;
            auto& working = *this->form;

            auto  src = listview->stubs();
            auto& dst = working.movement.overrides;
            for (auto* stub : src) {
               bool found = false;
               for (const auto& item : dst) {
                  if (item.type == stub) {
                     found = true;
                     break;
                  }
               }
               if (!found) {
                  auto& item = dst.emplace_back();
                  item.type.set(working, stub);

                  auto loaded = stub->load().ptr_cast<dovah::loaded_forms::MovementType>();
                  if (loaded) {
                     item.speeds = loaded->speeds;
                  }
               }
            }
         });
         QObject::connect(listview, &DKFormListPane::formsRemoved, this, [this, listview]() {
            if (this->_state.filling_movement_type_list)
               return;
            auto& working = *this->form;

            auto& list = working.movement.overrides;
            bool  any  = false;
            for (auto& item : list) {
               if (listview->contains(item.type.get_form_stub()))
                  continue;
               item.type.set(working, nullptr);
               any = true;
            }
            std::erase_if(list, [](const auto& item) -> bool {
               return item.type == nullptr;
            });
         });

         for (auto* widget : this->_subwidgets.movement_data_override.values)
            QObject::connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormDialogRace::_push_movement_type_overrides_to_form);

         _pull_movement_type_overrides_to_ui(nullptr); // set initial enable states
      }
   #pragma endregion
   #pragma region Attack Data tab
      //
      // Just the one premade widget. It'll set itself up for us.
      //
   #pragma endregion
   #pragma region Combat tab
      ui::set_unsigned_range<float>(this->ui.injuredHealthPercentage);
      ui::set_unsigned_range<float>(this->ui.unarmedDamage);
      ui::set_unsigned_range<float>(this->ui.unarmedReach);
      this->ui.unarmedEquipSlot->setAllowedFormType(dovah::form_type::equip_slot);
      ui::set_unsigned_range<float>(this->ui.aimAngleTolerance);
      {
         auto* view  = this->ui.equipSlots;
         auto* model = this->_models.equip_slots = new RaceEquipSlotsModel(this);
         view->setModel(model);

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         view->horizontalHeader()->setStretchLastSection(true);
      }
      {
         auto* view  = this->ui.equipTypes;
         auto* model = this->_models.equip_types = new RaceEquipTypesModel(this);
         view->setModel(model);

         ui::typical_tableview_config(view);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         view->horizontalHeader()->setStretchLastSection(true);
      }
   #pragma endregion
   #pragma region Lip Synching tab
      QObject::connect(this->ui.flagFaceGenHead, &QCheckBox::toggled, this, [this](bool checked) {
         auto* widget = this->ui.flagDefaultFaceGenPhonemes;
         widget->setEnabled(checked);
         if (!checked)
            widget->setChecked(false);
      });
      this->ui.flagDefaultFaceGenPhonemes->setEnabled(false);
      QObject::connect(this->ui.flagDefaultFaceGenPhonemes, &QCheckBox::toggled, this, &FormDialogRace::_set_phonemes_are_default);

      {
         auto* widget = this->ui.phonemeFaceFXList;
         for (size_t i = 0; i < dovah::face_fx::phoneme_count; ++i) {
            widget->addItem(editor_helpers::face_fx_phoneme_name((dovah::face_fx::phoneme)i));
         }
         QObject::connect(widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormDialogRace::_on_selected_phoneme_changed);
      }
      {
         auto* phoneme_picker = this->ui.phonemeFaceFXList;
         auto* target_view    = this->ui.phonemeTargets;

         auto* model = this->_models.current_phoneme_morphs = new RacePhonemeMorphsModel(this);
         target_view->setModel(model);

         ui::typical_tableview_config(target_view);
         target_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
         ui::set_tableview_column_flex(target_view, [](DKHeaderView& header, const QFontMetrics& metrics) {
            header.setColumnFlex(RacePhonemeMorphsModel::Column::Name,   1, 0, metrics.boundingRect("RoflLmao").width() * 1.5F + 4);
            header.setColumnFlex(RacePhonemeMorphsModel::Column::Weight, 0, 0, metrics.boundingRect("1.000000").width() * 1.5F + 4);
         });

         QObject::connect(target_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormDialogRace::_pull_phoneme_target_to_ui);

         QObject::connect(this->ui.buttonPhonemeMorphNew,    &QPushButton::clicked, this, &FormDialogRace::_add_phoneme_target);
         QObject::connect(this->ui.buttonPhonemeMorphDelete, &QPushButton::clicked, this, &FormDialogRace::_remove_phoneme_target);
         //
         QObject::connect(this->ui.currentPhonemeMorphName, &QLineEdit::textChanged, this, &FormDialogRace::_push_phoneme_target_from_ui);
         QObject::connect(this->ui.currentPhonemeWeight,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FormDialogRace::_push_phoneme_target_from_ui);
      }
   #pragma endregion
   #pragma region Face Data tab
      {  // Base Head Parts
         auto _configure = [this](
            dovah::sex    sex,
            QTableView*   view,
            DKFormPicker* picker
         ) {
            auto* model  = this->_models.head_parts_base[sex] = new FaceBaseHeadPartsModel(this);
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

            auto* filter = this->_filters.base_head_part[sex] = new HeadPartPickerFilter(this);
            picker->setAllowedFormType(dovah::form_type::head_part);
            picker->setCustomFilter(filter);
            filter->setRequiredRace(this->formStub());
            filter->setRequiredSex(sex);

            auto* sel_model = view->selectionModel();
            QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, view, model, sel_model, picker, filter]() {
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
               filter->setRequiredType(FaceBaseHeadPartsModel::slotToType(slot.value()));
               picker->setFormStub(stub);
            });
            QObject::connect(picker, &DKFormPicker::formChanged, this, [this, sel_model, model, picker](dovah::form_stub* stub) {
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
         };
         _configure(dovah::sex::female, this->ui.baseHeadPartsF, this->ui.baseHeadPartPickerF);
         _configure(dovah::sex::male,   this->ui.baseHeadPartsM, this->ui.baseHeadPartPickerM);
      }
      {  // Additional Head Parts
         auto _configure = [this](dovah::sex sex, QTableView* view) {
            this->_subwidgets.head_parts.extra[sex] = view;

            auto* model = this->_models.head_parts_extra[sex] = new FaceExtraHeadPartsModel(this);
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
         };
         _configure(dovah::sex::female, this->ui.extraHeadPartsF);
         _configure(dovah::sex::male,   this->ui.extraHeadPartsM);
      }
      {  // Available morphs
         auto _configure = [this](
            dovah::sex sex,
            QTableView* view
         ) {
            auto* model = this->_models.available_face_morphs[sex] = new RaceAvailableFaceMorphsModel(this);
            view->setModel(model);

            ui::typical_tableview_config(view);
            view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
            view->horizontalHeader()->setStretchLastSection(true);
         };
         _configure(dovah::sex::female, this->ui.availableMorphsF);
         _configure(dovah::sex::male,   this->ui.availableMorphsM);
      }
      {  // Hair Colors
         auto _configure = [this](
            dovah::sex sex,
            DKFormListPane* available_hair_colors,
            DKFormPicker*   default_hair_color
         ) {
            available_hair_colors->setAllowedFormTypes({ dovah::form_type::color });
            default_hair_color->setAllowedFormType(dovah::form_type::color);

            auto* filter = this->_filters.default_hair_color[sex] = new FormPickerFromFormListPaneFilter(this);
            filter->setPane(available_hair_colors);
            default_hair_color->setCustomFilter(filter);
         };

         _configure(
            dovah::sex::female,
            this->ui.hairColorsF,
            this->ui.defaultHairColorF
         );
         _configure(
            dovah::sex::male,
            this->ui.hairColorsM,
            this->ui.defaultHairColorM
         );
      }
   #pragma endregion
   #pragma region Face Tints tab
      {
         this->_subwidgets.tints[dovah::sex::female] = {
            .layers = {
               .buttons = {
                  .create    = this->ui.buttonTintsNewF,
                  .move_up   = this->ui.buttonTintsMoveUpF,
                  .move_down = this->ui.buttonTintsMoveDownF,
                  .remove    = this->ui.buttonTintsDeleteF,
               },
               .view = this->ui.tintsTableF,
               .edit = {
                  .container     = this->ui.layoutTintEditF,
                  .texture       = this->ui.currentTintFTexture,
                  .type          = this->ui.currentTintFType,
                  .default_color = this->ui.currentTintFDefaultColor,
               },
            },
            .presets = {
               .buttons = {
                  .create    = this->ui.buttonTintPresetFNew,
                  .move_up   = this->ui.buttonTintPresetFMoveUp,
                  .move_down = this->ui.buttonTintPresetFMoveDown,
                  .remove    = this->ui.buttonTintPresetFDelete,
               },
               .view = this->ui.currentTintFPresetList,
               .edit = {
                  .color = this->ui.currentTintFPresetColor,
                  .alpha = {
                     .slider  = this->ui.currentTintFPresetAlphaSlider,
                     .spinbox = this->ui.currentTintFPresetAlpha,
                  },
               },
            },
         };
         this->_subwidgets.tints[dovah::sex::male] = {
            .layers = {
               .buttons = {
                  .create    = this->ui.buttonTintsNewM,
                  .move_up   = this->ui.buttonTintsMoveUpM,
                  .move_down = this->ui.buttonTintsMoveDownM,
                  .remove    = this->ui.buttonTintsDeleteM,
               },
               .view = this->ui.tintsTableM,
               .edit = {
                  .container     = this->ui.layoutTintEditM,
                  .texture       = this->ui.currentTintMTexture,
                  .type          = this->ui.currentTintMType,
                  .default_color = this->ui.currentTintMDefaultColor,
               },
            },
            .presets = {
               .buttons = {
                  .create    = this->ui.buttonTintPresetMNew,
                  .move_up   = this->ui.buttonTintPresetMMoveUp,
                  .move_down = this->ui.buttonTintPresetMMoveDown,
                  .remove    = this->ui.buttonTintPresetMDelete,
               },
               .view = this->ui.currentTintMPresetList,
               .edit = {
                  .color = this->ui.currentTintMPresetColor,
                  .alpha = {
                     .slider  = this->ui.currentTintMPresetAlphaSlider,
                     .spinbox = this->ui.currentTintMPresetAlpha,
                  },
               },
            },
         };

         auto _configure = [this](dovah::sex sex) {
            auto& sw = this->_subwidgets.tints[sex];

            auto* layer_table  = sw.layers.view;
            auto* preset_table = sw.presets.view;

            auto* model = this->_models.tint_layer_model[sex] = new RaceTintLayerModel(this);
            layer_table->setModel(model);

            // Support for the Del key:
            layer_table->installEventFilter(this);
            preset_table->installEventFilter(this);

            // Buttons for adding/moving/removing list items:
            {  // Layer buttons
               const auto& btn = sw.layers.buttons;
               QObject::connect(btn.move_up, &QPushButton::clicked, this, [this, model, sex]() {
                  model->move_layer(this->_selected_tint_layer(sex), false);
               });
               QObject::connect(btn.move_down, &QPushButton::clicked, this, [this, model, sex]() {
                  model->move_layer(this->_selected_tint_layer(sex), true);
               });
               QObject::connect(btn.create, &QPushButton::clicked, this, [this, sex]() {
                  this->_add_new_tint_layer(sex);
               });
               QObject::connect(btn.remove, &QPushButton::clicked, this, [this, model, sex]() {
                  model->remove_layer(this->_selected_tint_layer(sex));
               });
            }
            {  // Preset buttons
               const auto& btn = sw.presets.buttons;
               QObject::connect(btn.move_up, &QPushButton::clicked, this, [this, sex, model]() {
                  model->move_preset(this->_selected_tint_preset(sex), false);
               });
               QObject::connect(btn.move_down, &QPushButton::clicked, this, [this, sex, model]() {
                  model->move_preset(this->_selected_tint_preset(sex), true);
               });
               QObject::connect(btn.create, &QPushButton::clicked, this, [this, sex]() {
                  this->_add_new_tint_preset(sex);
               });
               QObject::connect(btn.remove, &QPushButton::clicked, this, [this, sex, model]() {
                  model->remove_preset(this->_selected_tint_preset(sex));
               });
            }

            auto* preset_model = this->_models.tint_preset_model[sex] = new RaceTintLayerPresetsModel(this);
            preset_model->setSourceModel(model);
            preset_table->setModel(preset_model);
            preset_table->setRootIndex(preset_model->mapFromSource(model->noPresetQMI()));

            // Initial enable states:
            sw.layers.edit.container->setEnabled(false);
            sw.presets.edit.color->setEnabled(false);
            sw.presets.edit.alpha.spinbox->setEnabled(false);
            sw.presets.edit.alpha.slider->setEnabled(false);

            sw.presets.edit.color->setAllowedFormType(dovah::form_type::color);
            // TODO: Can the color of a preset be nullptr? Should we allowNone?

            {  // Layer table
               ui::typical_tableview_config(layer_table);
               layer_table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
               ui::set_tableview_column_flex(layer_table, [](DKHeaderView& header, const QFontMetrics& metrics) {
                  header.setColumnFlex(RaceTintLayerModel::LayerColumn::TexturePath, 3, 0);
                  header.setColumnFlex(RaceTintLayerModel::LayerColumn::NumPresets,  0, 0, metrics.boundingRect("99").width() * 1.5F + 4);
                  header.setColumnFlex(RaceTintLayerModel::LayerColumn::Type,        1, 0, metrics.boundingRect("Cheek Color Upper").width() * 1.5F + 4);
               });
               QObject::connect(layer_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, sex](const QItemSelection& sel) {
                  this->_on_tint_layer_selection_changed(sex, sel);
               });
            }
            {  // Layer editing
               auto _change_layer  = [this, sex]() { this->_push_tint_layer_to_model(sex); };

               QObject::connect(sw.layers.edit.texture, &DKGameFilePicker::valueChanged, this, _change_layer);
               {  // Layer type
                  auto* widget = sw.layers.edit.type;
                  widget->clear();
                  {  // Items
                     widget->addItem(tr("None", "face tint type"), (int)dovah::face_tint_type::none);
                     widget->addItem(tr("Chin", "face tint type"), (int)dovah::face_tint_type::chin);
                     widget->addItem(tr("Cheek Color Upper", "face tint type"), (int)dovah::face_tint_type::cheek_color_upper);
                     widget->addItem(tr("Cheek Color Lower", "face tint type"), (int)dovah::face_tint_type::cheek_color_lower);
                     widget->addItem(tr("Dirt", "face tint type"), (int)dovah::face_tint_type::dirt);
                     widget->addItem(tr("Eyeliner", "face tint type"), (int)dovah::face_tint_type::eyeliner);
                     widget->addItem(tr("Eyeshadow Upper", "face tint type"), (int)dovah::face_tint_type::eyeshadow_upper);
                     widget->addItem(tr("Eyeshadow Lower", "face tint type"), (int)dovah::face_tint_type::eyeshadow_lower);
                     widget->addItem(tr("Facepaint", "face tint type"), (int)dovah::face_tint_type::facepaint);
                     widget->addItem(tr("Forehead", "face tint type"), (int)dovah::face_tint_type::forehead);
                     widget->addItem(tr("Laugh Lines", "face tint type"), (int)dovah::face_tint_type::laugh_lines);
                     widget->addItem(tr("Lip Color", "face tint type"), (int)dovah::face_tint_type::lip_color);
                     widget->addItem(tr("Skin Tone", "face tint type"), (int)dovah::face_tint_type::skin_tone);
                     widget->addItem(tr("Neck", "face tint type"), (int)dovah::face_tint_type::neck);
                     widget->addItem(tr("Nose", "face tint type"), (int)dovah::face_tint_type::nose);
                  }
                  QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, _change_layer);
               }
               {  // Default color
                  auto* widget = sw.layers.edit.default_color;
                  auto* filter = this->_filters.tint_layer_default_color[sex] = new RaceTintDefaultColorPickerFilter(this);
                  filter->setModel(model);

                  widget->setAllowedFormType(dovah::form_type::color);
                  widget->setCustomFilter(filter);

                  QObject::connect(widget, &DKFormPicker::formChanged, this, _change_layer);
               }
            }
            {  // Preset table
               ui::typical_tableview_config(preset_table);
               preset_table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
               ui::set_tableview_column_flex(preset_table, [](DKHeaderView& header, const QFontMetrics& metrics) {
                  header.setColumnFlex(RaceTintLayerModel::PresetColumn::ColorPreview, 0, 0, metrics.boundingRect("   ").width() * 1.5F + 4);
                  header.setColumnFlex(RaceTintLayerModel::PresetColumn::Name,  1, 0);
                  header.setColumnFlex(RaceTintLayerModel::PresetColumn::Alpha, 0, 0, metrics.boundingRect("1.000").width() * 1.5F + 4);
               });
               QObject::connect(preset_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, sex](const QItemSelection& sel) {
                  this->_on_tint_preset_selection_changed(sex, sel);
               });
            }
            {  // Preset editing
               auto _change_preset = [this, sex]() { this->_push_tint_preset_to_model(sex); };
                  
               QObject::connect(sw.presets.edit.color,         &DKFormPicker::formChanged, this, _change_preset);
               QObject::connect(sw.presets.edit.alpha.spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, _change_preset);
               QObject::connect(sw.presets.edit.alpha.slider,  &DKFloatSlider::valueChanged, this, [this, &sw](float value) {
                  sw.presets.edit.alpha.spinbox->setValue(value);
               });
            }
            // Done.
         };
         _configure(dovah::sex::female);
         _configure(dovah::sex::male);
      }
      this->ui.currentTintFDefaultColor->setAllowedFormType(dovah::form_type::color);
      this->ui.currentTintMDefaultColor->setAllowedFormType(dovah::form_type::color);
      this->ui.defaultFaceTextureF->setAllowedFormType(dovah::form_type::texture_set);
      this->ui.defaultFaceTextureM->setAllowedFormType(dovah::form_type::texture_set);
      this->ui.faceTexturesF->setAllowedFormTypes({ dovah::form_type::texture_set });
      this->ui.faceTexturesM->setAllowedFormTypes({ dovah::form_type::texture_set });
      {  // Complexions
         auto _configure = [this](
            dovah::sex      sex,
            DKFormPicker*   picker,
            DKFormListPane* complexions
         ) {
            picker->setAllowedFormType(dovah::form_type::texture_set);
            picker->setAllowNone(true);
            complexions->setAllowedFormTypes({ dovah::form_type::texture_set });

            auto* filter = this->_filters.default_complexion[sex] = new FormPickerFromFormListPaneFilter(this);
            filter->setPane(complexions);
            picker->setCustomFilter(filter);
         };
         _configure(dovah::sex::female, this->ui.defaultFaceTextureF, this->ui.faceTexturesF);
         _configure(dovah::sex::male,   this->ui.defaultFaceTextureM, this->ui.faceTexturesM);
      }
   #pragma endregion
   #pragma region Presets tab
      this->ui.presetsF->setAllowedFormTypes({ dovah::form_type::actor_base });
      this->ui.presetsM->setAllowedFormTypes({ dovah::form_type::actor_base });
      {
         auto sex = dovah::sex::female;

         auto* pane   = this->ui.presetsF;
         auto* filter = this->_filters.preset_actors[sex] = new RacePresetActorFormFilter(this);
         filter->setRace(this->formStub());
         filter->setSex(sex);
         pane->setCustomFilter(filter);
      }
      {
         auto sex = dovah::sex::male;

         auto* pane   = this->ui.presetsM;
         auto* filter = this->_filters.preset_actors[sex] = new RacePresetActorFormFilter(this);
         filter->setRace(this->formStub());
         filter->setSex(sex);
         pane->setCustomFilter(filter);
      }
   #pragma endregion

   this->load(); // this creates the working copy.
   this->_state.filling_movement_type_list = false;
}
void FormDialogRace::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   
   #pragma region General tab
      // Inherit
      ui::bind(this->ui.morphRace, working.morph_race, working);
      ui::bind(this->ui.armorRace, working.armor_race, working);

      // Miscellaneous
      ui::bind(this->ui.creatureSize,      working.stats.creature_size);
      ui::bind(this->ui.flightRadius,      working.movement.flight_radius);
      ui::bind(this->ui.baseCarryCapacity, working.stats.base_carry_capacity);

      // Flags
      {
         using flag = loaded_form_type::race_flag;
         auto& dst  = working.race_flags;

         ui::bind(this->ui.flagPlayable,        dst, flag::playable);
         ui::bind(this->ui.flagAllowPickpocket, dst, flag::can_be_pickpocketed);
         ui::bind(this->ui.flagNoKnockdowns,    dst, flag::no_knockdowns);

         ui::bind(this->ui.flagChild,           dst, flag::child);
         ui::bind(this->ui.flagCannotOpenDoors, dst, flag::cant_open_doors);
         ui::bind(this->ui.flagNoShadow,        dst, flag::no_shadow);

         ui::bind(this->ui.flagAllowPlayerDialogue,      dst, flag::allow_player_dialogue);
         ui::bind(this->ui.flagAllowRagdollCollision,    dst, flag::allow_ragdoll_collision);
         ui::bind(this->ui.flagSpellsAlignWithMagicNode, dst, flag::spells_align_with_magic_node);

         ui::bind(this->ui.flagFaceGenHead,             dst, flag::facegen_head);
         ui::bind(this->ui.flagMultipleMembraneShaders, dst, flag::allow_multiple_membrane_shaders);
         ui::bind(this->ui.flagCanPickUpItems,          dst, flag::can_pick_up_items);
      }

      // Skill bonuses
      for (size_t i = 0; i < num_skill_boosts; ++i) {
         auto* widget_which = this->_subwidgets.skills.which[i];
         auto* widget_value = this->_subwidgets.skills.boost[i];

         auto& src_opt = working.stats.skill_boosts[i];
         if (src_opt.has_value()) {
            auto& src = src_opt.value();
            widget_which->setCurrentIndex(widget_which->findData((int)src.skill));
            widget_value->setValue(src.boost);
         } else {
            widget_which->setCurrentIndex(widget_which->findData(-1));
            widget_value->setValue(0);
         }
      }

      // Attributes
      ui::bind(this->ui.attrBaseH, working.stats.attribute_base.h);
      ui::bind(this->ui.attrBaseM, working.stats.attribute_base.m);
      ui::bind(this->ui.attrBaseS, working.stats.attribute_base.s);
      ui::bind(this->ui.attrRegenH, working.stats.attribute_regen.h);
      ui::bind(this->ui.attrRegenM, working.stats.attribute_regen.m);
      ui::bind(this->ui.attrRegenS, working.stats.attribute_regen.s);
      ui::bind(this->ui.flagRegenInCombat, working.race_flags, loaded_form_type::race_flag::regen_health_in_combat);

      // Spells
      this->ui.abilities->pullStubs(working.spells.forms);

      // Basic Movement Options
      {
         using flag = loaded_form_type::race_flag;
         auto& dst  = working.race_flags;

         ui::bind(this->ui.flagImmobile, dst, flag::immobile);
         ui::bind(this->ui.flagWalks, dst, flag::walks);
         ui::bind(this->ui.flagSwims, dst, flag::swims);
         ui::bind(this->ui.flagFlies, dst, flag::flies);
         ui::bind(this->ui.flagAvoidsRoads, dst, flag::avoids_roads);
         ui::bind(this->ui.flagNotPushable, dst, flag::not_pushable);
         ui::bind(this->ui.flagNoWaterCombat, dst, flag::no_water_combat);
         ui::bind(this->ui.flagNoRotateToHeadtrack, dst, flag::no_rotate_to_headtrack);
         ui::bind(this->ui.flagUsesHeadtrackAnims, dst, flag::uses_headtrack_anims);

         ui::bind(this->ui.flagTiltPitch, dst, flag::tilt_front_back);
         ui::bind(this->ui.flagTiltRoll,  dst, flag::tilt_left_right);
         ui::bind(this->ui.flagWorldRaycastsForIK,    dst, flag::use_world_raycasts_for_foot_ik);
         ui::bind(this->ui.flagAlwaysProxyController, dst, flag::always_use_proxy_controller);
      }

      // Mount Data
      ui::bind(this->ui.mountOffsetX, working.mount_data.climb_on_offset.x);
      ui::bind(this->ui.mountOffsetY, working.mount_data.climb_on_offset.y);
      ui::bind(this->ui.mountOffsetZ, working.mount_data.climb_on_offset.z);
      ui::bind(this->ui.dismountOffsetX, working.mount_data.dismount_offset.x);
      ui::bind(this->ui.dismountOffsetY, working.mount_data.dismount_offset.y);
      ui::bind(this->ui.dismountOffsetZ, working.mount_data.dismount_offset.z);
      ui::bind(this->ui.mountCamOffsetX, working.mount_data.camera_offset.x);
      ui::bind(this->ui.mountCamOffsetY, working.mount_data.camera_offset.y);
      ui::bind(this->ui.mountCamOffsetZ, working.mount_data.camera_offset.z);
      ui::bind(this->ui.flagAllowMountedCombat, working.alt_flags, loaded_form_type::alt_flag::allow_mounted_combat);
   #pragma endregion
   #pragma region Keywords tab
      this->ui.keywords->pullStubs(working.keywords.forms);
   #pragma endregion
   #pragma region Body tab
      ui::bind(this->ui.mass, working.stats.base_mass);
      ui::bind(this->ui.bodyPartData, working.body_part_data, working);
      ui::bind(this->ui.skin, working.skin, working);

      {  // Female
         auto& dst = working.by_sex.female;
         ui::bind(this->ui.heightMultF, dst.height_mult);
         ui::bind(this->ui.bodyWeightF, dst.weight);
         this->ui.skeletonF->initializeFrom(dst.skeleton_nif);
         this->ui.behaviorGraphF->initializeFrom(dst.behavior_graph);
         this->ui.bodyTextureF->initializeFrom(dst.lighting_model);
         ui::bind(this->ui.decapArmorF, dst.decapitate_armor, working);
         ui::bind(this->ui.voicetypeF, dst.voicetype, working);
      }
      {  // Male
         auto& dst = working.by_sex.male;
         ui::bind(this->ui.heightMultM, dst.height_mult);
         ui::bind(this->ui.bodyWeightM, dst.weight);
         this->ui.skeletonM->initializeFrom(dst.skeleton_nif);
         this->ui.behaviorGraphM->initializeFrom(dst.behavior_graph);
         this->ui.bodyTextureM->initializeFrom(dst.lighting_model);
         ui::bind(this->ui.decapArmorM, dst.decapitate_armor, working);
         ui::bind(this->ui.voicetypeM, dst.voicetype, working);
      }

      {  // Biped objects
         this->_models.biped_objects->initializeFrom(working, working.biped_object);
         this->_update_slot_dropdowns();
         auto _set_up_slot_picker = [this](QComboBox* widget, int32_t& dst) {
            {
               int i = widget->findData(dst);
               if (i >= 0)
                  widget->setCurrentIndex(i);
               else
                  widget->setCurrentIndex(widget->findData(-1));
            }
            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [widget, &dst]() {
               auto data = widget->currentData();
               if (data.isValid())
                  dst = data.toInt();
               else
                  dst = -1;
            });
         };
         _set_up_slot_picker(this->ui.bipedSlotBody,   working.biped_object_info.body);
         _set_up_slot_picker(this->ui.bipedSlotHair,   working.biped_object_info.hair);
         _set_up_slot_picker(this->ui.bipedSlotHead,   working.biped_object_info.head);
         _set_up_slot_picker(this->ui.bipedSlotShield, working.biped_object_info.shield);
      }
   #pragma endregion
   #pragma region Blood tab
      ui::bind(this->ui.impactMaterialType, working.material_type, working);
      ui::bind(this->ui.decapBloodArt, working.decapitation_effect, working);
      ui::bind(this->ui.impactDataSet, working.impact_data_set, working);
      ui::bind(this->ui.soundOpen, working.container_sounds.open, working);
      ui::bind(this->ui.soundClose, working.container_sounds.close, working);
   #pragma endregion
   #pragma region Text tab
      this->ui.name->setText(gls.convert_localized_string(working.name));
      this->ui.description->setPlainText(gls.convert_localized_string(working.description));
   #pragma endregion
   #pragma region Movement Details tab
      ui::bind(this->ui.accelerationRate, working.movement.acceleration_rate);
      ui::bind(this->ui.decelerationRate, working.movement.deceleration_rate);
      ui::bind(this->ui.angularAccelerationRate, working.movement.angular_acceleration_rate);
      ui::bind(this->ui.angularTolerance, working.movement.angular_tolerance);
      ui::bind(this->ui.flagUseAdvancedAvoidance, working.alt_flags, loaded_form_type::alt_flag::use_advanced_avoidance);

      this->_models.base_movement_types->initializeFrom(working);

      {  // Movement data overrides
         auto* widget  = this->ui.movementTypeOverrideForms;
         auto  blocker = QSignalBlocker(widget);
         widget->clear();
         for (auto& item : working.movement.overrides)
            if (auto* stub = item.type.get_form_stub())
               widget->addStub(stub);
      }
   #pragma endregion
   #pragma region Attack Data tab
      this->ui.attackData->initializeFrom(working.attack_data);
   #pragma endregion
   #pragma region Combat tab
      ui::bind(this->ui.injuredHealthPercentage, working.stats.injured_health_threshold);
      ui::bind(this->ui.unarmedDamage, working.stats.unarmed.damage);
      ui::bind(this->ui.unarmedReach, working.stats.unarmed.reach);
      ui::bind(this->ui.unarmedEquipSlot, working.equipment.unarmed_equip_slot, working);
      ui::bind(this->ui.aimAngleTolerance, working.stats.aim_angle_tolerance);
      ui::bind(this->ui.flagCanDualWield, working.race_flags, loaded_form_type::race_flag::can_dual_wield);
      ui::bind(this->ui.flagNonHostile, working.alt_flags, loaded_form_type::alt_flag::non_hostile);

      this->_models.equip_slots->initializeFrom(working.equipment.equip_slots);
      this->_models.equip_types->initializeFrom(working);
   #pragma endregion
   #pragma region Lip Synching tab
      {
         auto* model = this->_models.current_phoneme_morphs;
         if (working.uses_default_facegen_phonemes()) {
            this->ui.flagDefaultFaceGenPhonemes->setChecked(true);
         } else {
            this->ui.flagDefaultFaceGenPhonemes->setChecked(false);

            model->setAllMorphNames(working.phonemes.morph_names);
            this->_on_selected_phoneme_changed();
         }
      }
   #pragma endregion
   #pragma region Face Data tab
      {  // Head part inheritance mode
         constexpr const auto all_flags = loaded_form_type::race_flag::overlay_head_part_list | loaded_form_type::race_flag::override_head_part_list;

         this->ui.faceHeadPartsOverlay->setProperty("inherit_flag", (int)loaded_form_type::race_flag::overlay_head_part_list);
         this->ui.faceHeadPartsOverride->setProperty("inherit_flag", (int)loaded_form_type::race_flag::override_head_part_list);
         this->ui.faceHeadPartsInherit->setProperty("inherit_flag", (int)0);
         auto& widgets = this->_subwidgets.head_part_inheritance.all = {
            this->ui.faceHeadPartsOverlay,
            this->ui.faceHeadPartsOverride,
            this->ui.faceHeadPartsInherit,
         };

         if (!(working.race_flags & all_flags)) {
            for (auto* widget : widgets) {
               if (widget->property("inherit_flag").toInt() == 0) {
                  widget->setChecked(true);
                  break;
               }
            }
         } else {
            for (auto* widget : widgets) {
               if (working.race_flags & widget->property("inherit_flag").toInt()) {
                  widget->setChecked(true);
                  break;
               }
            }
         }
         for (auto* widget : widgets) {
            QObject::connect(widget, &QRadioButton::toggled, this, [this, widget](bool checked) {
               if (!checked)
                  return;
               auto& dst = this->form->race_flags;
               dst &= ~all_flags;
               dst |= (loaded_form_type::race_flags_t) widget->property("inherit_flag").toInt();
            });
         }
      }
      {  // Base Head Parts and Additional Head Parts
         using base_slot = FaceBaseHeadPartsModel::Slot;

         auto& fic = dovahkit::subsystems::form_info_cache::core::get();
         for (size_t i = 0; i < dovah::sex_count; ++i) {
            auto sex = (dovah::sex)i;
         
            auto* model_base  = this->_models.head_parts_base[sex];
            auto* model_extra = this->_models.head_parts_extra[sex];

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

            for (auto& form_use : working.by_sex[sex].head_data.head_parts) {
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
      }
      {  // Available Morphs
         this->_models.available_face_morphs.female->initializeFrom(working, dovah::sex::female);
         this->_models.available_face_morphs.male->initializeFrom(working, dovah::sex::male);
      }
      this->ui.hairColorsF->pullStubs(working.by_sex.female.head_data.hair_colors);
      this->ui.hairColorsM->pullStubs(working.by_sex.male.head_data.hair_colors);
      ui::bind(this->ui.defaultHairColorF, working.by_sex.female.head_data.default_hair_color, working);
      ui::bind(this->ui.defaultHairColorM, working.by_sex.male.head_data.default_hair_color, working);
   #pragma endregion
   #pragma region Face Tints tab
      {  // Tint layers
         for (size_t i = 0; i < dovah::sex_count; ++i) {
            auto sex = (dovah::sex)i;

            auto* model = this->_models.tint_layer_model[sex];
            auto* view  = this->_subwidgets.tints[sex].presets.view;
            auto* proxy = this->_models.tint_preset_model[sex];

            model->importLayers(working, sex);
            //
            // Model-resets will clear `setRootIndex` calls made to any views into the model, 
            // even if the root indices in question survive the reset. We need to set the root 
            // indices of our presets views back up.
            //
            view->setRootIndex(proxy->mapFromSource(model->noPresetQMI()));
         }
      }
      {  // Complexions
         this->ui.faceTexturesF->pullStubs(working.by_sex.female.head_data.face_textures);
         this->ui.faceTexturesM->pullStubs(working.by_sex.male.head_data.face_textures);
         ui::bind(this->ui.defaultFaceTextureF, working.by_sex.female.head_data.default_face_texture, working);
         ui::bind(this->ui.defaultFaceTextureM, working.by_sex.male.head_data.default_face_texture, working);
      }
   #pragma endregion
   #pragma region Presets tab
      this->ui.presetsF->pullStubs(working.by_sex.female.head_data.preset_actors);
      this->ui.presetsM->pullStubs(working.by_sex.male.head_data.preset_actors);
   #pragma endregion
}
void FormDialogRace::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   #pragma region General tab
      // Skill bonuses
      for (size_t i = 0; i < num_skill_boosts; ++i) {
         auto* widget_which = this->_subwidgets.skills.which[i];
         auto* widget_value = this->_subwidgets.skills.boost[i];

         auto& dst_opt = working.stats.skill_boosts[i];

         auto data = widget_which->currentData().toInt();
         if (data < 0) {
            dst_opt = {};
         } else {
            auto& dst = dst_opt.emplace();
            dst.skill = (dovah::skill)data;
            dst.boost = widget_value->value();
         }
      }

      // Spells
      this->ui.abilities->commitStubs(working.spells.forms, working);
   #pragma endregion
   #pragma region Keywords tab
      this->ui.keywords->commitStubs(working.keywords.forms, working);
   #pragma endregion
   #pragma region Body tab
      {  // Female
         auto& dst = working.by_sex.female;
         this->ui.skeletonF->commitTo(dst.skeleton_nif, working);
         this->ui.behaviorGraphF->commitTo(dst.behavior_graph, working);
         this->ui.bodyTextureF->commitTo(dst.lighting_model, working);
      }
      {  // Male
         auto& dst = working.by_sex.male;
         this->ui.skeletonM->commitTo(dst.skeleton_nif, working);
         this->ui.behaviorGraphM->commitTo(dst.behavior_graph, working);
         this->ui.bodyTextureM->commitTo(dst.lighting_model, working);
      }

      this->_models.biped_objects->commitTo(working, working.biped_object);
   #pragma endregion
   #pragma region Blood tab
      //
      // All properties in here are live-updated.
      //
   #pragma endregion
   #pragma region Text tab
      gls.assign_localized_string(working.name,        this->ui.name->text());
      gls.assign_localized_string(working.description, this->ui.description->toPlainText());
   #pragma endregion
   #pragma region Movement Details tab
      this->_models.base_movement_types->commitTo(working);
   #pragma endregion
   #pragma region Attack Data tab
      this->ui.attackData->commitTo(working.attack_data, working);
   #pragma endregion
   #pragma region Combat tab
      this->_models.equip_slots->commitTo(working.equipment.equip_slots, working);
      this->_models.equip_types->commitTo(working);
   #pragma endregion
   #pragma region Lip Synching tab
      //
      // Synchronizes more-or-less in real-time.
      //
   #pragma endregion
   #pragma region Face Data tab
      // Base Head Parts and Additional Head Parts
      for (size_t i = 0; i < dovah::sex_count; ++i) {
         auto sex = (dovah::sex)i;
         
         std::vector<dovah::form_stub*> head_parts;
         {
            auto* model = this->_models.head_parts_base[sex];
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
            auto*  model = this->_models.head_parts_extra[sex];
            size_t size  = model->rowCount();
            head_parts.reserve(head_parts.size() + size);
            for (size_t i = 0; i < size; ++i) {
               if (auto* stub = model->headPart(i))
                  head_parts.push_back(stub);
            }
         }

         auto& dst_list = working.by_sex[sex].head_data.head_parts;
         dovah::clear_form_reference_list(dst_list, working);
         for (auto* stub : head_parts) {
            auto& form_use = dst_list.emplace_back();
            write_form_ref(form_use, stub);
         }
      }
      
      {  // Available Morphs
         this->_models.available_face_morphs.female->commitTo(working, dovah::sex::female);
         this->_models.available_face_morphs.male->commitTo(working, dovah::sex::male);
      }

      this->ui.hairColorsF->commitStubs(working.by_sex.female.head_data.hair_colors, working);
      this->ui.hairColorsM->commitStubs(working.by_sex.male.head_data.hair_colors, working);
   #pragma endregion
   #pragma region Face Tints tab
      {  // Tint layers
         auto& by_sex = this->_models.tint_layer_model;
         by_sex.female->exportLayers(working, dovah::sex::female);
         by_sex.male->exportLayers(working, dovah::sex::male);
      }
      {  // Complexions
         this->ui.faceTexturesF->commitStubs(working.by_sex.female.head_data.face_textures, working);
         this->ui.faceTexturesM->commitStubs(working.by_sex.male.head_data.face_textures, working);
      }
   #pragma endregion
   #pragma region Presets tab
      this->ui.presetsF->commitStubs(working.by_sex.female.head_data.preset_actors, working);
      this->ui.presetsM->commitStubs(working.by_sex.male.head_data.preset_actors, working);
   #pragma endregion
}

void FormDialogRace::_update_slot_dropdowns() {
   const auto widgets = std::array{
      this->ui.bipedSlotBody,
      this->ui.bipedSlotHair,
      this->ui.bipedSlotHead,
      this->ui.bipedSlotShield,
   };
   const auto blockers = std::array{
      QSignalBlocker(this->ui.bipedSlotBody),
      QSignalBlocker(this->ui.bipedSlotHair),
      QSignalBlocker(this->ui.bipedSlotHead),
      QSignalBlocker(this->ui.bipedSlotShield),
   };

   constexpr const size_t slot_count = loaded_form_type::max_biped_object_name_count;

   std::array<QString, slot_count> names = {};
   if (auto* model = this->_models.biped_objects) {
      for (size_t i = 0; i < slot_count; ++i) {
         auto qmi = model->index(i, RaceBipedObjectSlotsModel::Column::Name, {});
         names[i] = model->data(qmi, Qt::UserRole).toString();
      }
   }

   for (auto* widget : widgets) {
      auto prior = widget->currentData();

      widget->clear();
      widget->addItem(tr("NONE"), (int)-1);
      for (size_t i = 0; i < slot_count; ++i) {
         auto& name = names[i];
         if (name.isEmpty())
            continue;
         widget->addItem(name, (int)i);
      }

      if (prior.isValid()) {
         auto i = widget->findData(prior);
         if (i >= 0)
            widget->setCurrentIndex(widget->findData(prior));
         else {
            widget->setCurrentIndex(-1);
            emit widget->currentIndexChanged(-1);
         }
      }
   }
}

#pragma region Movement type override editing
namespace {
   constexpr bool _movement_type_value_is_radians(size_t index) {
      if (index >= 8)
         return true;
      return false;
   }
}
void FormDialogRace::_pull_movement_type_overrides_to_ui(dovah::form_stub* stub) {
   const auto& fields = this->_subwidgets.movement_data_override.values;
   if (stub) {
      for (auto& data : this->form->movement.overrides) {
         if (data.type == stub) {
            auto& list = data.speeds.list;
            for (size_t i = 0; i < list.size(); ++i) {
               auto* widget = fields[i];
               if (!widget)
                  continue;
               auto blocker = QSignalBlocker(widget);
               widget->setEnabled(true);

               auto value = list[i];
               if (_movement_type_value_is_radians(i)) {
                  value *= cobb::radians_to_degrees_mult;
               }
               widget->setValue(value);
            }
            return;
         }
      }
   }
   //
   // No stub or not found:
   //
   for (auto* widget : fields) {
      if (widget)
         widget->setEnabled(false);
   }
}
void FormDialogRace::_push_movement_type_overrides_to_form() {
   auto& sw = this->_subwidgets.movement_data_override;

   dovah::form_stub* form = nullptr;
   {
      auto sel = sw.form_pane->selectedForms();
      if (sel.empty())
         return;
      form = sel[0];
      if (!form)
         return;
   }

   const auto& fields = this->_subwidgets.movement_data_override.values;
   for (auto& data : this->form->movement.overrides) {
      if (data.type == form) {
         auto& list = data.speeds.list;
         for (size_t i = 0; i < list.size(); ++i) {
            auto* widget = fields[i];
            if (!widget)
               continue;

            auto value = widget->value();
            if (_movement_type_value_is_radians(i)) {
               value *= cobb::degrees_to_radians_mult;
            }
            list[i] = value;
         }
         break;
      }
   }
}
#pragma endregion

#pragma region Phoneme editing
std::optional<dovah::face_fx::phoneme> FormDialogRace::_current_phoneme() const {
   auto* sel_model = this->ui.phonemeFaceFXList->selectionModel();
   auto  rows      = sel_model->selectedRows();
   if (rows.isEmpty())
      return {};
   return (dovah::face_fx::phoneme)rows[0].row();
}
void FormDialogRace::_on_selected_phoneme_changed() {
   bool allow_edit = !this->ui.flagDefaultFaceGenPhonemes->isChecked();

   dovah::face_fx::phoneme phoneme;
   {
      auto phoneme_opt = this->_current_phoneme();
      bool valid       = phoneme_opt.has_value() && this->form;
      this->ui.phonemeTargets->setEnabled(valid && allow_edit);
      this->ui.buttonPhonemeMorphNew->setEnabled(valid && allow_edit);
      this->ui.buttonPhonemeMorphDelete->setEnabled(valid && allow_edit);
      this->ui.currentPhonemeMorphName->setEnabled(valid && allow_edit);
      this->ui.currentPhonemeWeight->setEnabled(valid && allow_edit);
      if (valid) {
         phoneme = phoneme_opt.value();
      } else {
         return;
      }
   }
   auto& dst = this->form->phonemes.weights[(size_t)phoneme];
   this->_models.current_phoneme_morphs->setAllMorphWeights(dst);
   this->_pull_phoneme_target_to_ui();
}

void FormDialogRace::_pull_phoneme_target_to_ui() {
   auto* view  = this->ui.phonemeTargets;
   auto* model = this->_models.current_phoneme_morphs;
   auto  sel   = view->selectionModel()->selection();

   bool allow_edit = !this->ui.flagDefaultFaceGenPhonemes->isChecked();
   bool fail       = sel.isEmpty();
   this->ui.buttonPhonemeMorphDelete->setEnabled(!fail && allow_edit);
   this->ui.currentPhonemeMorphName->setEnabled(!fail && allow_edit);
   this->ui.currentPhonemeWeight->setEnabled(!fail && allow_edit);
   if (fail) {
      return;
   }

   auto  i      = sel[0].topLeft().row();
   float weight = model->morphWeight(i);
   auto  name   = model->morphName(i);

   auto blockers = std::array{
      QSignalBlocker(this->ui.currentPhonemeMorphName),
      QSignalBlocker(this->ui.currentPhonemeWeight),
   };

   this->ui.currentPhonemeMorphName->setText(name);
   this->ui.currentPhonemeWeight->setValue(weight);
}
void FormDialogRace::_push_phoneme_target_from_ui() {
   if (this->ui.flagDefaultFaceGenPhonemes->isChecked())
      return;

   auto phoneme = this->_current_phoneme();
   if (!phoneme.has_value())
      return;

   auto* view  = this->ui.phonemeTargets;
   auto* model = this->_models.current_phoneme_morphs;
   auto  sel   = view->selectionModel()->selection();
   if (sel.isEmpty())
      return;
   auto i = sel[0].topLeft().row();

   auto name   = this->ui.currentPhonemeMorphName->text();
   auto weight = this->ui.currentPhonemeWeight->value();

   model->setMorphName(i, name);
   model->setMorphWeight(i, weight);

   if (this->form) {
      auto& dst = this->form->phonemes;
      dst.morph_names[i] = name.toStdString();
      dst.weights[(size_t)phoneme.value()][i] = weight;
   }
}

void FormDialogRace::_add_phoneme_target() {
   if (this->ui.flagDefaultFaceGenPhonemes->isChecked())
      return;

   auto* view  = this->ui.phonemeTargets;
   auto* model = this->_models.current_phoneme_morphs;
   auto  qmi   = model->addMorph();
   if (!qmi.isValid())
      return;

   size_t i = qmi.row();
   this->form->insert_phoneme_morph(i);
   {
      auto& dst_list = this->form->phonemes.morph_names;
      assert(dst_list.size() == i);
      auto& dst_name = this->form->phonemes.morph_names.emplace_back();
      dst_name = model->morphName(i).toStdString();
   }

   auto* sel_model = view->selectionModel();
   auto  tl = qmi.siblingAtColumn(0);
   auto  br = qmi.siblingAtColumn(model->columnCount({}));
   sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}
void FormDialogRace::_remove_phoneme_target() {
   if (this->ui.flagDefaultFaceGenPhonemes->isChecked())
      return;

   auto* view  = this->ui.phonemeTargets;
   auto* model = this->_models.current_phoneme_morphs;
   auto  sel   = view->selectionModel()->selection();
   if (sel.isEmpty())
      return;

   size_t i = sel[0].topLeft().row();
   model->deleteMorph(i);
   this->form->delete_phoneme_morph(i);
}
//
void FormDialogRace::_set_phonemes_are_default(bool use_defaults) {
   if (!this->form)
      return;
   auto& dst = this->form->phonemes;

   auto* model       = this->_models.current_phoneme_morphs;
   auto  phoneme_opt = this->_current_phoneme();

   if (use_defaults) {
      this->form->revert_to_default_facegen_phonemes();

      {
         auto& src = dovah::face_fx::default_facegen_race_phonemes.morph_names;
         model->setAllMorphNames({ src.begin(), src.end() });
      }
      if (phoneme_opt.has_value()) {
         model->setAllMorphWeights(dst.weights[(size_t)phoneme_opt.value()]);
      }
   } else {
      //
      // Switching from default to non-default.
      //
      if (this->form->uses_default_facegen_phoneme_morph_names()) {
         this->form->copy_default_facegen_phonemes();
      }
   }
   this->_on_selected_phoneme_changed();
}
#pragma endregion

#pragma region Tint layer editing
void FormDialogRace::_add_new_tint_layer(dovah::sex sex) {
   auto* model     = this->_models.tint_layer_model[sex];
   auto* sel_model = (sex == dovah::sex::female ? this->ui.tintsTableF : this->ui.tintsTableM)->selectionModel();

   auto result = model->create_layer();
   if (!result.has_value())
      return;
   auto qmi = model->layer_qmi(result.value());
   if (!qmi.isValid())
      return;
   QItemSelection sel(qmi.siblingAtColumn(0), qmi.siblingAtColumn(RaceTintLayerModel::LayerColumnCount));
   sel_model->select(sel, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}
void FormDialogRace::_add_new_tint_preset(dovah::sex sex) {
   auto* model     = this->_models.tint_layer_model[sex];
   auto  layer_qmi = this->_selected_tint_layer(sex);
   if (!layer_qmi.isValid())
      return;
   auto result = model->create_preset(layer_qmi);
   if (!result.has_value())
      return;
   auto qmi = model->preset_qmi(result.value());
   if (!qmi.isValid())
      return;
   qmi = this->_models.tint_preset_model[sex]->mapFromSource(qmi);

   auto* view      = this->_subwidgets.tints[sex].presets.view;
   auto* sel_model = view->selectionModel();
   //
   QItemSelection sel(qmi.siblingAtColumn(0), qmi.siblingAtColumn(RaceTintLayerModel::PresetColumnCount));
   sel_model->select(sel, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}
void FormDialogRace::_on_tint_layer_selection_changed(dovah::sex sex, const QItemSelection& sel) {
   auto& sw = this->_subwidgets.tints[sex];

   auto* layer_model  = this->_models.tint_layer_model[sex];
   auto* preset_model = this->_models.tint_preset_model[sex];
   auto* preset_table = sw.presets.view;

   auto* default_color_filter = this->_filters.tint_layer_default_color[sex];

   std::optional<RaceTintLayerModel::LayerData> layer_opt;
   QModelIndex qmi;
   if (!sel.isEmpty()) {
      qmi       = sel[0].topLeft();
      layer_opt = layer_model->get_layer(qmi);
   }
   bool has_valid_layer = layer_opt.has_value();

   sw.layers.buttons.move_up->setEnabled(has_valid_layer);
   sw.layers.buttons.move_down->setEnabled(has_valid_layer);
   sw.layers.buttons.remove->setEnabled(has_valid_layer);
   sw.presets.buttons.create->setEnabled(has_valid_layer);
   if (!has_valid_layer) {
      sw.layers.edit.container->setEnabled(false);
      preset_table->setRootIndex(preset_model->mapFromSource(layer_model->noPresetQMI()));
      preset_table->selectionModel()->clearSelection();
      default_color_filter->setLayer({});
      return;
   }

   const auto& layer = layer_opt.value();

   const auto blockers = std::array{
      QSignalBlocker(sw.layers.edit.texture),
      QSignalBlocker(sw.layers.edit.type),
   };

   sw.layers.edit.texture->setValue(ui::types::game_file_path("Data\\Textures\\").append(QString::fromStdString(layer.texture)));
   sw.layers.edit.type->setCurrentIndex(sw.layers.edit.type->findData((int)layer.type));
   preset_table->setRootIndex(preset_model->mapFromSource(qmi));
   preset_table->selectionModel()->clearSelection();
   default_color_filter->setLayer(qmi);
   sw.layers.edit.container->setEnabled(true);
}
void FormDialogRace::_on_tint_preset_selection_changed(dovah::sex sex, const QItemSelection& sel) {
   auto& sw = this->_subwidgets.tints[sex];

   auto* layer_model  = this->_models.tint_layer_model[sex];
   auto* preset_model = this->_models.tint_preset_model[sex];
   auto* preset_table = sw.presets.view;

   std::optional<RaceTintLayerModel::PresetData> preset_opt;
   if (!sel.isEmpty()) {
      auto qmi = preset_model->mapToSource(sel[0].topLeft());
      preset_opt = layer_model->get_preset(qmi);
   }
   bool has_valid_preset = preset_opt.has_value();

   auto* edit_alpha = sw.presets.edit.alpha.slider;
   auto* edit_color = sw.presets.edit.color;

   edit_alpha->setEnabled(has_valid_preset);
   sw.presets.edit.alpha.spinbox->setEnabled(has_valid_preset);
   edit_color->setEnabled(has_valid_preset);
   sw.presets.buttons.move_up->setEnabled(has_valid_preset);
   sw.presets.buttons.move_down->setEnabled(has_valid_preset);
   sw.presets.buttons.remove->setEnabled(has_valid_preset);
   if (!has_valid_preset) {
      return;
   }

   const auto& preset = preset_opt.value();

   //
   // The slider just forwards its value into the spinbox, and the spinbox is what 
   // forwards changes to the form. We only need to block the spinbox, and update 
   // the slider.
   //

   const auto blockers = std::array{
      QSignalBlocker(edit_color),
      QSignalBlocker(edit_alpha),
   };

   edit_color->setFormStub(preset.color.form);
   edit_alpha->setValue(preset.alpha);
}
QModelIndex FormDialogRace::_selected_tint_layer(dovah::sex sex) const {
   return ui::get_selected_row_qmi(this->_subwidgets.tints[sex].layers.view->selectionModel());
}
QModelIndex FormDialogRace::_selected_tint_preset(dovah::sex sex) const {
   auto qmi = ui::get_selected_row_qmi(this->_subwidgets.tints[sex].presets.view->selectionModel());
   return this->_models.tint_preset_model[sex]->mapToSource(qmi);
}
//
void FormDialogRace::_push_tint_layer_to_model(dovah::sex sex) {
   auto* model = this->_models.tint_layer_model[sex];
   auto  qmi   = this->_selected_tint_layer(sex);

   auto data_opt = model->get_layer(qmi);
   if (!data_opt.has_value())
      return;
   auto& data = data_opt.value();

   const auto& sw = this->_subwidgets.tints[sex].layers.edit;
   data.texture       = sw.texture->value().lexically_relative("Data\\Textures\\").to_string().toStdString();
   data.type          = (dovah::face_tint_type)sw.type->currentData().toInt();
   data.default_color = sw.default_color->formStub();

   model->overwrite_layer(qmi, data);
}
void FormDialogRace::_push_tint_preset_to_model(dovah::sex sex) {
   auto* model = this->_models.tint_layer_model[sex];
   auto  qmi   = this->_selected_tint_preset(sex);

   auto data_opt = model->get_preset(qmi);
   if (!data_opt.has_value())
      return;
   auto& data = data_opt.value();

   const auto& sw = this->_subwidgets.tints[sex].presets.edit;
   data.alpha      = sw.alpha.spinbox->value();
   data.color.form = sw.color->formStub();

   model->overwrite_preset(qmi, data);
}
#pragma endregion

/*virtual*/ bool FormDialogRace::eventFilter(QObject* object, QEvent* event) /*override*/ {
   bool is_del_key = false;
   if (event->type() == QEvent::Type::KeyPress) {
      auto* casted = (QKeyEvent*)event;
      is_del_key = casted->key() == Qt::Key_Delete;
   }

   if (is_del_key) {
      //
      // Handle the Delete key on the Additional Head Parts listviews:
      //
      for (size_t i = 0; i < dovah::sex_count; ++i) {
         auto  sex    = (dovah::sex)i;
         auto* widget = this->_subwidgets.head_parts.extra[sex];
         if (object == widget) {
            auto* sel_model = widget->selectionModel();
            auto* model     = this->_models.head_parts_extra[sex];
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
      }
      if constexpr (allow_del_key_on_tint_layer_list) { // Del key for deleting a tint layer
         auto* widget_f = this->ui.tintsTableF;
         auto* widget_m = this->ui.tintsTableM;
         if (object == widget_f || object == widget_m) {
            auto sex = (object == widget_f) ? dovah::sex::female : dovah::sex::male;

            auto* sel_model = ((decltype(widget_f))object)->selectionModel();
            auto* model     = this->_models.tint_layer_model[sex];

            auto qmi = ui::get_selected_row_qmi(sel_model);
            if (qmi.isValid()) {
               model->remove_layer(qmi);
            }
            return true;
         }
      }
      {  // Del key for deleting a preset from a tint layer
         auto* widget_f = this->ui.currentTintFPresetList;
         auto* widget_m = this->ui.currentTintMPresetList;
         if (object == widget_f || object == widget_m) {
            auto sex = (object == widget_f) ? dovah::sex::female : dovah::sex::male;

            auto* sel_model = ((decltype(widget_f))object)->selectionModel();
            auto* model     = this->_models.tint_layer_model[sex];
            auto* model_ps  = this->_models.tint_preset_model[sex];

            auto qmi = model_ps->mapToSource(ui::get_selected_row_qmi(sel_model));
            if (qmi.isValid()) {
               model->remove_preset(qmi);
            }
            return true;
         }
      }
      if (object == this->ui.phonemeTargets) {
         this->_remove_phoneme_target();
         return true;
      }
   }


   return false;
}