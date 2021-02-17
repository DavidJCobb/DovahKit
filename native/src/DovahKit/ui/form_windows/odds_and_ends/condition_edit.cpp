#include "condition_edit.h"
#include <QShowEvent>
#include <QStandardItemModel>
#include "../../../helpers/qt/basic_bindings.h"
#include "../../../helpers/qt/spinbox.h"
#include "../../../helpers/qt/strings.h"
#include "../../../editor/core.h"
#include "../../generic/FormsOfTypeCombobox.h"
#include "../../generic/RefPickerButton.h"
#include "../../../dovah/forms/factories/hardcoded.h"
#include "../../../dovah/data/conditions.h"
#include "../../../dovah/data/story_manager.h"
#include "../../../dovah/forms/Quest.h"

namespace {
   constexpr int RunOnTypeRole           = Qt::ItemDataRole::UserRole;
   constexpr int RunOnFormIDRole         = Qt::ItemDataRole::UserRole + 1;
   constexpr int RunOnPlayerSentinelRole = Qt::ItemDataRole::UserRole + 2;
   
   namespace _arg_types {
      using namespace dovah::condition_parameter_types;
   }
}

bool ConditionEditDialog::_FunctionListProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
   auto* sm   = (QStandardItemModel*)this->sourceModel();
   auto* item = sm->item(source_row, 0);
   if (item) {
      if (item->data(ExcludeRole).toBool())
         return true;
   }
   return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

ConditionEditDialog::ConditionEditDialog(dovah::form_stub& containing_form, working_condition& wc, QWidget* parent) : QDialog(parent), context(containing_form), working(wc) {
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
   this->parameters[0].holder = this->ui.param1Holder;
   this->parameters[1].holder = this->ui.param2Holder;
   this->parameters[2].holder = this->ui.param3Holder;
   for (size_t i = 0; i < this->parameters.size(); ++i) {
      auto& p      = this->parameters[i];
      auto* layout = new QGridLayout;
      layout->setMargin(0);
      p.holder->setLayout(layout);
      //
      p.widget = new ConditionParameterEditor(containing_form, this->working, i, p.holder);
      layout->addWidget(p.widget);
   }
   #pragma endregion
   #pragma region Handle parameter flags
   {
      auto* widget = this->ui.paramFlags;
      widget->clear();
      widget->addItem(tr("Default", "condition param override flags"), (int)0);
      widget->addItem(tr("Use Aliases", "condition param override flags"), (int)condition_t::flag::use_aliases);
      widget->addItem(tr("Use Package Data", "condition param override flags"), (int)condition_t::flag::use_package_data);
      //
      if (working.flags & condition_t::flag::use_aliases)
         widget->setCurrentIndex(1);
      if (working.flags & condition_t::flag::use_package_data)
         widget->setCurrentIndex(2);
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         this->working.flags &= ~(condition_t::flag::use_aliases | condition_t::flag::use_package_data);
         this->working.flags |= this->ui.paramFlags->currentData().toInt();
         this->working.fix_parameter_types();
         for (auto& p : this->parameters)
            p.widget->rebuild();
      });
   }
   #pragma endregion
   #pragma region Handle run-on
   {
      auto* widget = this->ui.runOn;
      widget->clear(); // clear anything that might've been done in Qt Designer
      widget->addItem(tr("Subject",       "condition run on"), (int)condition_t::run_on_type::subject);
      widget->addItem(tr("Target",        "condition run on"), (int)condition_t::run_on_type::target);
      widget->addItem(tr("Reference",     "condition run on"), (int)condition_t::run_on_type::reference);
      widget->addItem(tr("Combat Target", "condition run on"), (int)condition_t::run_on_type::combat_target);
      widget->addItem(tr("Linked Ref",    "condition run on"), (int)condition_t::run_on_type::linked_ref);
      widget->addItem(tr("Alias",         "condition run on"), (int)condition_t::run_on_type::quest_alias);
      widget->addItem(tr("Package Data",  "condition run on"), (int)condition_t::run_on_type::package_data);
      widget->addItem(tr("Event Data",    "condition run on"), (int)condition_t::run_on_type::event_data);
      //
      widget->addItem(tr("Player", "condition run on"), (int)condition_t::run_on_type::reference);
      widget->setItemData(widget->count() - 1, uint32_t(dovah::hardcoded_form_ids::PlayerRef), RunOnFormIDRole);
      widget->setItemData(widget->count() - 1, true, RunOnPlayerSentinelRole);
      //
      this->_updateRunOn();
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         auto* widget = this->ui.runOn;
         //
         this->working.run_on.type      = (condition_t::run_on_type)this->ui.runOn->currentData().toInt();
         this->working.run_on.index     = -1;
         this->working.run_on.reference = nullptr;
         auto data = this->ui.runOn->currentData(RunOnFormIDRole);
         if (data.isValid()) {
            auto id = data.value<uint32_t>();
            this->working.run_on.reference = DovahKitCore::get().get_form(id);
         }
         //
         this->_updateRunOn();
      });
      QObject::connect(this->ui.runOnButton, &RefPickerButton::valueChanged, this, [this](dovah::form_stub* stub) {
         this->working.run_on.reference = stub;
         if (this->working.run_on.type != condition_t::run_on_type::reference)
            return;
         this->_updateRunOn();
      });
   }
   #pragma endregion
   cobb::qt::bind(this->ui.flagSwapSubjectAndTarget, this->working.flags, condition_t::flag::swap_subject_and_target);
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
         dovah::for_each_condition_function([model](const dovah::condition_function& func) {
            auto* item = new QStandardItem(func.name);
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
      widget->setCurrentIndex(widget->findData(this->working.function));
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         auto id = this->ui.function->currentData().toInt();
         if (!dovah::condition_function::lookup_by_id(id))
            return;
         this->working.function = id;
         this->working.reset_parameters();
         for (auto& p : this->parameters)
            p.widget->rebuild();
      });
   }
   #pragma endregion
   #pragma region Handle parameters
   {
      for (auto& p : this->parameters)
         p.widget->rebuild();
      //
      // Handle interdependent parameters:
      //
      QObject::connect(this->parameters[0].widget, &ConditionParameterEditor::valueChanged, this, [this]() {
         auto* func = dovah::condition_function::lookup_by_id(this->working.function);
         if (!func)
            return;
         if (func->uses_event_data) {
            this->parameters[1].widget->clear();
            this->parameters[2].widget->clear();
            return;
         }
         if (auto* type = func->argument_types[1]) {
            if (type->is_union()) {
               this->working.fix_parameter_types();
               this->parameters[1].widget->clear();
               return;
            }
         }
      });
      QObject::connect(this->parameters[1].widget, &ConditionParameterEditor::valueChanged, this, [this]() {
         auto* func = dovah::condition_function::lookup_by_id(this->working.function);
         if (!func || !func->uses_event_data)
            return;
         this->parameters[2].widget->clear();
      });
   }
   #pragma endregion
   #pragma region Handle operator
   {  // Result: Operator
      auto* widget = this->ui.op;
      widget->clear(); // clear anything that might've been done in Qt Designer
      widget->addItem(tr("==", "condition operator"), (int)condition_t::operator_type::equal);
      widget->addItem(tr("!=", "condition operator"), (int)condition_t::operator_type::not_equal);
      widget->addItem(tr(">",  "condition operator"), (int)condition_t::operator_type::greater);
      widget->addItem(tr(">=", "condition operator"), (int)condition_t::operator_type::greater_or_equal);
      widget->addItem(tr("<",  "condition operator"), (int)condition_t::operator_type::less);
      widget->addItem(tr("<=", "condition operator"), (int)condition_t::operator_type::less_or_equal);
      cobb::qt::bind(widget, this->working.comparison.op);
   }
   #pragma endregion
   #pragma region Handle operand
   {  // Result: Operand
      cobb::qt::remove_spinbox_bounds(this->ui.operandConstant);
      cobb::qt::bind(this->ui.operandConstant, this->working.comparison.operand.constant);
      //
      this->ui.operandGlobal->setAllowedFormType(dovah::form_type::global);
      this->ui.operandGlobal->populate();
      if (auto* stub = this->working.comparison.operand.global) {
         if (!stub->is_none_stub())
            this->ui.operandGlobal->setFormByID(stub->formID);
      }
      //
      bool use_global = (this->working.flags & working_condition::flag::compare_to_global);
      this->ui.flagCompareToGlobal->setCheckState(use_global ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
      if (use_global) {
         this->ui.operandStack->setCurrentWidget(this->ui.operandPageGlobal);
      } else {
         this->ui.operandStack->setCurrentWidget(this->ui.operandPageConstant);
      }
      QObject::connect(this->ui.flagCompareToGlobal, &QCheckBox::stateChanged, this, [this](int state) {
         if (state == Qt::CheckState::Checked) {
            this->ui.operandStack->setCurrentWidget(this->ui.operandPageGlobal);
         } else {
            this->ui.operandStack->setCurrentWidget(this->ui.operandPageConstant);
         }
      });
   }
   #pragma endregion
   cobb::qt::bind(this->ui.flagOr, this->working.flags, condition_t::flag::or_linked);
   //
   #pragma region Frontend signals
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->reject();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* target) {
      if (target == this->context.owner) {
         this->reject();
         return;
      }
      if (!this->working.sever_outbound_references_to(target))
         return;
      this->forceUpdateParameters();
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* target) {
      if (!this->working.refers_to_form(target))
         return;
      this->forceUpdateParameters();
   });
   QObject::connect(&editor, &DovahKitCore::formWorkingCopyDeleteComplete, this, [this](dovah::form_stub* target) {
      if (!this->working.refers_to_form(target))
         return;
      this->forceUpdateParameters();
   });
   QObject::connect(&editor, &DovahKitCore::questWorkingCopyStagesAltered, this, [this](dovah::form_stub* target) {
      if (this->working.parameters[1].underlying != dovah::condition_parameter_underlying_type::quest_stage)
         return;
      if (this->working.parameters[0].form != target)
         return;
      this->parameters[1].widget->rebuild();
   });
   QObject::connect(&editor, &DovahKitCore::questWorkingCopyAliasesAltered, this, [this](dovah::form_stub* target) {
      if (target != this->context.quest)
         return;
      for (int i = 0; i < this->working.parameters.size(); ++i) {
         if (this->working.parameters[i].underlying == dovah::condition_parameter_underlying_type::aliasID)
            this->parameters[i].widget->rebuild();
      }
      this->_updateRunOn();
   });
   QObject::connect(&editor, &DovahKitCore::packageWorkingCopyPackageDataAltered, this, [this](dovah::form_stub* target) {
      if (target != this->context.package)
         return;
      for (int i = 0; i < this->working.parameters.size(); ++i) {
         if (this->working.parameters[i].underlying == dovah::condition_parameter_underlying_type::package_data)
            this->parameters[i].widget->rebuild();
      }
      this->_updateRunOn();
   });
   #pragma endregion
}

void ConditionEditDialog::forceUpdateParameters() {
   this->parameters[0].widget->rebuild();
   this->parameters[1].widget->rebuild();
   this->parameters[2].widget->rebuild();
}

void ConditionEditDialog::_updateRunOn() {
   const auto blocker0 = QSignalBlocker(this->ui.runOnDropdown);
   const auto blocker1 = QSignalBlocker(this->ui.runOnButton);
   //
   bool is_player = this->ui.runOn->currentData(RunOnPlayerSentinelRole).toBool();
   switch (this->working.run_on.type) {
      case condition_t::run_on_type::subject:
      case condition_t::run_on_type::target:
      case condition_t::run_on_type::linked_ref:
      case condition_t::run_on_type::combat_target:
         {
            this->ui.runOnButton->setValue(nullptr);
            this->ui.runOnDropdown->setEnabled(false);
            this->ui.runOnDropdown->clear();
            this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         }
         return;
      case condition_t::run_on_type::reference:
         {
            dovah::form_stub* stub = this->working.run_on.reference;
            this->ui.runOnButton->setValue(stub);
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
      case condition_t::run_on_type::quest_alias:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnButton->setValue(nullptr);
         this->ui.runOnDropdown->setEnabled(true);
         this->ui.runOnDropdown->clear();
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* q = this->context.get_owning_quest()) {
            auto* widget = this->ui.runOnDropdown;
            q->for_each_alias_of_type(dovah::loaded_forms::Alias::alias_type::reference, [widget](dovah::loaded_forms::Alias* alias) {
               widget->addItem(alias->name.c_str(), alias->id);
               return false;
            });
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(this->working.run_on.index));
         return;
      case condition_t::run_on_type::package_data:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnButton->setValue(nullptr);
         this->ui.runOnDropdown->setEnabled(true);
         this->ui.runOnDropdown->clear();
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* p = this->context.get_owning_package()) {
            //
            // TODO: package data
            //
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(this->working.run_on.index));
         return;
      case condition_t::run_on_type::event_data:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnButton->setValue(nullptr);
         this->ui.runOnDropdown->clear();
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* q = this->context.get_owning_quest()) {
            auto  code = q->event;
            auto* def  = dovah::story_event_definition::lookup(code);
            if (def)
               for (auto& data : def->members)
                  this->ui.runOnDropdown->addItem(data.name, dovah::story_event_definition::widen_member_code(data.signature));
            this->ui.runOnDropdown->setEnabled(def != nullptr);
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(this->working.run_on.index));
         return;
   }
}

void ConditionEditDialog::showEvent(QShowEvent* event) {
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