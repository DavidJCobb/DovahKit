#include "./DKConditionEditDialog.h"
#include <optional>
#include <string_view>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QShowEvent>
#include <QStackedWidget>
#include <QStandardItemModel>
#include "widgets/DKFormPicker.h"
#include "widgets/DKCompactObjectReferencePicker.h"
#include "helpers/qt/basic_bindings.h"
#include "helpers/qt/spinbox.h"
#include "helpers/qt/strings.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "ui/generic/FormsOfTypeCombobox.h"
#include "ui/utils/bind.h"

#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/conditions/event_function.h"
#include "dovah/data/conditions/parameter_typeinfo.h"
#include "dovah/data/conditions/parameter_underlying_type.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/structs/typed_package_info/custom.h"
#include "dovah/forms/Package.h"
#include "dovah/forms/Quest.h"
#include "dovah/forms/Scene.h"

// for special cases: GetVMQuestVariable
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/papyrus/compiled_script.h"
#include "editor/subsystems/assets.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/papyrus/core.h"

namespace {
   constexpr int RunOnTypeRole           = Qt::ItemDataRole::UserRole;
   constexpr int RunOnFormIDRole         = Qt::ItemDataRole::UserRole + 1;
   constexpr int RunOnPlayerSentinelRole = Qt::ItemDataRole::UserRole + 2;

   using parameter_type_override = dovah::loaded_forms::components::conditions::parameter_type_override;

   namespace special_case_functions {
      constexpr const auto _lookup_function_id_by_name(std::string_view name) {
         for (const auto& info : dovah::conditions::all_vanilla_function_info)
            if (info.name == name)
               return info.id;
         throw;
      }

      constexpr const auto GetVMQuestVariable    = _lookup_function_id_by_name("GetVMQuestVariable");
      constexpr const auto GetVMScriptVariable   = _lookup_function_id_by_name("GetVMScriptVariable");
      constexpr const auto IsInCombat            = _lookup_function_id_by_name("IsInCombat");
      constexpr const auto IsLimbGone            = _lookup_function_id_by_name("IsLimbGone");
      constexpr const auto IsPlayerActionActive  = _lookup_function_id_by_name("IsPlayerActionActive");
      constexpr const auto IsSceneActionComplete = _lookup_function_id_by_name("IsSceneActionComplete");
   }
}

namespace {
   dovah::loaded_forms::structs::custom_packages::package_data_declaration_map* _get_package_data_declaration_list(dovah::loaded_forms::Package* package) {
      using modern_package_info = dovah::loaded_forms::structs::typed_package_info::custom;
      if (!package)
         return nullptr;

      auto* custom = dynamic_cast<modern_package_info*>(package->typed_info);
      if (!custom)
         return nullptr;
      //
      // First, check if the package has a template. If so, redirect our checks to that 
      // template.
      //
      if (auto* tp_stub = custom->template_package.get_form_stub(); tp_stub) {
         package = tp_stub->load().ptr_cast<dovah::loaded_forms::Package>();
         if (!package)
            return nullptr;
         custom = dynamic_cast<modern_package_info*>(package->typed_info);
         if (!custom)
            return nullptr;
      }
      return &custom->data.declarations;
   }
}

bool DKConditionEditDialog::_FunctionListProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
   auto* sm   = (QStandardItemModel*)this->sourceModel();
   auto* item = sm->item(source_row, 0);
   if (item) {
      if (item->data(ExcludeRole).toBool())
         return true;
   }
   return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

DKConditionEditDialog::DKConditionEditDialog(dovah::form_stub& containing_form, const Condition& wc, QWidget* parent) : QDialog(parent), _context(containing_form), _value(wc) {
   ui.setupUi(this);
   //
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->reject();
   });
   //
   #pragma region Create parameter widgets
   this->parameters[0].parent = this->ui.param1Holder;
   this->parameters[1].parent = this->ui.param2Holder;
   this->parameters[2].parent = this->ui.param3Holder;
   for (size_t i = 0; i < this->parameters.size(); ++i) {
      auto& param  = this->parameters[i];
      auto* layout = new QGridLayout(param.parent);
      layout->setMargin(0);
      param.parent->setLayout(layout);

      param.stack = new QStackedWidget(param.parent);
      layout->addWidget(param.stack);

      param.blank    = new QWidget(param.parent);
      param.combobox = new QComboBox(param.parent);
      param.form     = new DKFormPicker(param.parent);
      param.ref      = new DKCompactObjectReferencePicker(param.parent);
      param.spinbox  = new QDoubleSpinBox(param.parent);
      param.textbox  = new QLineEdit(param.parent);
      param.stack->addWidget(param.blank);
      param.stack->addWidget(param.combobox);
      param.stack->addWidget(param.form);
      param.stack->addWidget(param.ref);
      param.stack->addWidget(param.spinbox);
      param.stack->addWidget(param.textbox);

      param.stack->setCurrentWidget(param.blank);

      param.combobox->setEditable(false);
      param.combobox->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);

      QObject::connect(param.combobox, QOverload<int>::of(&QComboBox::activated), this, [this, i, stack = param.stack](int index) {
         //
         // NOTE:
         // 
         // Normally, we'd use QComboBox::currentIndexChanged, which fires only when the combobox's 
         // selected index changed. Here, though, we're using QComboBox::activated, which fires when 
         // the user selects any item, even if that item is already selected. Why?
         // 
         // Well, some special-cased condition functions allow users to type arbitrary values into 
         // the combobox. So consider this edge-case, because Qt's design spec evidently didn't:
         // 
         //  - User opens a condition for editing.
         //  - User types a new value into the combobox.
         //  - User saves the condition.
         //  - User opens the condition for editing again.
         //  - We restore the combobox's edit-text,...
         //  = ...but because the insert-policy is NoInsert, we're not actually adding an item,...
         //  = ...so the current index is 0.
         //  - User selects the first (i.e. zeroth) item in the combobox.
         // 
         // In that scenario, the user has changed the selection from custom text to a specific item 
         // in the combobox... but that item was already the "current index," so `currentIndexChanged` 
         // never fires.
         //
         auto* widget = (QComboBox*)sender();
         if (stack->currentWidget() != widget || !widget->isEnabled())
            return;
         QVariant value;
         {
            auto data = widget->currentData();
            if (data.isNull()) {
               value = index;
            } else {
               value = data;
            }
         }
         this->_on_parameter_changed(i, value);
      });
      QObject::connect(param.form,     &DKFormPicker::formChanged, this, [this, i, stack = param.stack](dovah::form_stub* stub) {
         auto* widget = (QWidget*)sender();
         if (stack->currentWidget() != widget || !widget->isEnabled())
            return;
         this->_on_parameter_changed(i, QVariant::fromValue(stub));
      });
      QObject::connect(param.ref,      &DKCompactObjectReferencePicker::refChanged, this, [this, i, stack = param.stack](dovah::form_stub* stub) {
         auto* widget = (QWidget*)sender();
         if (stack->currentWidget() != widget || !widget->isEnabled())
            return;
         this->_on_parameter_changed(i, QVariant::fromValue(stub));
      });
      QObject::connect(param.spinbox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, i, stack = param.stack](double value) {
         auto* widget = (QDoubleSpinBox*)sender();
         if (stack->currentWidget() != widget || !widget->isEnabled())
            return;
         QVariant variant;
         if (widget->decimals() == 0) {
            variant = (int)value;
         } else {
            variant = (float)value;
         }
         this->_on_parameter_changed(i, variant);
      });
      QObject::connect(param.textbox,  &QLineEdit::textChanged, this, [this, i, stack = param.stack](QString text) {
         auto* widget = (QLineEdit*)sender();
         if (stack->currentWidget() != widget || !widget->isEnabled())
            return;
         QVariant value;
         if (widget->maxLength() == 1) {
            if (text.isEmpty()) {
               return;
            }
            value = (char) text[0].unicode();
         } else {
            value = text;
         }
         this->_on_parameter_changed(i, value);
      });
   }
   #pragma endregion
   #pragma region Handle parameter flags
   {
      auto* widget = this->ui.paramFlags;
      widget->clear();
      widget->addItem(tr("Default",          "condition param override flags"), (int)parameter_type_override::none);
      widget->addItem(tr("Use Aliases",      "condition param override flags"), (int)parameter_type_override::alias);
      widget->addItem(tr("Use Package Data", "condition param override flags"), (int)parameter_type_override::package_data);
      widget->setCurrentIndex(widget->findData((int)this->_value.override_types_with));
      
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         auto desired = (parameter_type_override)this->ui.paramFlags->currentData().toInt();
         this->_value.set_type_override(desired);
         this->_update_all_parameters_ui();
      });
   }
   #pragma endregion
   #pragma region Handle run-on
   {
      using run_on_type = ui::types::conditions::run_on_type;

      auto* widget = this->ui.runOn;
      widget->clear(); // clear anything that might've been done in Qt Designer
      widget->addItem(tr("Subject",       "condition run on"), (int)run_on_type::subject);
      widget->addItem(tr("Target",        "condition run on"), (int)run_on_type::target);
      widget->addItem(tr("Reference",     "condition run on"), (int)run_on_type::reference);
      widget->addItem(tr("Combat Target", "condition run on"), (int)run_on_type::combat_target);
      widget->addItem(tr("Linked Ref",    "condition run on"), (int)run_on_type::linked_ref);
      widget->addItem(tr("Alias",         "condition run on"), (int)run_on_type::quest_alias);
      widget->addItem(tr("Package Data",  "condition run on"), (int)run_on_type::package_data);
      widget->addItem(tr("Event Data",    "condition run on"), (int)run_on_type::event_data);
      //
      widget->addItem(tr("Player", "condition run on"), (int)run_on_type::reference);
      widget->setItemData(widget->count() - 1, uint32_t(dovah::hardcoded_form_ids::PlayerRef), RunOnFormIDRole);
      widget->setItemData(widget->count() - 1, true, RunOnPlayerSentinelRole);
      
      this->_update_run_on_ui();
      
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         auto* widget = this->ui.runOn;
         //
         this->_value.run_on.type = (run_on_type)this->ui.runOn->currentData().toInt();
         this->_value.reset_run_on_entity();
         auto data = this->ui.runOn->currentData(RunOnFormIDRole);
         if (data.isValid()) {
            auto id = data.value<uint32_t>();
            this->_value.run_on.entity = DovahKitCore::get().get_form(id);
         }
         //
         this->_update_run_on_ui();
      });
      QObject::connect(this->ui.runOnButton, &DKCompactObjectReferencePicker::refChanged, this, [this](dovah::form_stub* stub) {
         if (this->_value.run_on.type != run_on_type::reference)
            return;
         this->_value.run_on.entity = stub;
         this->_update_run_on_ui();
      });
   }
   #pragma endregion
   ui::bind(this->ui.flagSwapSubjectAndTarget, this->_value.flags.swap_subject_and_target);
   #pragma region Handle function
   {
      auto* widget = this->ui.function;
      widget->clear(); // clear anything that might've been done in Qt Designer
      //
      #pragma region Handle sorting and filtering
      auto* proxy = new _FunctionListProxy(widget);
      auto* model = new QStandardItemModel(widget);
      proxy->setSourceModel(model);
      proxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
      proxy->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
      widget->setModel(proxy);
      {
         proxy->setDynamicSortFilter(false);
         dovah::conditions::for_each_function_info([model](const dovah::conditions::function_info& func) {
            auto  name = QString::fromUtf8(QByteArray(func.name.data(), func.name.size()));
            auto* item = new QStandardItem(name);
            item->setData(func.id, Qt::ItemDataRole::UserRole);
            model->appendRow(item);
            return false;
         });
         proxy->setDynamicSortFilter(true);
      }
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, proxy](int index) {
         //
         // Ensure that the currently-selected item always has the ExcludeRole, so that it's never filtered out. 
         // We don't want the filter to ever actually *change* the currently selected function, which means that 
         // we need to ensure that the currently selected function is never filtered out.
         //
         auto* widget = this->ui.function;
         if (widget->currentData(_FunctionListProxy::ExcludeRole).toBool())
            return;
         proxy->setDynamicSortFilter(false); // we need to call setItemData twice; we don't want the first call to reshuffle everything before we're done
         auto  prev   = widget->findData(true, _FunctionListProxy::ExcludeRole);
         if (prev >= 0)
            widget->setItemData(prev, false, _FunctionListProxy::ExcludeRole);
         if (index >= 0)
            widget->setItemData(index, true, _FunctionListProxy::ExcludeRole);
         proxy->setDynamicSortFilter(true);
         proxy->invalidate();
         proxy->sort(Qt::SortOrder::AscendingOrder); // yes, we have to call this manually here
      });
      //
      {
         //
         // Code to filter the function list. For efficiency, we throttle updates.
         //
         QObject::connect(&this->function_filter_update_throttle, &QTimer::timeout, [this, proxy]() {
            proxy->setFilterFixedString(this->ui.filterFunction->text());
         });
         QObject::connect(this->ui.filterFunction, &QLineEdit::textEdited, this, [this](const QString& text) {
            auto& timer = this->function_filter_update_throttle;
            if (timer.isActive())
               return;
            timer.start(200);
         });
         QObject::connect(this->ui.filterFunction, &QLineEdit::editingFinished, this, [this, proxy]() {
            auto& timer = this->function_filter_update_throttle;
            timer.stop();
            proxy->setFilterFixedString(this->ui.filterFunction->text());
         });
      }
      #pragma endregion
      //
      widget->setCurrentIndex(widget->findData(this->_value.function));
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         auto id = this->ui.function->currentData().toInt();
         if (!dovah::conditions::function_info_by_id(id))
            return;
         this->_value.set_function_id(id);
         this->_update_all_parameters_ui();
      });
   }
   #pragma endregion
   this->_update_all_parameters_ui();
   #pragma region Handle operator
   {  // Result: Operator
      using comparison_operator = ui::types::conditions::comparison_operator;

      auto* widget = this->ui.op;
      widget->clear(); // clear anything that might've been done in Qt Designer
      widget->addItem(tr("==", "condition operator"), (int)comparison_operator::equal);
      widget->addItem(tr("!=", "condition operator"), (int)comparison_operator::not_equal);
      widget->addItem(tr(">",  "condition operator"), (int)comparison_operator::greater);
      widget->addItem(tr(">=", "condition operator"), (int)comparison_operator::greater_or_equal);
      widget->addItem(tr("<",  "condition operator"), (int)comparison_operator::less);
      widget->addItem(tr("<=", "condition operator"), (int)comparison_operator::less_or_equal);
      ui::bind(widget, this->_value.comparison.op);
   }
   #pragma endregion
   #pragma region Handle operand
   {  // Result: Operand
      cobb::qt::remove_spinbox_bounds(this->ui.operandConstant);
      QObject::connect(this->ui.operandConstant, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
         this->_value.comparison.operand = (float)value;
      });
      QObject::connect(this->ui.operandGlobal, &DKFormPicker::formChanged, this, [this](dovah::form_stub* value) {
         this->_value.comparison.operand = value;
      });
      QObject::connect(this->ui.flagCompareToGlobal, &QCheckBox::stateChanged, this, [this](int state) {
         if (state == Qt::CheckState::Checked) {
            this->ui.operandStack->setCurrentWidget(this->ui.operandPageGlobal);
         } else {
            this->ui.operandStack->setCurrentWidget(this->ui.operandPageConstant);
         }
      });

      this->ui.operandGlobal->setAllowedFormType(dovah::form_type::global);
      this->_update_comparison_operand_ui();
   }
   #pragma endregion
   ui::bind(this->ui.flagOr, this->_value.flags.or_linked);
   //
   #pragma region Frontend signals
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_context = {};
      this->_value   = {};
      this->reject();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* target) {
      this->_on_form_deleted(*target);
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* target) {
      this->_on_form_modified(*target);
   });
   QObject::connect(&editor, &DovahKitCore::formWorkingCopyDeleteComplete, this, [this](dovah::form_stub* target) {
      this->_on_form_modified(*target);
   });
   QObject::connect(&editor, &DovahKitCore::questWorkingCopyStagesAltered, this, [this](dovah::form_stub* target) {
      using underlying_type = dovah::conditions::parameter_underlying_type;

      if (this->_value.get_argument_underlying_type(1) == underlying_type::quest_stage) { // HACK HACK HACK
         if (auto* casted = std::get_if<dovah::form_stub*>(&this->_value.parameters[0])) {
            if (*casted == target)
               this->_update_parameter_ui(1);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::questWorkingCopyAliasesAltered, this, [this](dovah::form_stub* target) {
      using underlying_type = dovah::conditions::parameter_underlying_type;

      if (target != this->_context.quest)
         return;
      for (size_t i = 0; i < 2; ++i) {
         auto* typeinfo = this->_value.get_effective_argument_typeinfo(i);
         if (!typeinfo)
            continue;
         if (typeinfo->underlying_type == underlying_type::alias)
            this->_update_parameter_ui(i);
      }
      this->_update_run_on_ui();
   });
   QObject::connect(&editor, &DovahKitCore::packageWorkingCopyPackageDataAltered, this, [this](dovah::form_stub* target) {
      if (target != this->_context.package)
         return;
      for (size_t i = 0; i < this->_value.parameters.size(); ++i) {
         const auto* typeinfo = this->_value.get_effective_argument_typeinfo(i);
         if (typeinfo && typeinfo->underlying_type == dovah::conditions::parameter_underlying_type::package_data)
            this->_update_parameter_ui(i);
      }
      this->_update_run_on_ui();
   });
   #pragma endregion

   #pragma region "What's This?"
   this->ui.runOn->setWhatsThis(tr(
      "<p>Most condition functions are run in terms of a particular ObjectReference. These "
      "functions are generally named in terms of what they run on. For example, <code>GetSex</code> "
      "gets the sex of the actor it runs on; <code>GetInFaction</code> checks whether the actor "
      "it runs on is the member of some faction; and so on.</p>"
      "<p>But... What ObjectReference does the function run on? Well, you can choose.</p>"
      "<dl>"
         "<dt><b>Subject</b></dt>"
            "<dd><p>The \"Subject\" depends on context. For dialogue conditions, it's the actor "
            "who's going to say the line. For quest targets, it's the player. For Magic Effects "
            "and similar, it's the actor being targeted by the effect, <em>unless</em> the Swap "
            "Subject and Target box is checked, in which case it's the caster.</p></dd>"
         "<dt><b>Target</b></dt>"
            "<dd><p>The \"Target\" depends on context. For dialogue conditions, it's the actor "
            "who is being spoken to. For package conditions, it's the ref that has been selected "
            "as a target. For Magic Effects and similar, it's the actor being targeted by the "
            "effect.</p></dd>"
         "<dt><b>Reference</b></dt>"
            "<dd><p>A specific ObjectReference pre-placed in the game world.</p></dd>"
         "<dt><b>Combat Target</b></dt>"
            "<dd><p>For magic conditions, this is the actor being targeted by the magic effect. "
            "Otherwise, if the Subject is in combat, this is their target.</p></dd>"
         "<dt><b>Linked Reference</b></dt>"
            "<dd><p>The Subject's linked ref, if they have one.</p></dd>"
      "</dl>"
      "<p>For conditions that exist somewhere inside of a quest, you can also run the condition on "
      "one of the quest's aliases; and for conditions that exist somewhere inside of a package, you "
      "can run the condition on any package data that is an ObjectReference.</p>"
   ));
   #pragma endregion
}

void DKConditionEditDialog::forceUpdateParameters() {
   this->_update_all_parameters_ui();
}

void DKConditionEditDialog::_on_form_deleted(const dovah::form_stub& stub) {
   if (&stub == this->_context.owner) {
      this->_context = {};
      this->_value   = {};
      this->reject();
      return;
   }
   if (&stub == this->_context.package) {
      this->_context.package        = nullptr;
      this->_context.loaded.package = {};
   }
   if (&stub == this->_context.quest) {
      this->_context.quest        = nullptr;
      this->_context.loaded.quest = {};
   }
   if (!this->_value.sever_outbound_references_to(&stub))
      return;
   this->_update_run_on_ui();
   this->_update_all_parameters_ui();
   this->_update_comparison_operand_ui();
};
void DKConditionEditDialog::_on_form_modified(const dovah::form_stub& stub) {
   bool refers = false;
   if (&stub == this->_context.package)
      refers = true;
   else if (&stub == this->_context.quest)
      refers = true;
   else if (this->_value.refers_to_form(&stub))
      refers = true;

   if (!refers)
      return;
   this->_update_run_on_ui();
   this->_update_all_parameters_ui();
   this->_update_comparison_operand_ui();
}

void DKConditionEditDialog::_update_comparison_operand_ui() {
   const auto blockers = std::array{
      QSignalBlocker(this->ui.flagCompareToGlobal),
      QSignalBlocker(this->ui.operandConstant),
      QSignalBlocker(this->ui.operandGlobal),
   };

   bool use_global = false;
   {
      auto& src = this->_value.comparison.operand;
      if (std::holds_alternative<dovah::form_stub*>(src)) {
         use_global = true;

         auto* stub = std::get<dovah::form_stub*>(src);
         if (stub && !stub->is_none_stub())
            this->ui.operandGlobal->setFormStub(stub);
      } else {
         this->ui.operandConstant->setValue(std::get<float>(src));
      }
   }

   this->ui.flagCompareToGlobal->setCheckState(use_global ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
   if (use_global) {
      this->ui.operandStack->setCurrentWidget(this->ui.operandPageGlobal);
   } else {
      this->ui.operandStack->setCurrentWidget(this->ui.operandPageConstant);
   }
}
void DKConditionEditDialog::_update_run_on_ui() {
   const auto blocker0 = QSignalBlocker(this->ui.runOnDropdown);
   const auto blocker1 = QSignalBlocker(this->ui.runOnButton);
   
   bool is_player = this->ui.runOn->currentData(RunOnPlayerSentinelRole).toBool();

   auto& entity = this->_value.run_on.entity;

   switch (this->_value.run_on.type) {
      using enum ui::types::conditions::run_on_type;
      case subject:
      case target:
      case linked_ref:
      case combat_target:
         {
            this->ui.runOnButton->setRef(nullptr);
            this->ui.runOnDropdown->setEnabled(false);
            this->ui.runOnDropdown->clear();
            this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         }
         return;
      case reference:
         {
            dovah::form_stub* stub = nullptr;
            if (auto* casted = std::get_if<dovah::form_stub*>(&entity))
               stub = *casted;

            this->ui.runOnButton->setRef(stub);
            this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageButton);
            //
            if (stub && stub->formID == dovah::hardcoded_form_ids::PlayerRef) {
               if (!is_player) {
                  const auto blocker = QSignalBlocker(this->ui.runOn);
                  this->ui.runOn->setCurrentIndex(this->ui.runOn->findData(true, RunOnPlayerSentinelRole));
               }
               this->ui.runOnDropdown->setEnabled(false);
               this->ui.runOnDropdown->clear();
               this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
            }
         }
         return;
      case quest_alias:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnButton->setRef(nullptr);
         this->ui.runOnDropdown->setEnabled(true);
         this->ui.runOnDropdown->clear();
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* q = this->_context.get_owning_quest()) {
            auto* widget = this->ui.runOnDropdown;
            q->for_each_alias_of_type(dovah::loaded_forms::Alias::alias_type::reference, [widget](dovah::loaded_forms::Alias* alias) {
               widget->addItem(alias->name.c_str(), alias->id);
               return false;
            });
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         if (auto* casted = std::get_if<uint32_t>(&entity)) {
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(*casted));
         } else {
            this->ui.runOnDropdown->setCurrentIndex(0);
         }
         return;
      case package_data:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnButton->setRef(nullptr);
         this->ui.runOnDropdown->setEnabled(true);
         this->ui.runOnDropdown->clear();
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         {
            bool valid = false;
            if (auto* package = this->_context.get_owning_package()) {
               if (auto* decls = _get_package_data_declaration_list(package)) {
                  valid = true;
                  for (auto& entry : decls->entries) {
                     if (entry.unique_id == 0xFF)
                        continue;
                     auto name = QString::fromStdString(entry.name);
                     this->ui.runOnDropdown->addItem(name, (int)entry.unique_id);
                  }
               }
            }
            this->ui.runOnDropdown->setEnabled(valid);
         }
         if (auto* casted = std::get_if<uint32_t>(&entity)) {
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(*casted));
         } else {
            this->ui.runOnDropdown->setCurrentIndex(0);
         }
         return;
      case event_data:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnButton->setRef(nullptr);
         this->ui.runOnDropdown->clear();
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* q = this->_context.get_owning_quest()) {
            auto  code = q->event;
            auto* def  = dovah::story_event_definition::lookup(code);
            if (def)
               for (auto& data : def->members)
                  this->ui.runOnDropdown->addItem(data.name, dovah::story_event_definition::widen_member_code(data.signature));
            this->ui.runOnDropdown->setEnabled(def != nullptr);
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         if (auto* casted = std::get_if<uint32_t>(&entity)) {
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(*casted));
         } else {
            this->ui.runOnDropdown->setCurrentIndex(0);
         }
         return;
   }
}

void DKConditionEditDialog::_on_parameter_changed(size_t index, QVariant value) {
   if (this->_value.event_parameters.has_value()) {
      auto& dst = this->_value.event_parameters.value();
      switch (index) {
         case 0: // event function
            dst.function = value.toInt();
            this->_update_parameter_ui(1);
            this->_update_parameter_ui(2);
            break;
         case 1: // event member
            dst.member = value.toInt();
            this->_update_parameter_ui(2);
            break;
         case 2: // event form
            dst.form = value.value<dovah::form_stub*>();
            break;
      }
      return;
   }

   using underlying_type = dovah::conditions::parameter_underlying_type;

   if (index > 1)
      return;

   const auto* typeinfo = this->_value.get_effective_argument_typeinfo(index);
   if (!typeinfo || typeinfo == &dovah::conditions::parameter_types::None)
      return;
   if (typeinfo->underlying_type == underlying_type::none)
      return;

   switch (typeinfo->underlying_type) {
      case underlying_type::alias:
         this->_value.parameters[index] = (uint32_t)value.value<int>();
         break;
      case underlying_type::character:
         this->_value.parameters[index] = value.value<char>();
         break;
      case underlying_type::enumeration:
         this->_value.parameters[index] = (int32_t)value.value<int>();
         break;
      case underlying_type::float32:
         this->_value.parameters[index] = value.value<float>();
         break;
      case underlying_type::form:
         this->_value.parameters[index] = value.value<dovah::form_stub*>();
         break;
      case underlying_type::int_signed:
         this->_value.parameters[index] = (int32_t)value.value<int>();
         break;
      case underlying_type::int_unsigned:
         this->_value.parameters[index] = (uint32_t)value.value<int>();
         break;
      case underlying_type::package_data:
         this->_value.parameters[index] = (uint32_t)value.value<int>();
         break;
      case underlying_type::string:
         this->_value.parameters[index] = value.value<QString>().toStdString();
         break;
   }

   if (index == 0) {
      const auto* next_typeinfo = this->_value.get_effective_argument_typeinfo(index + 1);
      if (next_typeinfo && next_typeinfo->is_union()) {
         this->_update_parameter_ui(index + 1);
      } else {
         switch (this->_value.function) {
            case special_case_functions::GetVMQuestVariable:
            case special_case_functions::GetVMScriptVariable:
            case special_case_functions::IsSceneActionComplete:
               this->_update_parameter_ui(index + 1);
               break;
         }
      }
   }
}

void DKConditionEditDialog::_renew_combobox_edit_handler(size_t index) {
   auto& param = this->parameters[index];
   if (!param.combobox->isEditable())
      return;

   auto* line = param.combobox->lineEdit();
   if (!line)
      return;

   QObject::disconnect(line, &QLineEdit::editingFinished, this, nullptr); // ensure signal never stacks
   QObject::connect(line, &QLineEdit::editingFinished, this, [this, index, stack = param.stack, widget = param.combobox]() {
      if (stack->currentWidget() != widget || !widget->isEnabled())
         return;
      if (!widget->isEditable())
         return;

      auto _fallback_to_combobox = [this, index, widget]() {
         QVariant value;
         {
            auto data = widget->currentData();
            if (data.isNull()) {
               value = index;
            } else {
               value = data;
            }
         }
         this->_on_parameter_changed(index, value);
      };

      if (widget->lineEdit()->text().isEmpty()) {
         _fallback_to_combobox();
         return;
      }
      QVariant value;
      {
         auto text         = widget->currentText();
         auto desired_type = QMetaType::Void;

         {
            //
            // Handle exact matches for known values:
            //
            auto i = widget->currentIndex();
            if (i >= 0) {
               auto item_text = widget->itemText(i);
               if (text == item_text) {
                  _fallback_to_combobox();
                  return;
               }
            }
         }

         // Vile hack to know what data type to use:
         if (widget->count() > 0) {
            bool success    = false;
            auto basis_data = widget->itemData(0); // base it on the first item's data's type
            desired_type = (decltype(desired_type)) basis_data.type(); // cast needed for QVariant's historical jank
         }

         bool success = false;
         switch (desired_type) {
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Short:
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::UShort:
               text  = text.trimmed();
               value = text.toInt(&success);
               if (!success) {
                  //
                  // Many known values are formatted like "0 (Torso)" and such. Convert them 
                  // over in a "friendly" way, e.g. "0 (Trso)" -> 0.
                  //
                  auto i = text.indexOf(' ');
                  if (i >= 0) {
                     text  = text.left(i);
                     value = text.toInt(&success);
                  }
               }
               break;
            case QMetaType::Double:
            case QMetaType::Float:
               text  = text.trimmed();
               value = text.toFloat(&success);
               if (!success) {
                  //
                  // Many known values are formatted like "0 (Torso)" and such. Convert them 
                  // over in a "friendly" way, e.g. "0 (Trso)" -> 0.
                  //
                  auto i = text.indexOf(' ');
                  if (i >= 0) {
                     text  = text.left(i);
                     value = text.toFloat(&success);
                  }
               }
               break;
            case QMetaType::QString:
               success = true;
               value   = text;
               break;
         }
         if (!success) {
            if (!text.isEmpty()) {
               QApplication::beep();
               //
               // Windows' `MessageBeep` API can fail if called too rapidly. And I mean, like,
               // until the system is restarted. If it gets called too rapidly due to a bug, 
               // we may never know... unless we flood the console too.
               //
               qDebug("BEEP");
            }
            this->_update_parameter_ui(index); // revert the value to a valid one
            return;
         }
      }
      this->_on_parameter_changed(index, value);
   });

}
bool DKConditionEditDialog::_update_parameter_ui_for_special_case(size_t index) {
   using underlying_type = dovah::conditions::parameter_underlying_type;

   if (index >= 2) {
      return false;
   }
   const auto& value      = this->_value.parameters[index];
   const auto* typeinfo   = this->_value.get_effective_argument_typeinfo(index);
   const auto  underlying = typeinfo ? typeinfo->underlying_type : underlying_type::none;

   auto& param = this->parameters[index];
   //
   const auto blockers = std::array{
      QSignalBlocker(param.combobox),
      QSignalBlocker(param.form),
      QSignalBlocker(param.ref),
      QSignalBlocker(param.spinbox),
      QSignalBlocker(param.textbox),
   };

   auto function_id = this->_value.function;

   if (function_id == special_case_functions::GetVMQuestVariable || function_id == special_case_functions::GetVMScriptVariable) {
      if (index != 1 || underlying != underlying_type::string)
         return false;

      auto& prev_value = this->_value.parameters[index - 1];
      if (!std::holds_alternative<dovah::form_stub*>(prev_value))
         return false;

      auto* subject = std::get<dovah::form_stub*>(prev_value);
      auto* widget  = param.combobox;
      widget->clear();
      widget->setEditable(false);
      if (!subject) {
         widget->setEnabled(false);
      } else {
         widget->setEnabled(true);

         auto& assets  = dovahkit::subsystems::assets::get();
         auto& fic     = dovahkit::subsystems::form_info_cache::core::get();
         auto  scripts = fic.get_scripts_attached_to_form(*subject);
         if (!scripts.empty()) {
            for (const auto* script : scripts) {
               bool conditional = false;
               if (script->info.loose.has_value())
                  conditional = script->info.loose.value().flags.conditional;
               else if (script->info.packed.has_value())
                  conditional = script->info.packed.value().flags.conditional;

               if (!conditional)
                  continue;
               
               //
               // The script itself is flagged as conditional, so it is allowed to have 
               // conditional-flagged properties.
               //

               dovah::compiled_papyrus_script data;
               {
                  std::filesystem::path path("scripts/");
                  path /= script->name + ".pex";

                  auto* file = assets.lookup_game_asset(path);
                  if (!file)
                     continue;
                  try {
                     data.read_file(file->data(), file->size());
                     delete file;
                  } catch (dovah::compiled_papyrus_script::read_exception& e) {
                     delete file;
                     continue;
                  }
               }

               uint32_t conditional_mask = 0;
               for (const auto& flag : data.user_flags) {
                  if (dovah::papyrus::helpers::name_equals(flag.name, "conditional")) {
                     conditional_mask = flag.to_mask();
                     break;
                  }
               }
               if (!conditional_mask)
                  continue;


               for (const auto& object : data.objects) {
                  if (!dovah::papyrus::helpers::name_equals(object.name, script->name)) // guard against multi-PEXs
                     continue;
                  for (const auto& prop : object.properties) {
                     if (!(prop.property_flags & conditional_mask))
                        continue;

                     auto& autovar_name = prop.autovar_name;
                     if (autovar_name.empty())
                        continue;
                     QString text = QString::fromUtf8(QByteArray(autovar_name.data(), autovar_name.size()));
                     widget->addItem(text, text);
                  }
               }
            }
            //
            // Done scanning all attached scripts for conditional properties.
            //
         }
         //
         // Done handling all attached scripts.
         //
         widget->addItem(tr(" NONE", "Papyrus property auto-variable name - 'none' option"), QString(""));
         widget->model()->sort(0, Qt::AscendingOrder);

         int i = -1;
         if (std::holds_alternative<std::string>(value)) {
            const auto& str = std::get<std::string>(value);
            QString prior = QString::fromUtf8(QByteArray(str.data(), str.size()));

            i = widget->findData(prior);
         } else {
            i = widget->findData(QString(""));
         }
         if (i >= 0)
            widget->setCurrentIndex(i);
      }
      param.stack->setCurrentWidget(widget);
      return true;
   }
   if (function_id == special_case_functions::IsInCombat) {
      //
      // TODO: The single Integer parameter is a bool: if non-zero, then the condition checks whether 
      //       the actor is ignoring combat, and if so, returns 0 instead of 1 even if the actor is 
      //       currently in combat.
      // 
      //       We should render it as a checkbox with a label e.g. "Only if not ignoring." Of course, 
      //       we have no code for checkbox args, and we'd have to figure out how to truncate the 
      //       label (and let the user hover over it for a tooltip revealing its full value).
      //
   }
   if (function_id == special_case_functions::IsLimbGone) {
      auto* widget = param.combobox;
      widget->clear();
      widget->setEditable(true);
      widget->addItem(tr("0 (Torso)",    "IsLimbGone value"), (int32_t)0);
      widget->addItem(tr("1 (Head)",     "IsLimbGone value"), (int32_t)1);
      widget->addItem(tr("2 (Eye)",      "IsLimbGone value"), (int32_t)2);
      widget->addItem(tr("3 (Look At)",  "IsLimbGone value"), (int32_t)3);
      widget->addItem(tr("4 (Fly Grab)", "IsLimbGone value"), (int32_t)4);
      widget->addItem(tr("5 (Saddle)",   "IsLimbGone value"), (int32_t)5);
      if (std::holds_alternative<int32_t>(value)) {
         auto limb = std::get<int32_t>(value);

         int i = widget->findData(limb);
         if (i >= 0)
            widget->setCurrentIndex(i);
         else
            widget->setEditText(QString::number(limb));
      }
      param.stack->setCurrentWidget(widget);
      return true;
   }
   if (function_id == special_case_functions::IsPlayerActionActive) {
      auto* widget = param.combobox;
      widget->clear();
      widget->setEditable(true);
      int32_t raw = 0;
      widget->addItem(tr("Swing Melee Weapon",    "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Cast Spell",            "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Shooting Bow",          "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Grabbing (Z-Key) Ref",  "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Knocking Over Objects", "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Standing on Furniture", "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Zoomed-In Aim",         "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Destroy Object",        "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Locked Object",         "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Pickpocket Crosshair",  "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Cast Self Spell",       "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Shout",                 "PLAYER_ACTION"), (int32_t)raw++);
      widget->addItem(tr("Actor Collision",       "PLAYER_ACTION"), (int32_t)raw++);
      if (std::holds_alternative<int32_t>(value)) {
         auto action_value = std::get<int32_t>(value);

         int i = widget->findData(action_value);
         if (i >= 0)
            widget->setCurrentIndex(i);
         else
            widget->setEditText(QString::number(action_value));
      }
      param.stack->setCurrentWidget(widget);
      return true;
   }
   if (function_id == special_case_functions::IsSceneActionComplete) {
      //
      // First parameter is a Scene form; second parameter is the index of an action in that 
      // scene. When we can load Scenes, show a drop-down of the actions instead of a spinbox.
      // 
      if (index != 1)
         return false;

      dovah::form_stub* scene = nullptr;
      {
         auto& scene_value = this->_value.parameters[index - 1];
         if (!std::holds_alternative<dovah::form_stub*>(scene_value))
            return false;
         scene = std::get<dovah::form_stub*>(scene_value);
      }
      if (!scene)
         return false;

      auto loaded = scene->load().ptr_cast<dovah::loaded_forms::Scene>();

      auto* widget = param.combobox;
      widget->clear();
      widget->setEditable(false);
      if (loaded) {
         for (auto& action : loaded->actions) {
            auto name = QString::fromStdString(action.name);
            if (name.isEmpty())
               name = tr("Action #%1").arg(action.action_id);
            else
               name = tr("Action #%1: %2").arg(action.action_id).arg(name);
            widget->addItem(name, (int)action.action_id);
         }

         int i = -1;
         if (auto* casted = std::get_if<uint32_t>(&this->_value.parameters[index])) {
            i = widget->findData(*casted);
         }
         if (i >= 0)
            widget->setCurrentIndex(i);
      }
      param.stack->setCurrentWidget(widget);
      return true;
      //
      // TODO: Should we handle GetStageDone's quest stage parameter the same way, and remove 
      //       the "quest stage" type that's built into the condition internals?
      //
   }
   return false;
}
//
void DKConditionEditDialog::_update_parameter_ui(size_t index) {
   auto& param = this->parameters[index];

   auto* function = dovah::conditions::function_info_by_id(this->_value.function);
   if (function && function->uses_event_data) {
      param.stack->setCurrentWidget(param.blank); // signal handlers will abort while this is the current widget

      bool event_has_form = false;
      bool event_has_refr = false;

      const auto& src_opt = this->_value.event_parameters;
      switch (index) {
         case 0: // event function
            {
               auto* widget = param.combobox;
               widget->clear();
               widget->addItem("GetIsID",      dovah::conditions::event_function::GetIsID);
               widget->addItem("GetItemValue", dovah::conditions::event_function::GetItemValue);
               widget->addItem("GetValue",     dovah::conditions::event_function::GetValue);
               widget->addItem("HasKeyword",   dovah::conditions::event_function::HasKeyword);
               widget->addItem("IsInList",     dovah::conditions::event_function::IsInList);
               //
               if (src_opt.has_value()) {
                  widget->setCurrentIndex(widget->findData((int)src_opt.value().function));
               }
            }
            break;
         case 1: // event member
            {
               auto* widget = param.combobox;
               widget->clear();
               //
               bool member_must_be_form = false; // NOTE: this is not a limitation that the CK enforces
               bool member_cant_be_refr = true;  // NOTE: the CK always enforces this limitation, for all functions
               if (src_opt.has_value()) {
                  switch (src_opt.value().function) {
                     case dovah::conditions::event_function::GetIsID:
                     case dovah::conditions::event_function::GetItemValue:
                     case dovah::conditions::event_function::HasKeyword:
                     case dovah::conditions::event_function::IsInList:
                        member_must_be_form = true;
                        break;
                  }
               }
               if (auto* quest = this->_context.get_owning_quest()) {
                  if (auto* event = dovah::story_event_definition::lookup(quest->event)) {
                     for (auto& member : event->members) {
                        if (member_cant_be_refr && member.can_only_be_reference())
                           continue;
                        if (member_must_be_form && !member.is_form())
                           continue;
                        widget->addItem(member.name, member.signature);
                     }
                  }
               }
               if (src_opt.has_value()) {
                  widget->setCurrentIndex(widget->findData(src_opt.value().member));
               }
            }
            break;
         case 2:
            {
               if (!src_opt.has_value()) {
                  break;
               }
               const auto& src = src_opt.value();

               QVector<dovah::form_type> allowed;
               switch (src.function) {
                  case dovah::conditions::event_function::GetIsID:
                     {
                        bool known       = false;
                        bool use_default = false;
                        if (auto* quest = this->_context.get_owning_quest()) {
                           if (auto* event = dovah::story_event_definition::lookup(quest->event)) {
                              if (auto* member = event->member_by_signature(src.member)) {
                                 known = true;
                                 //
                                 auto& list = member->allowed_form_types;
                                 if (!list.empty()) {
                                    if (list.size() == 1 && list[0] == dovah::form_type::none) {
                                       use_default = true;
                                    } else {
                                       for (auto ft : list) {
                                          if (!dovah::form_type_is_reference(ft))
                                             allowed.push_back(ft);
                                       }
                                    }
                                 }
                              }
                           }
                        }
                        if (use_default) {
                           allowed = {
                              dovah::form_type::acoustic_space, // Confirmed in CK. Strange, since these aren't placeable.
                              dovah::form_type::activator,
                              dovah::form_type::actor_base,
                              dovah::form_type::container,
                              dovah::form_type::door,
                              dovah::form_type::flora,
                              dovah::form_type::furniture,
                              dovah::form_type::grass,
                              dovah::form_type::hazard,
                              dovah::form_type::idle_marker,
                              dovah::form_type::light,
                              dovah::form_type::movable_static,
                              dovah::form_type::projectile,
                              dovah::form_type::sound_descriptor,
                              dovah::form_type::statik,
                              dovah::form_type::talking_activator,
                              dovah::form_type::tree,
                              // Items:
                              dovah::form_type::ammo,
                              dovah::form_type::armor,
                              dovah::form_type::armor_addon,
                              dovah::form_type::book,
                              dovah::form_type::key,
                              dovah::form_type::leveled_item,
                              dovah::form_type::misc_item,
                              dovah::form_type::potion,
                              dovah::form_type::scroll,
                              dovah::form_type::soul_gem,
                              dovah::form_type::weapon,
                              // Magic:
                              dovah::form_type::enchantment,
                              dovah::form_type::leveled_spell,
                              dovah::form_type::shout,
                              dovah::form_type::spell,
                              // Other:
                              dovah::form_type::formlist,
                           };
                        }
                     }
                     break;
                  case dovah::conditions::event_function::HasKeyword:
                     allowed.push_back(dovah::form_type::keyword);
                     break;
                  case dovah::conditions::event_function::IsInList:
                     allowed.push_back(dovah::form_type::formlist);
                     break;
               }
               if (allowed.isEmpty()) {
                  break;
               }
               if (allowed.size() == 1) {
                  switch (allowed[0]) {
                     case dovah::form_type::actor:
                        param.ref->setRequiredFormType(dovah::form_type::actor);
                        event_has_refr = true;
                        break;
                     case dovah::form_type::reference:
                        param.ref->setRequiredFormType(dovah::form_type::reference);
                        event_has_refr = true;
                        break;
                  }
                  if (event_has_refr)
                     break;
               }
               if (!allowed.isEmpty()) {
                  event_has_form = true;
                  //
                  auto* stub = src.form;
                  param.form->setAllowedFormTypes(allowed.toList());
                  param.form->setFormStub(stub);
               }
            }
            break;
      }

      //
      // Widget set up; now, show it.
      //

      switch (index) {
         case 0:
         case 1:
            param.stack->setCurrentWidget(param.combobox);
            break;
         case 2:
            if (!event_has_form) {
               param.stack->setCurrentWidget(param.blank);
               break;
            }
            if (event_has_refr) {
               param.stack->setCurrentWidget(param.ref);
               break;
            }
            param.stack->setCurrentWidget(param.form);
            break;
      }
      return;
   }

   using underlying_type = dovah::conditions::parameter_underlying_type;

   if (index == 2) {
      param.stack->setCurrentWidget(param.blank);
      return;
   }
   const auto& value    = this->_value.parameters[index];
   const auto* typeinfo = this->_value.get_effective_argument_typeinfo(index);
   if (!typeinfo || typeinfo == &dovah::conditions::parameter_types::None || typeinfo->underlying_type == underlying_type::none) {
      param.stack->setCurrentWidget(param.blank);
      return;
   }
   if (this->_update_parameter_ui_for_special_case(index)) {
      this->_renew_combobox_edit_handler(index);
      return;
   }

   const auto blockers = std::array{
      QSignalBlocker(param.combobox),
      QSignalBlocker(param.form),
      QSignalBlocker(param.ref),
      QSignalBlocker(param.spinbox),
      QSignalBlocker(param.textbox),
   };
   param.combobox->setEditable(false); // clean this up in case `_update_parameter_ui_for_special_case` changed it

   bool underlying_type_unchanged = param.last_shown_type == typeinfo->underlying_type;

   switch (typeinfo->underlying_type) {
      case underlying_type::alias:
         if (auto* quest = this->_context.get_owning_quest()) {
            auto* widget = param.combobox;
            widget->setEnabled(true);
            widget->clear();

            for (auto* alias : quest->aliases) {
               widget->addItem(QString::fromStdString(alias->name), alias->id);
            }
            if (std::holds_alternative<uint32_t>(value)) {
               auto i = widget->findData(std::get<uint32_t>(value));
               if (i >= 0)
                  widget->setCurrentIndex(i);
            }
         } else {
            param.combobox->setEnabled(false);
            param.combobox->clear();
         }
         param.stack->setCurrentWidget(param.combobox);
         break;
      case underlying_type::character:
         if (typeinfo->character_values.empty()) {
            auto* widget = param.textbox;
            widget->setMaxLength(1);

            QChar c = 'X';
            if (std::holds_alternative<char>(value))
               c = std::get<char>(value);
            widget->setText(c);

            param.stack->setCurrentWidget(widget);
         } else {
            auto* widget = param.combobox;
            widget->setEnabled(true);
            widget->clear();

            for (auto c : typeinfo->character_values) {
               widget->addItem(QChar(c), (int)c);
            }
            if (std::holds_alternative<char>(value)) {
               int  c = std::get<char>(value);
               auto i = widget->findData(c);
               if (i >= 0)
                  widget->setCurrentIndex(i);
            }

            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::enumeration:
         {
            auto* widget = param.combobox;
            widget->clear();

            if (typeinfo->enumeration_info.has_value()) {
               const auto& src = typeinfo->enumeration_info.value();

               widget->setEnabled(true);
               for (size_t i = 0; i < src.size; ++i) {
                  const auto& member = src.members[i];
                  auto name = QString::fromUtf8(QByteArray(member.name.data(), member.name.size()));

                  widget->addItem(name, member.value);
               }

               if (std::holds_alternative<int32_t>(value)) {
                  auto i = widget->findData(std::get<int32_t>(value));
                  if (i >= 0)
                     widget->setCurrentIndex(i);
               } else if (std::holds_alternative<uint32_t>(value)) {
                  auto i = widget->findData(std::get<uint32_t>(value));
                  if (i >= 0)
                     widget->setCurrentIndex(i);
               }
            } else {
               widget->setEnabled(false);
            }

            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::float32:
         {
            auto* widget = param.spinbox;
            widget->setDecimals(4);
            widget->setValue(0);

            if (std::holds_alternative<float>(value))
               widget->setValue(std::get<float>(value));

            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::form:
         {
            bool all_allowed_types_are_refs = true;

            QList<dovah::form_type> allowed_types;
            typeinfo->for_each_allowed_form_type([&allowed_types, &all_allowed_types_are_refs](dovah::form_type ft) {
               allowed_types.push_back(ft);

               if (!dovah::form_type_is_reference(ft)) {
                  all_allowed_types_are_refs = false;
               }
            });
            if (allowed_types.empty())
               all_allowed_types_are_refs = false;

            if (all_allowed_types_are_refs) {
               auto* widget = param.ref;
               if (allowed_types.size() == 1) {
                  //
                  // As of this writing, there are no cases of condition arg types that limit to 
                  // multiple specific REFR types. The only specific REFR types that exist are 
                  // actors, placed hazards, and placed projectiles.
                  // 
                  // That said, we probably should give DKCompactObjectReferencePicker the ability 
                  // to limit to multiple form types -- and the non-compact version too, if it 
                  // can't already do that.
                  //
                  widget->setRequiredFormType(allowed_types[0]);
               } else {
                  widget->setRequiredFormType(dovah::form_type::reference);
               }

               if (std::holds_alternative<dovah::form_stub*>(value)) {
                  auto* stub = std::get<dovah::form_stub*>(value);
                  if (stub && allowed_types.contains(stub->form_type)) {
                     widget->setRef(stub);
                  }
               }

               param.stack->setCurrentWidget(widget);
            } else {
               auto* widget = param.form;
               widget->setAllowedFormTypes(allowed_types);

               if (std::holds_alternative<dovah::form_stub*>(value)) {
                  auto* stub = std::get<dovah::form_stub*>(value);
                  if (stub && (allowed_types.empty() || allowed_types.contains(stub->form_type))) {
                     widget->setFormStub(stub);
                  }
               }

               param.stack->setCurrentWidget(widget);
            }
         }
         break;
      case underlying_type::int_signed:
         {
            auto* widget = param.spinbox;
            widget->setDecimals(0);
            widget->setValue(0);

            if (std::holds_alternative<int32_t>(value))
               widget->setValue(std::get<int32_t>(value));

            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::int_unsigned:
         {
            auto* widget = param.spinbox;
            widget->setDecimals(0);
            widget->setValue(0);

            if (std::holds_alternative<uint32_t>(value))
               widget->setValue(std::get<uint32_t>(value));

            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::quest_stage:
         {
            auto* widget = param.combobox;
            widget->clear();
            if (index > 0) {
               dovah::form_stub* quest = nullptr;
               {
                  const auto& prev_value = this->_value.parameters[index - 1];
                  if (std::holds_alternative<dovah::form_stub*>(prev_value)) {
                     auto* stub = std::get<dovah::form_stub*>(prev_value);
                     if (stub && stub->form_type == dovah::form_type::quest)
                        quest = stub;
                  }
               }
               if (quest) {
                  widget->setEnabled(true);
                  this->_quest_stages_to_combobox(*quest, widget, underlying_type_unchanged);
                  //
                  if (std::holds_alternative<uint32_t>(value)) {
                     auto i = widget->findData(std::get<uint32_t>(value));
                     if (i >= 0)
                        widget->setCurrentIndex(i);
                  }
               } else {
                  widget->setEnabled(false);
               }
            } else {
               widget->setEnabled(false);
            }
            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::package_data:
         {
            auto* widget = param.combobox;
            widget->clear();
            widget->setEnabled(false);
            if (auto* package = this->_context.get_owning_package()) {
               if (auto* decls = _get_package_data_declaration_list(package)) {
                  bool empty = true;
                  for (auto& entry : decls->entries) {
                     if (entry.unique_id == 0xFF)
                        continue;
                     auto name = QString::fromStdString(entry.name);
                     widget->addItem(name, (int)entry.unique_id);
                     empty = false;
                  }
                  widget->setEnabled(!empty);
                  if (std::holds_alternative<uint32_t>(value)) {
                     auto i = widget->findData(std::get<uint32_t>(value));
                     if (i >= 0)
                        widget->setCurrentIndex(i);
                  }
               }
            }
            param.stack->setCurrentWidget(widget);
         }
         break;
      case underlying_type::string:
         {
            auto* widget = param.textbox;
            widget->setMaxLength(32767);

            if (std::holds_alternative<std::string>(value)) {
               const auto& src = std::get<std::string>(value);
               widget->setText(QString::fromStdString(src));
            } else {
               widget->setText({});
            }

            param.stack->setCurrentWidget(widget);
         }
         break;
   }
}

void DKConditionEditDialog::_update_all_parameters_ui() {
   this->_update_parameter_ui(0);
   this->_update_parameter_ui(1);
   this->_update_parameter_ui(2);
}

void DKConditionEditDialog::_quest_stages_to_combobox(dovah::form_stub& quest, QComboBox* widget, bool maintain_selection) {
   widget->clear();

   using loaded_form_type = dovah::loaded_forms::Quest;

   //
   // If the quest to pull from is the owning quest for these conditions, then we're (probably) editing 
   // a condition somewhere in that quest, so grab stages from its working copy. That way, users won't 
   // have to create stages, close the quest dialog, and then reopen it to use those stages in conditions.
   //
   dovah::loaded_form_ptr<loaded_form_type> load;
   const loaded_form_type* src = nullptr;

   if (&quest == this->_context.quest) {
      src  = (const loaded_form_type*) quest.get_working_copy();
   } else {
      load = quest.load().ptr_cast<loaded_form_type>();
      src  = &*load;
   }
   if (!src)
      return;

   for (auto& stage : src->stages) {
      widget->addItem(QString::number(stage.index), stage.index);
   }
}

void DKConditionEditDialog::showEvent(QShowEvent* event) {
   if (event->spontaneous())
      return;
   if (this->did_param_holder_layout)
      return;
   //
   // Set the parameter holders' minimum heights, to ensure that things don't change size as 
   // we switch which controls are in each row.
   //
   this->did_param_holder_layout = true;
   //
   auto* button   = this->ui.buttonOK;
   auto* dropdown = this->ui.runOn;
   auto* spinbox  = this->ui.operandConstant;
   auto* textbox  = this->ui.filterFunction;
   //
   auto height = std::max(std::max(std::max(button->height(), dropdown->height()), spinbox->height()), textbox->height());
   this->ui.layoutBottom->setRowMinimumHeight(1, height);
   this->ui.layoutBottom->setRowMinimumHeight(2, height);
   this->ui.layoutBottom->setRowMinimumHeight(3, height);
}