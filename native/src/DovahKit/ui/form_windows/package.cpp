#include "./package.h"
#include "dovah/core.h"
#include "dovah/forms/components/papyrus/fragment_data/package_fragment_data.h"
#include "dovah/forms/structs/typed_package_info/custom.h"
#include "editor/localize/package_data_type.h"
#include "editor/localize/package_interrupt_override_type.h"
#include "editor/localize/package_procedure_tree_branch_type.h"
#include "editor/localize/package_procedure_type.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/typical_tableview_config.h"
#include "./package/FormSubdialogPackageLocation.h"
#include "./package/FormSubdialogPackageTarget.h"
#include "./package/PackageDataModel.h"
#include "./package/PackageProcedureParamsModel.h"
#include "./package/PackageProcedureTreeModel.h"
#include "./package/PackageTemplatePickerFilter.h"

FormDialogPackage::FormDialogPackage(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_filters.package_template = new PackageTemplatePickerFilter(this);
   this->ui.templateForm->setCustomFilter(this->_filters.package_template);

   this->ui.owningQuest->setAllowedFormType(dovah::form_type::quest);
   this->ui.combatStyle->setAllowedFormType(dovah::form_type::combat_style);
   {
      auto* widget = this->ui.interruptOverride;
      widget->clear();
      for (auto t : std::array{
         dovah::packages::interrupt_override_type::none,
         dovah::packages::interrupt_override_type::combat,
         dovah::packages::interrupt_override_type::guard_warn,
         dovah::packages::interrupt_override_type::observe_dead,
         dovah::packages::interrupt_override_type::spectator,
      }) {
         widget->addItem(editor::localize::package_interrupt_override_type(t), (int)t);
      }
   }

   #pragma region Package
      {
         this->ui.templateForm->setAllowedFormType(dovah::form_type::package);
      }
      {  // Package Data
         auto* model = this->_models.package_data = new PackageDataModel(this);
         auto* view  = this->ui.packdata;
         view->setModel(model);
         ui::typical_tableview_config(view);
         view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

         auto* sel_model = view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &FormDialogPackage::_on_packdata_selection_changed);
         #pragma region Buttons
            QObject::connect(this->ui.buttonPackdataNew, &QPushButton::clicked, this, [this, model, sel_model]() {
               auto qmi = model->appendRow();
               if (!qmi.isValid())
                  return;
               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(PackageDataModel::Column::__COUNT - 1);
               sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            });
            QObject::connect(this->ui.buttonPackdataMoveUp, &QPushButton::clicked, this, [this, model, sel_model]() {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               auto row = rows[0].row();
               model->moveRow(row, -1);
            });
            QObject::connect(this->ui.buttonPackdataMoveDown, &QPushButton::clicked, this, [this, model, sel_model]() {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               auto row = rows[0].row();
               model->moveRow(row, 1);
            });
            QObject::connect(this->ui.buttonPackdataDelete, &QPushButton::clicked, this, [this, model, sel_model]() {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               auto row = rows[0].row();
               model->deleteRow(row);
            });
         #pragma endregion

         {  // Package Type
            auto* widget = this->ui.currentPackdataType;
            widget->clear();
            for (auto t : std::array{
               dovah::packages::package_data_type::boolean,         // BGSPackageDataBool:           "Bool"
               dovah::packages::package_data_type::float32,         // BGSPackageDataFloat:          "Float"
               dovah::packages::package_data_type::integer,         // BGSPackageDataInt:            "Int"
               dovah::packages::package_data_type::location,        // BGSPackageDataLocation:       "Location"
               dovah::packages::package_data_type::object_list,     // BGSPackageDataObjectList:     "ObjectList"
               dovah::packages::package_data_type::single_ref,      // BGSPackageDataRef:            "SingleRef"
               dovah::packages::package_data_type::target_selector, // BGSPackageDataTargetSelector: "TargetSelector"
               dovah::packages::package_data_type::topic,           // BGSPackageDataTopic:          "Topic"
            }) {
               widget->addItem(editor::localize::package_data_type(t), (int)t);
            }
         }

         #pragma region Selected Package Data
            QObject::connect(this->ui.currentPackdataName, &QLineEdit::textChanged, this, &FormDialogPackage::_on_packdata_declaration_edited);
            QObject::connect(this->ui.currentPackdataIsPublic, &QCheckBox::toggled, this, &FormDialogPackage::_on_packdata_declaration_edited);

            QObject::connect(this->ui.currentPackdataType, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormDialogPackage::_on_packdata_type_edited);

            ui::set_range<float>(this->ui.currentPackdataValue_Float);
            ui::set_range<int32_t>(this->ui.currentPackdataValue_Int);
            ui::set_range<int32_t>(this->ui.currentPackdataValue_LocationRadius);
            ui::set_range<int32_t>(this->ui.currentPackdataValue_TargetRadius);

            QObject::connect(this->ui.currentPackdataValue_Bool, &QCheckBox::toggled, this, &FormDialogPackage::_on_packdata_value_edited);
            QObject::connect(this->ui.currentPackdataValue_Float, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FormDialogPackage::_on_packdata_value_edited);
            QObject::connect(this->ui.currentPackdataValue_Int, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogPackage::_on_packdata_value_edited);
            QObject::connect(this->ui.currentPackdataValue_LocationRadius, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogPackage::_on_packdata_value_edited);
            QObject::connect(this->ui.currentPackdataValue_TargetRadius, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogPackage::_on_packdata_value_edited);
            QObject::connect(this->ui.currentPackdataValue_Topic, &DKTopicOrSubtypePicker::valueChanged, this, &FormDialogPackage::_on_packdata_value_edited);

            QObject::connect(this->ui.currentPackdataValue_LocationButtonEdit, &QPushButton::clicked, this, [this]() {
               const auto row_opt = this->_selected_packdata_row();
               if (!row_opt.has_value())
                  return;
               const auto row = row_opt.value();

               FormSubdialogPackageLocation dialog;
               {
                  const auto value_opt = this->_models.package_data->rowValue(row);
                  if (value_opt.has_value()) {
                     auto& value = value_opt.value();
                     if (value.is<dovah::packages::package_data_type::location>())
                        dialog.setValue(value.as<dovah::packages::package_data_type::location>());
                  }
               }
               if (dialog.exec() == QDialog::DialogCode::Accepted) {
                  ui::types::packages::package_data_value value = dialog.value();
                  this->_models.package_data->setRowValue(row, value);
               }
            });
            QObject::connect(this->ui.currentPackdataValue_TargetButtonEdit, &QPushButton::clicked, this, [this]() {
               const auto row_opt = this->_selected_packdata_row();
               if (!row_opt.has_value())
                  return;
               const auto row = row_opt.value();

               FormSubdialogPackageTarget dialog;
               auto type = dovah::packages::package_data_type::single_ref;
               {
                  const auto value_opt = this->_models.package_data->rowValue(row);
                  if (value_opt.has_value()) {
                     auto& value = value_opt.value();
                     switch (auto prior_type = value.type()) {
                        case dovah::packages::package_data_type::single_ref:
                           dialog.setValue(value.as< dovah::packages::package_data_type::single_ref>());
                           type = prior_type;
                           break;
                        case dovah::packages::package_data_type::target_selector:
                           dialog.setValue(value.as< dovah::packages::package_data_type::target_selector>());
                           type = prior_type;
                           break;
                     }
                  }
               }
               if (dialog.exec() == QDialog::DialogCode::Accepted) {
                  ui::types::packages::package_data_value value;
                  switch (type) {
                     case dovah::packages::package_data_type::single_ref:
                        value.emplace<dovah::packages::package_data_type::single_ref>() = dialog.value();
                        break;
                     case dovah::packages::package_data_type::target_selector:
                        value.emplace<dovah::packages::package_data_type::target_selector>() = dialog.value();
                        break;
                  }
                  this->_models.package_data->setRowValue(row, value);
               }
            });
         #pragma endregion
      }
      {  // Procedure tree
         auto* model = this->_models.procedure_tree = new PackageProcedureTreeModel(this);
         auto* view  = this->ui.procedures;
         view->setModel(model);
         view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
         view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

         auto* sel_model = view->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &FormDialogPackage::_on_procedure_tree_selection_changed);

         {
            auto& menu_ui = this->_context_menus.procedure_tree;
            auto& menu    = menu_ui.menu;

            {
               auto* action = menu_ui.create_branch = new QAction(tr("Add branch"), this);
               menu.addAction(action);
               QAction::connect(action, &QAction::triggered, this, [this, sel_model]() {
                  auto parent = this->_selected_procedure_node_qmi();
                  auto qmi    = this->_models.procedure_tree->appendBranch(parent);
                  if (qmi.isValid())
                     sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
               });
            }
            {
               auto* action = menu_ui.create_procedure = new QAction(tr("Add procedure"), this);
               menu.addAction(action);
               QAction::connect(action, &QAction::triggered, this, [this, sel_model]() {
                  auto parent = this->_selected_procedure_node_qmi();
                  auto qmi    = this->_models.procedure_tree->appendProcedure(parent);
                  if (qmi.isValid())
                     sel_model->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
               });
            }
            {
               auto* action = menu_ui.remove = new QAction(tr("Delete"), this);
               menu.addAction(action);
               QAction::connect(action, &QAction::triggered, this, [this]() {
                  auto target = this->_selected_procedure_node_qmi();
                  if (target.isValid())
                     this->_models.procedure_tree->removeItem(target);
               });
            }
         }

         #pragma region Selected Procedure
            #pragma region Base procedure (node) properties
               {
                  auto* model = this->_models.procedure_params = new PackageProcedureParamsModel(this);
                  auto* view  = this->ui.currentProcedureInputs;
                  view->setModel(model);
                  view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
                  view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

                  model->setPackdataModel(this->_models.package_data);

                  auto* sel_model = view->selectionModel();
                  QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model, sel_model]() {
                     const auto rows = sel_model->selectedRows();
                     auto* picker = this->ui.currentProcedureInputPackdata;
                     if (rows.isEmpty()) {
                        picker->setEnabled(false);
                        return;
                     }
                     picker->setEnabled(true);
                     const auto qmi     = rows[0];
                     const auto blocker = QSignalBlocker(picker);

                     auto unique_id = model->data(qmi, PackageProcedureParamsModel::UniqueIDRole).toInt();
                     int  i = picker->findData(unique_id);
                     picker->setCurrentIndex(i);
                  });
               }
               QObject::connect(this->_models.package_data, &QAbstractItemModel::rowsInserted, this, &FormDialogPackage::_update_procedure_params_picker);
               QObject::connect(this->_models.package_data, &QAbstractItemModel::rowsRemoved, this, &FormDialogPackage::_update_procedure_params_picker);
               QObject::connect(this->_models.package_data, &QAbstractItemModel::dataChanged, this, &FormDialogPackage::_update_procedure_params_picker);
            #pragma endregion
            #pragma region Flag Overrides
            {
               using general_flag   = loaded_form_type::general_flag;
               using interrupt_flag = loaded_form_type::interrupt_flag;

               {
                  auto* widget = this->ui.procedureOverridePreferredSpeed;
                  widget->clear();
                  widget->addItem(tr("Walk", "preferred speed"), (int)dovah::packages::preferred_movement_speed::walk);
                  widget->addItem(tr("Fast Walk", "preferred speed"), (int)dovah::packages::preferred_movement_speed::fast_walk);
                  widget->addItem(tr("Jog", "preferred speed"), (int)dovah::packages::preferred_movement_speed::jog);
                  widget->addItem(tr("Run", "preferred speed"), (int)dovah::packages::preferred_movement_speed::run);

                  QObject::connect(this->ui.procedureOverrideFlagPreferredSpeed, &QCheckBox::toggled, widget, &QWidget::setEnabled);
                  widget->setEnabled(this->ui.procedureOverrideFlagPreferredSpeed->isChecked());
               }
            
               for (auto* widget : std::array{
                  this->ui.procedureOverrideFlagAllowSwim,
                  this->ui.procedureOverrideFlagAlwaysSneak,
                  this->ui.procedureOverrideFlagContinueIfPCNear,
                  this->ui.procedureOverrideFlagIgnoreCombat,
                  this->ui.procedureOverrideFlagMaintainSpeedAtGoal,
                  this->ui.procedureOverrideFlagMustComplete,
                  this->ui.procedureOverrideFlagNoCombatAlert,
                  this->ui.procedureOverrideFlagOffersServices,
                  this->ui.procedureOverrideFlagOncePerDay,
                  this->ui.procedureOverrideFlagWeaponDrawn,
                  this->ui.procedureOverrideFlagWeaponsUnequipped,
                  this->ui.procedureOverrideFlagWearSleepOutfit,
               }) {
                  widget->setProperty("is-interrupt-flag", false);
               }
               for (auto* widget : std::array{
                  this->ui.procedureOverrideFlagAggroRadius,
                  this->ui.procedureOverrideFlagAllowIdleChatter,
                  this->ui.procedureOverrideFlagFriendlyFireComments,
                  this->ui.procedureOverrideFlagHellosToPlayer,
                  this->ui.procedureOverrideFlagObserveCombatBehavior,
                  this->ui.procedureOverrideFlagRandomConversations,
                  this->ui.procedureOverrideFlagReactionToPlayerActions,
                  this->ui.procedureOverrideFlagWorldInteractions,
               }) {
                  widget->setProperty("is-interrupt-flag", true);
               }
               //
               this->ui.procedureOverrideFlagAllowSwim              ->setProperty("flag", general_flag::allow_swimming);
               this->ui.procedureOverrideFlagAlwaysSneak            ->setProperty("flag", general_flag::always_sneak);
               this->ui.procedureOverrideFlagContinueIfPCNear       ->setProperty("flag", general_flag::continue_if_player_near);
               this->ui.procedureOverrideFlagIgnoreCombat           ->setProperty("flag", general_flag::ignore_combat);
               this->ui.procedureOverrideFlagMaintainSpeedAtGoal    ->setProperty("flag", general_flag::maintain_speed_at_goal);
               this->ui.procedureOverrideFlagMustComplete           ->setProperty("flag", general_flag::must_complete);
               this->ui.procedureOverrideFlagNoCombatAlert          ->setProperty("flag", general_flag::no_combat_alert);
               this->ui.procedureOverrideFlagOffersServices         ->setProperty("flag", general_flag::offers_services);
               this->ui.procedureOverrideFlagOncePerDay             ->setProperty("flag", general_flag::once_per_day);
               this->ui.procedureOverrideFlagPreferredSpeed         ->setProperty("flag", general_flag::has_preferred_speed);
               this->ui.procedureOverrideFlagWeaponDrawn            ->setProperty("flag", general_flag::weapon_drawn);
               this->ui.procedureOverrideFlagWeaponsUnequipped      ->setProperty("flag", general_flag::weapons_unequipped);
               this->ui.procedureOverrideFlagWearSleepOutfit        ->setProperty("flag", general_flag::wear_sleep_outfit);
               //this->ui.procedureOverridePreferredSpeed->setProperty("flag", interrupt_flag::aggro_radius_behavior);
               //
               this->ui.procedureOverrideFlagAggroRadius            ->setProperty("flag", interrupt_flag::aggro_radius_behavior);
               this->ui.procedureOverrideFlagAllowIdleChatter       ->setProperty("flag", interrupt_flag::allow_idle_chatter);
               this->ui.procedureOverrideFlagFriendlyFireComments   ->setProperty("flag", interrupt_flag::friendly_fire_comments);
               this->ui.procedureOverrideFlagHellosToPlayer         ->setProperty("flag", interrupt_flag::hellos_to_player);
               this->ui.procedureOverrideFlagObserveCombatBehavior  ->setProperty("flag", interrupt_flag::observe_combat);
               this->ui.procedureOverrideFlagRandomConversations    ->setProperty("flag", interrupt_flag::random_conversations);
               this->ui.procedureOverrideFlagReactionToPlayerActions->setProperty("flag", interrupt_flag::react_to_player_actions);
               this->ui.procedureOverrideFlagWorldInteractions      ->setProperty("flag", interrupt_flag::world_interactions);

               for (auto* widget : std::array{
                  this->ui.procedureOverrideFlagAggroRadius,
                  this->ui.procedureOverrideFlagAllowIdleChatter,
                  this->ui.procedureOverrideFlagAllowSwim,
                  this->ui.procedureOverrideFlagAlwaysSneak,
                  this->ui.procedureOverrideFlagContinueIfPCNear,
                  this->ui.procedureOverrideFlagFriendlyFireComments,
                  this->ui.procedureOverrideFlagHellosToPlayer,
                  this->ui.procedureOverrideFlagIgnoreCombat,
                  this->ui.procedureOverrideFlagMaintainSpeedAtGoal,
                  this->ui.procedureOverrideFlagMustComplete,
                  this->ui.procedureOverrideFlagNoCombatAlert,
                  this->ui.procedureOverrideFlagObserveCombatBehavior,
                  this->ui.procedureOverrideFlagOffersServices,
                  this->ui.procedureOverrideFlagOncePerDay,
                  this->ui.procedureOverrideFlagRandomConversations,
                  this->ui.procedureOverrideFlagReactionToPlayerActions,
                  this->ui.procedureOverrideFlagWeaponDrawn,
                  this->ui.procedureOverrideFlagWeaponsUnequipped,
                  this->ui.procedureOverrideFlagWearSleepOutfit,
                  this->ui.procedureOverrideFlagWorldInteractions,
               }) {
                  QObject::connect(widget, &DKYesNoUnsetWidget::stateChanged, this, [this]() {
                     this->_push_procedure_flag_overrides_from_ui();
                  });
               }
               QObject::connect(this->ui.procedureOverrideFlagPreferredSpeed, &QCheckBox::toggled, this, [this]() {
                  this->_push_procedure_flag_overrides_from_ui();
               });
               QObject::connect(this->ui.procedureOverridePreferredSpeed, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
                  this->_push_procedure_flag_overrides_from_ui();
               });
            }
            #pragma endregion
         #pragma endregion
      }
   #pragma endregion
   #pragma region Flags
   {
      {
         auto* widget = this->ui.preferredSpeed;
         widget->clear();
         widget->addItem(tr("Walk", "preferred speed"), (int)dovah::packages::preferred_movement_speed::walk);
         widget->addItem(tr("Fast Walk", "preferred speed"), (int)dovah::packages::preferred_movement_speed::fast_walk);
         widget->addItem(tr("Jog", "preferred speed"), (int)dovah::packages::preferred_movement_speed::jog);
         widget->addItem(tr("Run", "preferred speed"), (int)dovah::packages::preferred_movement_speed::run);

         QObject::connect(this->ui.flagPreferredSpeed, &QCheckBox::toggled, widget, &QWidget::setEnabled);
         widget->setEnabled(this->ui.flagPreferredSpeed->isChecked());
      }

      this->ui.buttonClearAllInterruptFlags->setProperty("operation", false);
      this->ui.buttonSetAllInterruptFlags->setProperty("operation", true);
      auto handler = [this]() {
         bool op = sender()->property("operation").toBool();
         for (auto* widget : std::array{
            this->ui.flagAllowIdleChatter,
            this->ui.flagHellosToPlayer,
            this->ui.flagRandomConversations,
            this->ui.flagObserveCombatBehavior,
            this->ui.flagObserveCorpseBehavior,
            this->ui.flagReactionToPlayerActions,
            this->ui.flagFriendlyFireComments,
            this->ui.flagAggroRadius,
            this->ui.flagWorldInteractions,
         }) {
            widget->setChecked(op);
         }
      };
      QObject::connect(this->ui.buttonClearAllInterruptFlags, &QPushButton::clicked, this, handler);
      QObject::connect(this->ui.buttonSetAllInterruptFlags, &QPushButton::clicked, this, handler);
   }
   #pragma endregion
   #pragma region Schedule and Conditions
   {
      using schedule_type = decltype(loaded_form_type::schedule);
      {  // Weekday
         using enumeration = schedule_type::schedule_weekday;
         auto* widget      = this->ui.scheduleWeekday;
         widget->clear();
         for (const auto& pair : std::array{
            std::pair{ enumeration::any,                     tr("Any",          "weekday") },
            std::pair{ enumeration::monday,                  tr("Monday",       "weekday") },
            std::pair{ enumeration::tuesday,                 tr("Tuesday",      "weekday") },
            std::pair{ enumeration::wednesday,               tr("Wednesday",    "weekday") },
            std::pair{ enumeration::thursday,                tr("Thursday",     "weekday") },
            std::pair{ enumeration::friday,                  tr("Friday",       "weekday") },
            std::pair{ enumeration::saturday,                tr("Saturday",     "weekday") },
            std::pair{ enumeration::sunday,                  tr("Sunday",       "weekday") },
            std::pair{ enumeration::all_weekdays,            tr("All Weekdays", "weekday") },
            std::pair{ enumeration::all_weekends,            tr("All Weekends", "weekday") },
            std::pair{ enumeration::monday_wednesday_friday, tr("Mon/Wed/Fri",  "weekday") },
            std::pair{ enumeration::tuesday_thursday,        tr("Tue/Thu",      "weekday") },
         }) {
            widget->addItem(pair.second, (int)pair.first);
         }
      }
      {  // Month
         using enumeration = schedule_type::schedule_month;
         auto* widget      = this->ui.scheduleMonth;
         widget->clear();
         for (const auto& pair : std::array{
            std::pair{ enumeration::any,       tr("Any",       "month") },
            std::pair{ enumeration::january,   tr("January",   "month") },
            std::pair{ enumeration::february,  tr("February",  "month") },
            std::pair{ enumeration::march,     tr("March",     "month") },
            std::pair{ enumeration::april,     tr("April",     "month") },
            std::pair{ enumeration::may,       tr("May",       "month") },
            std::pair{ enumeration::june,      tr("June",      "month") },
            std::pair{ enumeration::july,      tr("July",      "month") },
            std::pair{ enumeration::august,    tr("August",    "month") },
            std::pair{ enumeration::september, tr("September", "month") },
            std::pair{ enumeration::october,   tr("October",   "month") },
            std::pair{ enumeration::november,  tr("November",  "month") },
            std::pair{ enumeration::december,  tr("December",  "month") },
            std::pair{ enumeration::spring,    tr("Spring (Mar/Apr/May)", "month") },
            std::pair{ enumeration::summer,    tr("Summer (Jun/Jul/Aug)", "month") },
            std::pair{ enumeration::autumn,    tr("Autumn (Sep/Oct/Nov)", "month") },
            std::pair{ enumeration::winter,    tr("Winter (Dec/Jan/Feb)", "month") },
         }) {
            widget->addItem(pair.second, (int)pair.first);
         }
      }
      {  // Day
         auto* widget = this->ui.scheduleDate;
         widget->clear();
         widget->addItem(tr("Any", "date"), 0);
         for (int i = 1; i <= 31; ++i) {
            widget->addItem(QString::number(i), i);
         }
      }
      {  // Hour
         auto* widget = this->ui.scheduleHour;
         widget->clear();
         widget->addItem(tr("Any", "hour"), -1);
         for (int i = 0; i < 24; ++i) {
            widget->addItem(QString::number(i), i);
         }
      }
      {  // Minute
         auto* widget = this->ui.scheduleMinute;
         widget->clear();
         widget->addItem(tr("Any", "minute"), -1);
         for (int i = 0; i < 59; ++i) {
            widget->addItem(tr("%1", "minute").arg(i, 2, QChar('0')), i);
         }
      }
   }
   #pragma endregion
   #pragma region Begin/End/Change
      this->ui.onBeginIdle->setAllowedFormType(dovah::form_type::idle);
      this->ui.onEndIdle->setAllowedFormType(dovah::form_type::idle);
      this->ui.onChangeIdle->setAllowedFormType(dovah::form_type::idle);
   #pragma endregion

   this->load(); // this creates the working copy.
}
void FormDialogPackage::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   if (!_get_custom_package_data()) {
      if (working.typed_info) {
         working.convert_to_modern();
         if (!working.typed_info) {
            working.typed_info = new dovah::loaded_forms::structs::typed_package_info::custom;
         }
      } else {
         working.typed_info = new custom_package_data;
      }
   }

   ui::bind(this->ui.editorID, this->editor_id());
   {
      auto* widget  = this->ui.flagIsTemplate;
      bool  checked = working.type == dovah::packages::legacy_type::custom_template;
      // order of operations here is significant: we WANT to trigger the change handler when setting to checked:
      QObject::connect(widget, &QCheckBox::toggled, this, &FormDialogPackage::_set_is_package_template);
      widget->setChecked(checked);
   }
   ui::bind(this->ui.owningQuest, working.owning_quest, working);
   ui::bind(this->ui.combatStyle, working.combat_style, working);
   ui::bind(this->ui.interruptOverride, working.interrupt_override);

   #pragma region Package
   {
      auto* custom = _get_custom_package_data();
      assert(custom != nullptr);
      auto* template_data = _get_template_package_data();
      
      ui::bind(this->ui.templateForm, custom->template_package, working);
      QObject::connect(this->ui.templateForm, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
         if (stub) {
            this->ui.buttonPackdataNew->setEnabled(false);
            this->_models.package_data->clear();
            this->_models.package_data->setDeclarationsOwned(false);
            this->_models.package_data->setOwningQuest(this->ui.owningQuest->formStub());
            auto* template_data = _get_template_package_data();
            if (template_data)
               this->_models.package_data->importDeclarations(template_data->data.declarations, false);
         } else {
            this->ui.buttonPackdataNew->setEnabled(true);
            this->_models.package_data->setDeclarationsOwned(true);
         }
      });
      #pragma region Public Package Data
         this->_models.package_data->setOwningQuest(working.owning_quest.get_form_stub());
         QObject::connect(this->ui.owningQuest, &DKFormPicker::formChanged, this->_models.package_data, &PackageDataModel::setOwningQuest);

         if (custom->template_package) {
            if (template_data) {
               this->_models.package_data->importDeclarations(template_data->data.declarations, false);
            }
         } else {
            this->_models.package_data->importDeclarations(custom->data.declarations, true);
         }
         this->_models.package_data->importValues(custom->data.values);
      #pragma endregion
      #pragma region Procedure Tree
         this->_models.procedure_tree->import_tree(custom->procedures);
      #pragma endregion
   }
   #pragma endregion
   #pragma region Flags
   {
      ui::bind(this->ui.flagMustComplete, working.general_flags, loaded_form_type::general_flag::must_complete);
      ui::bind(this->ui.flagOncePerDay, working.general_flags, loaded_form_type::general_flag::once_per_day);
      ui::bind(this->ui.flagAlwaysSneak, working.general_flags, loaded_form_type::general_flag::always_sneak);
      ui::bind(this->ui.flagAllowSwim, working.general_flags, loaded_form_type::general_flag::allow_swimming);
      ui::bind(this->ui.flagIgnoreCombat, working.general_flags, loaded_form_type::general_flag::ignore_combat);
      ui::bind(this->ui.flagNoCombatAlert, working.general_flags, loaded_form_type::general_flag::no_combat_alert);
      ui::bind(this->ui.flagWeaponDrawn, working.general_flags, loaded_form_type::general_flag::weapon_drawn);
      ui::bind(this->ui.flagWeaponsUnequipped, working.general_flags, loaded_form_type::general_flag::weapons_unequipped);
      ui::bind(this->ui.flagWearSleepOutfit, working.general_flags, loaded_form_type::general_flag::wear_sleep_outfit);
      ui::bind(this->ui.flagMaintainSpeedAtGoal, working.general_flags, loaded_form_type::general_flag::maintain_speed_at_goal);
      ui::bind(this->ui.flagPreferredSpeed, working.general_flags, loaded_form_type::general_flag::has_preferred_speed);
      ui::bind(this->ui.preferredSpeed, working.preferred_speed);

      ui::bind(this->ui.flagAllowIdleChatter, working.interrupt_flags, loaded_form_type::interrupt_flag::allow_idle_chatter);
      ui::bind(this->ui.flagHellosToPlayer, working.interrupt_flags, loaded_form_type::interrupt_flag::hellos_to_player);
      ui::bind(this->ui.flagRandomConversations, working.interrupt_flags, loaded_form_type::interrupt_flag::random_conversations);
      ui::bind(this->ui.flagObserveCombatBehavior, working.interrupt_flags, loaded_form_type::interrupt_flag::observe_combat);
      ui::bind(this->ui.flagObserveCorpseBehavior, working.interrupt_flags, loaded_form_type::interrupt_flag::observe_corpse);
      ui::bind(this->ui.flagReactionToPlayerActions, working.interrupt_flags, loaded_form_type::interrupt_flag::react_to_player_actions);
      ui::bind(this->ui.flagFriendlyFireComments, working.interrupt_flags, loaded_form_type::interrupt_flag::friendly_fire_comments);
      ui::bind(this->ui.flagAggroRadius, working.interrupt_flags, loaded_form_type::interrupt_flag::aggro_radius_behavior);
      ui::bind(this->ui.flagWorldInteractions, working.interrupt_flags, loaded_form_type::interrupt_flag::world_interactions);

      ui::bind(this->ui.flagUnlockDoorsAtStart, working.general_flags, loaded_form_type::general_flag::unlock_doors_at_start);
      ui::bind(this->ui.flagUnlockDoorsAtEnd,   working.general_flags, loaded_form_type::general_flag::unlock_doors_at_end);
   }
   #pragma endregion
   #pragma region Schedule and Conditions
      #pragma region Schedule
         ui::bind(this->ui.scheduleWeekday, working.schedule.weekday);
         ui::bind(this->ui.scheduleMonth, working.schedule.month);
         {
            auto _bind_integral = [this]<typename Target>(QComboBox* widget, Target & target) {
               {
                  auto i = widget->findData(target);
                  if (i >= 0)
                     widget->setCurrentIndex(i);
                  else
                     widget->setCurrentIndex(0);
               }
               QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [widget, &target]() {
                  target = widget->currentData().toInt();
               });
            };
            _bind_integral(this->ui.scheduleDate,   working.schedule.day);
            _bind_integral(this->ui.scheduleHour,   working.schedule.hour);
            _bind_integral(this->ui.scheduleMinute, working.schedule.minute);
         }
         {
            auto& target = working.schedule.duration; // seconds
            auto* widget = this->ui.scheduleDuration; // hours
            widget->setValue((float)target / 60 / 60);
            QObject::connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, &target](double v) {
               target = v * 60 * 60;
            });
         }
      #pragma endregion
      this->ui.conditions->importFrom(working, working.conditions);
   #pragma endregion
   #pragma region Begin/End/Change
   {
      this->ui.onBeginFragment->setSourceWidget(this->ui.scriptListPane);
      this->ui.onEndFragment->setSourceWidget(this->ui.scriptListPane);
      this->ui.onChangeFragment->setSourceWidget(this->ui.scriptListPane);
      if (auto* casted = dynamic_cast<dovah::loaded_forms::components::papyrus::package_fragment_data*>(working.script_data.fragment_data)) {
         if (auto& opt = casted->fragments.on_begin; opt.has_value()) {
            auto* widget = this->ui.onBeginFragment;
            auto& src    = opt.value();
            widget->setCurrentScriptname(src.script);
            widget->setCurrentFunction(src.function);
         }
         if (auto& opt = casted->fragments.on_end; opt.has_value()) {
            auto* widget = this->ui.onEndFragment;
            auto& src    = opt.value();
            widget->setCurrentScriptname(src.script);
            widget->setCurrentFunction(src.function);
         }
         if (auto& opt = casted->fragments.on_change; opt.has_value()) {
            auto* widget = this->ui.onChangeFragment;
            auto& src    = opt.value();
            widget->setCurrentScriptname(src.script);
            widget->setCurrentFunction(src.function);
         }
      }
      auto _setup_event = [this, &working](
         decltype(loaded_form_type::events.begin)& backend,
         DKFormPicker*           idle_picker,
         DKTopicOrSubtypePicker* topic_picker
      ) {
         ui::bind(idle_picker, backend.idle, working);
         {
            auto& data = backend.topic.data;
            if (std::holds_alternative<uint32_t>(data)) {
               topic_picker->setSubtype(std::get<uint32_t>(data));
            } else {
               topic_picker->setTopic(std::get<dovah::form_reference_t>(data).get_form_stub());
            }
            //
            QObject::connect(topic_picker, &DKTopicOrSubtypePicker::valueChanged, this, [this, &data](dovah::form_stub* topic, uint32_t subtype) {
               if (topic) {
                  if (!std::holds_alternative<dovah::form_reference_t>(data)) {
                     data.emplace<dovah::form_reference_t>();
                  }
                  std::get<dovah::form_reference_t>(data).set(*this->form, topic);
               } else {
                  if (std::holds_alternative<dovah::form_reference_t>(data)) {
                     std::get<dovah::form_reference_t>(data).set(*this->form, nullptr);
                  }
                  data.emplace<uint32_t>() = subtype;
               }
            });
         }
      };
      _setup_event(working.events.begin,  this->ui.onBeginIdle,  this->ui.onBeginTopic);
      _setup_event(working.events.end,    this->ui.onEndIdle,    this->ui.onEndTopic);
      _setup_event(working.events.change, this->ui.onChangeIdle, this->ui.onChangeTopic);

      this->ui.scriptListPane->setFormWorkingCopy(&working);
   }
   #pragma endregion
   #pragma region Idles
      this->ui.idles->importData(working.idles);
   #pragma endregion

   this->_on_packdata_selection_changed();
   this->_pull_procedure_node_to_ui();
}
void FormDialogPackage::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_push_procedure_node_from_ui();

   #pragma region Package
   {  // Package data
      auto* custom = _get_custom_package_data();
      assert(custom != nullptr);

      this->_models.package_data->exportValues(custom->data.values, working);
      if (!custom->template_package) {
         this->_models.package_data->exportDeclarations(custom->data.declarations, working);
         this->_models.procedure_tree->export_tree(custom->procedures, working);
      }
   }
   #pragma endregion
   #pragma region Schedule and Conditions
      this->ui.conditions->exportTo(working, working.conditions);
   #pragma endregion
   #pragma region Begin/End/Change
      {
         using fragment_data_type   = dovah::loaded_forms::components::papyrus::package_fragment_data;
         using single_fragment_type = dovah::loaded_forms::components::papyrus::basic_fragment;

         auto* casted = dynamic_cast<fragment_data_type*>(working.script_data.fragment_data);
         assert((casted || !working.script_data.fragment_data) && "Fragment data for another form type shouldn't be here!");

         auto _commit = [&working, &casted](
            DKPapyrusFragmentFunctionPicker* widget,
            std::optional<single_fragment_type> decltype(fragment_data_type::fragments)::* target_ptr
         ) {
            auto scriptname = widget->currentScriptname();
            auto function   = widget->currentFunction();
            if (!scriptname.isEmpty() || !function.isEmpty()) {
               if (!casted) {
                  working.script_data.fragment_data = casted = new fragment_data_type;
               }
               auto& target = ((&casted->fragments)->*target_ptr);
               target.emplace();
               target.value().script   = scriptname.toStdString();
               target.value().function = function.toStdString();
            } else {
               if (casted) {
                  auto& target = ((&casted->fragments)->*target_ptr);
                  target.reset();
               }
            }
         };
         _commit(
            this->ui.onBeginFragment,
            &decltype(fragment_data_type::fragments)::on_begin
         );
         _commit(
            this->ui.onEndFragment,
            &decltype(fragment_data_type::fragments)::on_end
         );
         _commit(
            this->ui.onChangeFragment,
            &decltype(fragment_data_type::fragments)::on_change
         );
      }
      this->ui.scriptListPane->commit();
   #pragma endregion
   #pragma region Idles
      this->ui.idles->exportData(working.idles, working);
   #pragma endregion
}

FormDialogPackage::custom_package_data* FormDialogPackage::_get_custom_package_data() {
   auto& working = *this->form;
   return dynamic_cast<custom_package_data*>(working.typed_info);
}
FormDialogPackage::custom_package_data* FormDialogPackage::_get_template_package_data() {
   auto* custom = this->_get_custom_package_data();
   if (!custom)
      return nullptr;
   auto* tmpl = custom->template_package.get_form_stub();
   if (!tmpl)
      return nullptr;
   auto loaded = tmpl->load().ptr_cast<loaded_form_type>();
   if (!loaded)
      return nullptr;
   return dynamic_cast<custom_package_data*>(loaded->typed_info);
}
void FormDialogPackage::_set_is_package_template(bool is) {
   this->ui.templateForm->setEnabled(!is);

   auto& working = *this->form;
   if (is) {
      working.type = dovah::packages::legacy_type::custom_template;
      this->ui.templateForm->setFormStub(nullptr);
      this->ui.templateForm->setEnabled(false);
   } else {
      working.type = dovah::packages::legacy_type::custom;
      this->ui.templateForm->setEnabled(true);
   }
}

std::optional<size_t> FormDialogPackage::_selected_packdata_row() const {
   auto* view      = this->ui.packdata;
   auto* sel_model = view->selectionModel();
   auto  rows      = sel_model->selectedRows();
   if (rows.isEmpty())
      return {};
   auto row = rows[0].row();
   if (row < 0 || row >= this->_models.package_data->rowCount())
      return {};
   return row;
}
void FormDialogPackage::_on_packdata_selection_changed() {
   const auto* model = this->_models.package_data;

   const auto row_opt = _selected_packdata_row();
   if (!row_opt.has_value()) {
      this->ui.buttonPackdataMoveUp->setEnabled(false);
      this->ui.buttonPackdataMoveDown->setEnabled(false);
      this->ui.buttonPackdataDelete->setEnabled(false);
      this->ui.currentPackdataName->setEnabled(false);
      this->ui.currentPackdataType->setEnabled(false);
      this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_None);
      this->ui.currentPackdataIsPublic->setEnabled(false);
      return;
   }
   const auto row = row_opt.value();
   this->ui.buttonPackdataMoveUp->setEnabled(true);
   this->ui.buttonPackdataMoveDown->setEnabled(true);
   this->_update_packdata_deleteable();

   const auto blockers = std::array{
      QSignalBlocker(this->ui.currentPackdataName),
      QSignalBlocker(this->ui.currentPackdataType),
      QSignalBlocker(this->ui.currentPackdataIsPublic),
      QSignalBlocker(this->ui.currentPackdataValue_Bool),
      QSignalBlocker(this->ui.currentPackdataValue_Float),
      QSignalBlocker(this->ui.currentPackdataValue_Int),
      QSignalBlocker(this->ui.currentPackdataValue_LocationRadius),
      QSignalBlocker(this->ui.currentPackdataValue_TargetRadius),
      QSignalBlocker(this->ui.currentPackdataValue_Topic),
   };

   const bool owned = model->declarationsOwned();

   auto decl    = model->rowDeclaration(row);
   auto val_opt = model->rowValue(row);

   this->ui.currentPackdataName->setEnabled(owned);
   this->ui.currentPackdataType->setEnabled(owned);
   this->ui.currentPackdataIsPublic->setEnabled(owned);

   this->ui.currentPackdataName->setText(decl.name);
   this->ui.currentPackdataIsPublic->setChecked(decl.is_public);
   if (!val_opt.has_value()) {
      //
      // Default to "bool."
      //
      this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Bool);
      this->ui.currentPackdataValue_Bool->setChecked(false);
      return;
   }
   auto& val  = val_opt.value();
   auto  type = val.type();
   {
      auto i = this->ui.currentPackdataType->findData((int)type);
      if (i >= 0)
         this->ui.currentPackdataType->setCurrentIndex(i);
   }
   switch (type) {
      case dovah::packages::package_data_type::invalid:
         //
         // Default to "bool."
         //
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Bool);
         this->ui.currentPackdataValue_Bool->setChecked(false);
         break;
      case dovah::packages::package_data_type::boolean:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Bool);
         this->ui.currentPackdataValue_Bool->setChecked(val.as<dovah::packages::package_data_type::boolean>());
         break;
      case dovah::packages::package_data_type::float32:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Float);
         this->ui.currentPackdataValue_Float->setValue(val.as<dovah::packages::package_data_type::float32>());
         break;
      case dovah::packages::package_data_type::integer:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Int);
         this->ui.currentPackdataValue_Int->setValue(val.as<dovah::packages::package_data_type::integer>());
         break;
      case dovah::packages::package_data_type::location:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Location);
         {
            auto& src = val.as<dovah::packages::package_data_type::location>();
            this->ui.currentPackdataValue_LocationRadius->setValue(src.radius);
            this->ui.currentPackdataValue_LocationButtonEdit->setText(
               model->data(model->index(row, PackageDataModel::Column::Value, {}), Qt::DisplayRole).toString()
            );
         }
         break;
      case dovah::packages::package_data_type::object_list:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Float);
         this->ui.currentPackdataValue_Float->setValue(val.as<dovah::packages::package_data_type::object_list>());
         break;
      case dovah::packages::package_data_type::single_ref:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Target);
         {
            auto& src = val.as<dovah::packages::package_data_type::single_ref>();
            this->ui.currentPackdataValue_TargetRadius->setValue(src.distance);
            this->ui.currentPackdataValue_TargetButtonEdit->setText(
               model->data(model->index(row, PackageDataModel::Column::Value, {}), Qt::DisplayRole).toString()
            );
         }
         break;
      case dovah::packages::package_data_type::target_selector:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Target);
         {
            auto& src = val.as<dovah::packages::package_data_type::target_selector>();
            this->ui.currentPackdataValue_TargetRadius->setValue(src.distance);
            this->ui.currentPackdataValue_TargetButtonEdit->setText(
               model->data(model->index(row, PackageDataModel::Column::Value, {}), Qt::DisplayRole).toString()
            );
         }
         break;
      case dovah::packages::package_data_type::topic:
         this->ui.currentPackdataValueHolder->setCurrentWidget(this->ui.currentPackdataValuePage_Topic);
         {
            auto& src = val.as<dovah::packages::package_data_type::topic>();
            if (auto* topic = src.get_topic())
               this->ui.currentPackdataValue_Topic->setTopic(topic);
            else
               this->ui.currentPackdataValue_Topic->setSubtype(src.get_subtype_signature());
         }
         break;
   }
}
void FormDialogPackage::_on_packdata_declaration_edited() {
   auto* model = this->_models.package_data;

   const auto row_opt = _selected_packdata_row();
   if (!row_opt.has_value())
      return;
   auto row = row_opt.value();

   auto decl = model->rowDeclaration(row);
   decl.name      = this->ui.currentPackdataName->text();
   decl.is_public = this->ui.currentPackdataIsPublic->isChecked();
   model->setRowDeclaration(row, decl);
}
void FormDialogPackage::_on_packdata_type_edited() {
   auto* model = this->_models.package_data;

   const auto row_opt = _selected_packdata_row();
   if (!row_opt.has_value())
      return;
   auto row = row_opt.value();

   auto type = (dovah::packages::package_data_type)this->ui.currentPackdataType->currentData().toInt();

   ui::types::packages::package_data_value value;
   {
      auto opt = model->rowValue(row);
      if (opt.has_value())
         value = opt.value();
   }
   if (value.type() != type)
      value.convert_to(type);

   this->_on_packdata_selection_changed();
}
void FormDialogPackage::_on_packdata_value_edited() {
   auto* model = this->_models.package_data;

   const auto row_opt = _selected_packdata_row();
   if (!row_opt.has_value())
      return;
   const auto row = row_opt.value();

   auto type = (dovah::packages::package_data_type)this->ui.currentPackdataType->currentData().toInt();

   ui::types::packages::package_data_value value;
   switch (type) {
      case dovah::packages::package_data_type::boolean:
         value.emplace<dovah::packages::package_data_type::boolean>() = this->ui.currentPackdataValue_Bool->isChecked();
         break;
      case dovah::packages::package_data_type::float32:
         value.emplace<dovah::packages::package_data_type::float32>() = this->ui.currentPackdataValue_Float->value();
         break;
      case dovah::packages::package_data_type::integer:
         value.emplace<dovah::packages::package_data_type::integer>() = this->ui.currentPackdataValue_Int->value();
         break;
      case dovah::packages::package_data_type::location:
         {
            auto& dst = value.emplace<dovah::packages::package_data_type::location>();
            dst.radius = this->ui.currentPackdataValue_LocationRadius->value();
         }
         break;
      case dovah::packages::package_data_type::single_ref:
         {
            auto& dst = value.emplace<dovah::packages::package_data_type::single_ref>();
            dst.distance = this->ui.currentPackdataValue_TargetRadius->value();
         }
         break;
      case dovah::packages::package_data_type::target_selector:
         {
            auto& dst = value.emplace<dovah::packages::package_data_type::target_selector>();
            dst.distance = this->ui.currentPackdataValue_TargetRadius->value();
         }
         break;
      case dovah::packages::package_data_type::topic:
         {
            auto* src = this->ui.currentPackdataValue_Topic;
            auto& dst = value.emplace<dovah::packages::package_data_type::topic>();
            if (auto* topic = src->topic())
               dst.data = topic;
            else
               dst.data = src->subtype();
         }
         break;
   }

   model->setRowValue(row, value);
}

QModelIndex FormDialogPackage::_selected_procedure_node_qmi() const {
   auto* view      = this->ui.procedures;
   auto* sel_model = view->selectionModel();
   auto  rows      = sel_model->selectedRows();
   if (!rows.isEmpty())
      return rows[0];
   return {};
}
void FormDialogPackage::_pull_procedure_node_to_ui() {
   const auto* model = this->_models.procedure_tree;

   QModelIndex qmi = _selected_procedure_node_qmi();
   if (!qmi.isValid()) {
      this->ui.selectedProcedureGroupbox->setEnabled(false);
      this->ui.currentProcedureStack->setCurrentWidget(this->ui.currentProcedurePageProcedure);
      this->ui.currentProcedureConditions->clear();
      return;
   }

   this->ui.selectedProcedureGroupbox->setEnabled(true);

   const auto blockers = std::array{
      QSignalBlocker(this->ui.currentProcedureType),
      QSignalBlocker(this->ui.currentProcedureCompletesPackage),
      QSignalBlocker(this->ui.currentProcedureConditions),
      QSignalBlocker(this->ui.currentProcedureInputPackdata),
      QSignalBlocker(this->ui.currentProcedureRepeatWhenComplete),
      QSignalBlocker(this->ui.procedureOverrideFlagAggroRadius),
      QSignalBlocker(this->ui.procedureOverrideFlagAllowIdleChatter),
      QSignalBlocker(this->ui.procedureOverrideFlagAllowSwim),
      QSignalBlocker(this->ui.procedureOverrideFlagAlwaysSneak),
      QSignalBlocker(this->ui.procedureOverrideFlagContinueIfPCNear),
      QSignalBlocker(this->ui.procedureOverrideFlagFriendlyFireComments),
      QSignalBlocker(this->ui.procedureOverrideFlagHellosToPlayer),
      QSignalBlocker(this->ui.procedureOverrideFlagIgnoreCombat),
      QSignalBlocker(this->ui.procedureOverrideFlagMaintainSpeedAtGoal),
      QSignalBlocker(this->ui.procedureOverrideFlagMustComplete),
      QSignalBlocker(this->ui.procedureOverrideFlagNoCombatAlert),
      QSignalBlocker(this->ui.procedureOverrideFlagObserveCombatBehavior),
      QSignalBlocker(this->ui.procedureOverrideFlagOffersServices),
      QSignalBlocker(this->ui.procedureOverrideFlagOncePerDay),
      QSignalBlocker(this->ui.procedureOverrideFlagPreferredSpeed),
      QSignalBlocker(this->ui.procedureOverrideFlagRandomConversations),
      QSignalBlocker(this->ui.procedureOverrideFlagReactionToPlayerActions),
      QSignalBlocker(this->ui.procedureOverrideFlagWeaponDrawn),
      QSignalBlocker(this->ui.procedureOverrideFlagWeaponsUnequipped),
      QSignalBlocker(this->ui.procedureOverrideFlagWearSleepOutfit),
      QSignalBlocker(this->ui.procedureOverrideFlagWorldInteractions),
      QSignalBlocker(this->ui.procedureOverridePreferredSpeed),
   };

   auto var_branch_type = model->data(qmi, PackageProcedureTreeModel::BranchTypeRole);
   if (var_branch_type.isValid()) {
      this->ui.currentProcedureStack->setCurrentWidget(this->ui.currentProcedurePageBranch);
      {
         auto* widget = this->ui.currentProcedureType;
         widget->clear();
         for (auto v : std::array{
            dovah::packages::procedure_tree_branch_type::random,
            dovah::packages::procedure_tree_branch_type::sequence,
            dovah::packages::procedure_tree_branch_type::simultaneous,
            dovah::packages::procedure_tree_branch_type::stacked,
            }) {
            widget->addItem(editor::localize::package_procedure_tree_branch_type(v), (int)v);
         }
         widget->model()->sort(0);
         auto i = widget->findData(var_branch_type.toInt());
         if (i >= 0)
            widget->setCurrentIndex(i);
      }

      auto flags = model->data(qmi, PackageProcedureTreeModel::BranchFlagsRole).toInt();
      {
         auto* widget = this->ui.currentProcedureRepeatWhenComplete;
         if ((dovah::packages::procedure_tree_branch_type)var_branch_type.toInt() == dovah::packages::procedure_tree_branch_type::simultaneous) {
            widget->setText(tr("Repeat until all child procedures complete"));
         } else {
            widget->setText(tr("Repeat when complete"));
         }
         widget->setChecked(flags & ui::types::packages::procedure_tree_typed_data::branch::flag::repeat_when_complete);
      }
   } else {
      this->ui.currentProcedureStack->setCurrentWidget(this->ui.currentProcedurePageProcedure);
      {
         auto* widget = this->ui.currentProcedureType;
         widget->clear();
         for (const auto& info : dovah::packages::all_procedure_type_info) {
            widget->addItem(editor::localize::package_procedure_type(info.type), (int)info.type);
         }
         widget->model()->sort(0);

         auto var_procedure_type = model->data(qmi, PackageProcedureTreeModel::ProcedureTypeRole);
         auto i = widget->findData(var_procedure_type.toInt());
         if (i >= 0)
            widget->setCurrentIndex(i);
      }

      auto flags = model->data(qmi, PackageProcedureTreeModel::ProcedureFlagsRole).toInt();
      this->ui.currentProcedureCompletesPackage->setChecked(flags & ui::types::packages::procedure_tree_typed_data::procedure::flag::success_completes_package);

      {
         const auto yes_no_widgets = std::array{
            this->ui.procedureOverrideFlagAggroRadius,
            this->ui.procedureOverrideFlagAllowIdleChatter,
            this->ui.procedureOverrideFlagAllowSwim,
            this->ui.procedureOverrideFlagAlwaysSneak,
            this->ui.procedureOverrideFlagContinueIfPCNear,
            this->ui.procedureOverrideFlagFriendlyFireComments,
            this->ui.procedureOverrideFlagHellosToPlayer,
            this->ui.procedureOverrideFlagIgnoreCombat,
            this->ui.procedureOverrideFlagMaintainSpeedAtGoal,
            this->ui.procedureOverrideFlagMustComplete,
            this->ui.procedureOverrideFlagNoCombatAlert,
            this->ui.procedureOverrideFlagObserveCombatBehavior,
            this->ui.procedureOverrideFlagOffersServices,
            this->ui.procedureOverrideFlagOncePerDay,
            this->ui.procedureOverrideFlagRandomConversations,
            this->ui.procedureOverrideFlagReactionToPlayerActions,
            this->ui.procedureOverrideFlagWeaponDrawn,
            this->ui.procedureOverrideFlagWeaponsUnequipped,
            this->ui.procedureOverrideFlagWearSleepOutfit,
            this->ui.procedureOverrideFlagWorldInteractions,
         };

         auto override_flags = model->data(qmi, PackageProcedureTreeModel::ProcedureOverrideFlagsRole);
         if (override_flags.isValid()) {
            auto data = override_flags.value<dovah::loaded_forms::structs::custom_packages::package_flag_overrides>();
            for (DKYesNoUnsetWidget* widget : yes_no_widgets) {
               const auto blocker = QSignalBlocker(widget);

               auto flag  = widget->property("flag").toInt();
               auto state = Qt::CheckState::PartiallyChecked; // "unchanged"

               if (widget->property("is-interrupt-flag").toBool()) {
                  if (data.interrupt.set & flag) {
                     state = Qt::CheckState::Checked;
                  } else if (data.interrupt.clear & flag) {
                     state = Qt::CheckState::Unchecked;
                  }
               } else {
                  if (data.general.set & flag) {
                     state = Qt::CheckState::Checked;
                  } else if (data.general.clear & flag) {
                     state = Qt::CheckState::Unchecked;
                  }
               }

               widget->setCheckState(state);
            }

            const auto blocker = QSignalBlocker(this->ui.procedureOverrideFlagPreferredSpeed);
            this->ui.procedureOverrideFlagPreferredSpeed->setChecked(data.general.set & loaded_form_type::general_flag::has_preferred_speed);

            {
               auto*      widget  = this->ui.procedureOverridePreferredSpeed;
               const auto blocker = QSignalBlocker(widget);

               auto speed = data.preferred_speed;
               auto i     = widget->findData((int)speed);
               if (i >= 0)
                  widget->setCurrentIndex(i);
            }
         } else {
            for (DKYesNoUnsetWidget* widget : yes_no_widgets) {
               const auto blocker = QSignalBlocker(widget);
               widget->setCheckState(Qt::CheckState::PartiallyChecked);
            }
            const auto blocker = QSignalBlocker(this->ui.procedureOverrideFlagPreferredSpeed);
            this->ui.procedureOverrideFlagPreferredSpeed->setChecked(false);
         }
      }

      this->_update_procedure_params_list(qmi);
      this->_update_procedure_params_picker();
   }

   auto conditions = model->nodeConditions(qmi);
   this->ui.conditions->importFrom(*this->form, conditions);
}
void FormDialogPackage::_push_procedure_flag_overrides_from_ui(QModelIndex qmi) {
   auto* model = this->_models.procedure_tree;

   if (!qmi.isValid()) {
      qmi = _selected_procedure_node_qmi();
      if (!qmi.isValid())
         return;
   }

   auto var_proc_type = model->data(qmi, PackageProcedureTreeModel::ProcedureTypeRole);
   if (!var_proc_type.isValid())
      return;
   
   const auto yes_no_widgets = std::array{
      this->ui.procedureOverrideFlagAggroRadius,
      this->ui.procedureOverrideFlagAllowIdleChatter,
      this->ui.procedureOverrideFlagAllowSwim,
      this->ui.procedureOverrideFlagAlwaysSneak,
      this->ui.procedureOverrideFlagContinueIfPCNear,
      this->ui.procedureOverrideFlagFriendlyFireComments,
      this->ui.procedureOverrideFlagHellosToPlayer,
      this->ui.procedureOverrideFlagIgnoreCombat,
      this->ui.procedureOverrideFlagMaintainSpeedAtGoal,
      this->ui.procedureOverrideFlagMustComplete,
      this->ui.procedureOverrideFlagNoCombatAlert,
      this->ui.procedureOverrideFlagObserveCombatBehavior,
      this->ui.procedureOverrideFlagOffersServices,
      this->ui.procedureOverrideFlagOncePerDay,
      this->ui.procedureOverrideFlagRandomConversations,
      this->ui.procedureOverrideFlagReactionToPlayerActions,
      this->ui.procedureOverrideFlagWeaponDrawn,
      this->ui.procedureOverrideFlagWeaponsUnequipped,
      this->ui.procedureOverrideFlagWearSleepOutfit,
      this->ui.procedureOverrideFlagWorldInteractions,
   };

   dovah::loaded_forms::structs::custom_packages::package_flag_overrides data;
   bool any = false;
   for (const auto* widget : yes_no_widgets) {
      const auto flag  = widget->property("flag").toInt();
      const auto state = widget->checkState();
      if (widget->property("is-interrupt-flag").toBool()) {
         switch (state) {
            case Qt::CheckState::Checked:
               any = true;
               data.interrupt.set |= flag;
               break;
            case Qt::CheckState::Unchecked:
               any = true;
               data.interrupt.clear |= flag;
               break;
         }
      } else {
         switch (state) {
            case Qt::CheckState::Checked:
               any = true;
               data.general.set |= flag;
               break;
            case Qt::CheckState::Unchecked:
               any = true;
               data.general.clear |= flag;
               break;
         }
      }
   }
   if (this->ui.procedureOverrideFlagPreferredSpeed->isChecked()) {
      any = true;
      data.general.set |= loaded_form_type::general_flag::has_preferred_speed;
      data.preferred_speed = (dovah::packages::preferred_movement_speed)this->ui.procedureOverridePreferredSpeed->currentData().toInt();
   }

   if (any) {
      model->setData(qmi, QVariant::fromValue(data), PackageProcedureTreeModel::ProcedureOverrideFlagsRole);
   } else {
      model->setData(qmi, {}, PackageProcedureTreeModel::ProcedureOverrideFlagsRole);
   }
}
void FormDialogPackage::_push_procedure_node_from_ui(QModelIndex qmi) {
   auto* model = this->_models.procedure_tree;

   if (!qmi.isValid()) {
      qmi = _selected_procedure_node_qmi();
      if (!qmi.isValid())
         return;
   }

   auto var_branch_type = model->data(qmi, PackageProcedureTreeModel::BranchTypeRole);
   if (var_branch_type.isValid()) {
      model->setData(qmi, this->ui.currentProcedureType->currentData().toInt(), PackageProcedureTreeModel::BranchTypeRole);

      int flags = 0;
      if (this->ui.currentProcedureRepeatWhenComplete->isChecked())
         flags |= ui::types::packages::procedure_tree_typed_data::branch::flag::repeat_when_complete;
      model->setData(qmi, flags, PackageProcedureTreeModel::BranchFlagsRole);
   } else {
      model->setData(qmi, this->ui.currentProcedureType->currentData().toInt(), PackageProcedureTreeModel::ProcedureTypeRole);

      int flags = 0;
      if (this->ui.currentProcedureCompletesPackage->isChecked())
         flags |= ui::types::packages::procedure_tree_typed_data::procedure::flag::success_completes_package;
      model->setData(qmi, flags, PackageProcedureTreeModel::ProcedureFlagsRole);

      this->_push_procedure_flag_overrides_from_ui(qmi);
      
      {
         auto param_ids = this->_models.procedure_params->exportData();
         model->setProcedureParameterIDs(qmi, param_ids);
      }
   }

   std::vector<ui::types::conditions::condition> conditions;
   this->ui.conditions->exportTo(*this->form, conditions);
   model->setNodeConditions(qmi, conditions);
}
void FormDialogPackage::_on_procedure_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected) {
   if (!deselected.empty()) {
      auto range = deselected[0];
      auto qmi   = range.topLeft();
      this->_push_procedure_node_from_ui(qmi);
   }
   this->_pull_procedure_node_to_ui();

   auto qmi      = _selected_procedure_node_qmi();
   bool any      = qmi.isValid();
   bool has_root = this->_models.procedure_tree->hasRoot();
   this->_context_menus.procedure_tree.remove->setEnabled(any);
   //
   // Can only create children of a selected node, or create a root if the model is empty:
   //
   this->_context_menus.procedure_tree.create_branch->setEnabled(any || !has_root);
   this->_context_menus.procedure_tree.create_procedure->setEnabled(any || !has_root);
}

void FormDialogPackage::_update_procedure_params_picker() {
   const auto blocker = QSignalBlocker(this->ui.currentProcedureInputPackdata);
   
   auto* packdata_model = this->_models.package_data;
   auto* widget         = this->ui.currentProcedureInputPackdata;
   auto  prior          = widget->currentData();
   widget->clear();
   size_t rows = packdata_model->rowCount();
   for (size_t i = 0; i < rows; ++i) {
      auto qmi       = packdata_model->index(i, 0, {});
      bool is_public = packdata_model->data(qmi, Qt::EditRole).toBool();
      if (!is_public)
         continue;
      widget->addItem(
         packdata_model->data(qmi.siblingAtColumn(PackageDataModel::Column::Name), Qt::DisplayRole).toString(),
         packdata_model->data(qmi, PackageDataModel::UniqueIDRole).toInt()
      );
   }
   widget->setEnabled(rows > 0 && !this->ui.currentProcedureInputs->selectionModel()->selectedRows().isEmpty());
   if (prior.isValid())
      widget->setCurrentIndex(widget->findData(prior));
}
void FormDialogPackage::_update_procedure_params_list(QModelIndex qmi) {
   if (!qmi.isValid()) {
      qmi = _selected_procedure_node_qmi();
      if (!qmi.isValid())
         return;
   }
   const auto blocker = QSignalBlocker(this->ui.currentProcedureInputPackdata);
   this->_models.procedure_params->importData(
      (dovah::packages::procedure_type)this->_models.procedure_tree->data(qmi, PackageProcedureTreeModel::ProcedureTypeRole).toInt(),
      this->_models.procedure_tree->getProcedureParameterIDs(qmi)
   );
}
void FormDialogPackage::_update_packdata_deleteable() const {
   if (!this->_models.package_data->declarationsOwned()) {
      this->ui.buttonPackdataDelete->setEnabled(false);
      return;
   }
   auto row_opt = _selected_packdata_row();
   if (!row_opt.has_value())
      return;
   auto    row       = row_opt.value();
   uint8_t unique_id = this->_models.package_data->data(this->_models.package_data->index(row, 0, {}), PackageDataModel::UniqueIDRole).toInt();

   auto usage = this->_models.procedure_tree->countUsesOfPackdata();
   if (usage[unique_id] > 0) {
      this->ui.buttonPackdataDelete->setEnabled(false);
   } else {
      this->ui.buttonPackdataDelete->setEnabled(true);
   }
}