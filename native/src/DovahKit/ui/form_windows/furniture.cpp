#include "./furniture.h"
#include "dovah/core.h"
#include "ui/utils/enum_dropdown_configs/skill.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./furniture/FurnitureMarkersModel.h"

FormDialogFurniture::FormDialogFurniture(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   {
      auto* view  = this->ui.activeMarkers;
      auto* model = this->_models.markers = new FurnitureMarkersModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setWordWrap(false);

      auto* sel_model = view->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model](const QItemSelection& selected) {
         if (!this->_models.entry_points)
            return;
         const auto blocker = QSignalBlocker(this->ui.markerKeyword);
         if (selected.empty()) {
            this->_models.entry_points->setSource({});
            this->ui.markerKeyword->setEnabled(false);
            this->ui.markerKeyword->setFormStub(nullptr);
         } else {
            auto qmi = selected[0].topLeft();
            this->_models.entry_points->setSource(qmi);
            this->ui.markerKeyword->setEnabled(true);
            this->ui.markerKeyword->setFormStub(model->data(qmi, FurnitureMarkersModel::KeywordRole).value<dovah::form_stub*>());
         }
      });
      this->ui.markerKeyword->setEnabled(false);
   }
   {
      auto* view  = this->ui.markerEntryPoints;
      auto* model = this->_models.entry_points = new FurnitureMarkerEntryPointsProxyModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      view->setWordWrap(false);
   }
   this->ui.markerKeyword->setAllowedFormType(dovah::form_type::keyword);
   this->ui.associatedSpell->setAllowedFormType(dovah::form_type::spell);

   this->ui.interactionKeyword->setAllowedFormType(dovah::form_type::keyword);
   {
      auto* widget = this->ui.workbenchType;
      using enumeration = loaded_form_type::workbench_type;
      widget->clear();
      widget->addItem(tr("None"), (int)enumeration::none);
      widget->addItem(tr("Alchemy"), (int)enumeration::alchemy);
      widget->addItem(tr("Alchemy Experiment"), (int)enumeration::alchemy_experiment);
      widget->addItem(tr("Create Object"), (int)enumeration::create_object);
      widget->addItem(tr("Enchanting"), (int)enumeration::enchanting);
      widget->addItem(tr("Enchanting Experiment"), (int)enumeration::enchanting_experiment);
      widget->addItem(tr("Smithing Armor"), (int)enumeration::smithing_armor);
      widget->addItem(tr("Smithing Weapon"), (int)enumeration::smithing_weapon);
   }
   ui::enum_dropdown_configs::skill(this->ui.workbenchSkill, true, false);

   this->load(); // this creates the working copy.
}
void FormDialogFurniture::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   this->_update_nif_related_flags();
   QObject::connect(this->ui.model, &DKFormNIFPicker::dataChanged, this, [this]() {
      auto path = this->ui.model->value().model_path;
      this->_models.markers->setNIF(path);
      this->_update_nif_related_flags();
   });
   this->ui.destructionData->initializeFrom(working.destruction_data);
   this->ui.keywords->pullStubs(working.keywords.forms);
   this->_models.markers->importData(working);
   ui::bind(this->ui.associatedSpell, working.associated_spell, working);
   
   {  // Flags
      auto& record_flags = this->record_flags();
      auto& data_flags   = working.active_markers_and_furn_flags;
      ui::bind(this->ui.flagRandomAnimStart,   record_flags, loaded_form_type::form_flag::random_anim_start);
      ui::bind(this->ui.flagChild,             record_flags, loaded_form_type::form_flag::child_can_use);
      ui::bind(this->ui.flagIsMarker,          record_flags, loaded_form_type::form_flag::is_marker);
      ui::bind(this->ui.flagDisableActivation, data_flags,   loaded_form_type::furniture_flag::disables_activation);
      ui::bind(this->ui.flagMustExitToTalk,    record_flags, loaded_form_type::form_flag::must_exit_to_talk);
      ui::bind(this->ui.flagIsPerch,           record_flags, loaded_form_type::form_flag::is_perch);
      ui::bind(this->ui.flagIgnoredBySandbox,  record_flags, loaded_form_type::form_flag::ignore_object_interaction);

      QObject::connect(this->ui.flagMustExitToTalk, &QCheckBox::toggled, this, [this](bool toggled) {
         constexpr auto flag = loaded_form_type::furniture_flag::must_exit_to_talk;
         auto& mirror = this->form->active_markers_and_furn_flags;
         if (toggled)
            mirror |= flag;
         else
            mirror &= ~flag;
      });
      QObject::connect(this->ui.flagIsPerch, &QCheckBox::toggled, this, [this](bool toggled) {
         constexpr auto flag = loaded_form_type::furniture_flag::is_perch;
         auto& mirror = this->form->active_markers_and_furn_flags;
         if (toggled)
            mirror |= flag;
         else
            mirror &= ~flag;
      });
   }
   {
      ui::types::nif_for_form nif;
      nif.model_path = working.marker_model;
      this->ui.markerModel->setValue(nif);
   }
   ui::bind(this->ui.interactionKeyword, working.interact_keyword, working);
   ui::bind(this->ui.workbenchType, working.workbench.type);
   {
      auto& target = working.workbench.skill;
      auto* widget = this->ui.workbenchSkill;
      if (target.has_value()) {
         widget->setCurrentIndex(widget->findData((int)target.value()));
      } else {
         widget->setCurrentIndex(widget->findData(-1));
      }
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), widget, [widget, &target]() {
         auto skill = widget->currentData().toInt();
         if (skill < 0)
            target.reset();
         else
            target = (dovah::skill) skill;
      });
   }
   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogFurniture::_save_impl() {
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
   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->_models.markers->exportData(working);

   {
      auto v = this->ui.markerModel->value();
      working.marker_model = v.model_path;
   }
   this->ui.scriptListPane->commit();
}

void FormDialogFurniture::_update_nif_related_flags() {
   auto& working = *this->form;
   auto* model   = this->_models.markers;
   if (!model->wereMarkersLoadedFromNIF()) {
      working.active_markers_and_furn_flags &= ~(
         loaded_form_type::furniture_flag::nif_has_a_lean_marker |
         loaded_form_type::furniture_flag::nif_has_a_sit_marker |
         loaded_form_type::furniture_flag::nif_has_a_sleep_marker
      );
      return;
   }
   bool has_lean  = false;
   bool has_sit   = false;
   bool has_sleep = false;
   const size_t size = model->rowCount();
   for (size_t i = 0; i < size; ++i) {
      auto data = model->data(model->index(i, 0, {}), FurnitureMarkersModel::AnimationTypeRole);
      if (data.canConvert<int>()) {
         switch ((FurnitureMarkersModel::AnimationType)data.value<int>()) {
            case FurnitureMarkersModel::AnimationType::lean:
               has_lean = true;
               break;
            case FurnitureMarkersModel::AnimationType::sit:
               has_sit = true;
               break;
            case FurnitureMarkersModel::AnimationType::sleep:
               has_sleep = true;
               break;
         }
      }
   }
   if (has_lean)
      working.active_markers_and_furn_flags |=  loaded_form_type::furniture_flag::nif_has_a_lean_marker;
   else
      working.active_markers_and_furn_flags &= ~loaded_form_type::furniture_flag::nif_has_a_lean_marker;
   if (has_sit)
      working.active_markers_and_furn_flags |=  loaded_form_type::furniture_flag::nif_has_a_sit_marker;
   else
      working.active_markers_and_furn_flags &= ~loaded_form_type::furniture_flag::nif_has_a_sit_marker;
   if (has_sleep)
      working.active_markers_and_furn_flags |=  loaded_form_type::furniture_flag::nif_has_a_sleep_marker;
   else
      working.active_markers_and_furn_flags &= ~loaded_form_type::furniture_flag::nif_has_a_sleep_marker;
}