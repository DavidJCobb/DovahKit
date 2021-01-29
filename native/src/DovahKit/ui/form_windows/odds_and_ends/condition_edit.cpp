#include "condition_edit.h"
#include "../../../helpers/qt/basic_bindings.h"
#include "../../../helpers/qt/spinbox.h"

namespace {
   constexpr int RunOnTypeRole           = Qt::ItemDataRole::UserRole;
   constexpr int RunOnFormIDRole         = Qt::ItemDataRole::UserRole + 1;
   constexpr int RunOnPlayerSentinelRole = Qt::ItemDataRole::UserRole + 2;
}

ConditionEditDialog::ConditionEditDialog(loaded_form_t& containing_form, condition_t& c, QWidget* parent) : QDialog(parent), form(containing_form), condition(c) {
   ui.setupUi(this);
   //
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      this->_save();
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->reject();
   });
   //
   {
      auto* widget = this->ui.runOn;
      widget->clear(); // clear anything that might've been done in Qt Designer
      widget->addItem(tr("Subject",       "condition run on"), (int)condition_t::run_on_t::subject);
      widget->addItem(tr("Target",        "condition run on"), (int)condition_t::run_on_t::target);
      widget->addItem(tr("Reference",     "condition run on"), (int)condition_t::run_on_t::reference);
      widget->addItem(tr("Combat Target", "condition run on"), (int)condition_t::run_on_t::combat_target);
      widget->addItem(tr("Linked Ref",    "condition run on"), (int)condition_t::run_on_t::linked_ref);
      widget->addItem(tr("Alias",         "condition run on"), (int)condition_t::run_on_t::package_data);
      widget->addItem(tr("Package Data",  "condition run on"), (int)condition_t::run_on_t::subject);
      widget->addItem(tr("Event Data",    "condition run on"), (int)condition_t::run_on_t::event_data);
      //
      widget->addItem(tr("Player", "condition run on"), (int)condition_t::run_on_t::reference);
      widget->setItemData(widget->count() - 1, uint32_t(0x00000014), RunOnFormIDRole);
      widget->setItemData(widget->count() - 1, true, RunOnPlayerSentinelRole);
      //
      if (condition.run_on.type == condition_t::run_on_t::reference) {
         auto* stub = condition.run_on.reference.get_form_stub();
         if (stub && stub->formID == 0x00000014) {
            widget->setCurrentIndex(widget->findData(true, RunOnPlayerSentinelRole));
         } else {
            widget->setCurrentIndex(widget->findData((int)condition.run_on.type, RunOnTypeRole));
         }
      } else {
         widget->setCurrentIndex(widget->findData((int)condition.run_on.type));
      }
      //
      // TODO: the button or combobox
      //
      #if !_DEBUG
         static_assert(false, "Finish implementing Run On: you need to be able to set (and display) the specific reference, alias, etc.!");
         static_assert(false, "Finish implementing Run On: we need code for when the run-on-type combobox is changed!");
      #endif
   }
   cobb::qt::bind(this->ui.flagSwapSubjectAndTarget, condition.flags, condition_t::flag::swap_subject_and_target);
   {
      auto* widget = this->ui.function;
      widget->clear(); // clear anything that might've been done in Qt Designer
      for (auto& func : dovah::loaded_forms::components::condition_info::function_list)
         widget->addItem(func.name, func.id);
      for (auto& func : dovah::loaded_forms::components::condition_info::extended_function_list)
         widget->addItem(func.name, func.id);
      //
      widget->setCurrentIndex(widget->findData(condition.function));
      //
      // TODO: code to filter the function list
      //
      // TODO: code to change the function
      //
      #if !_DEBUG
         static_assert(false, "Finish implementing Function: you need to be able to filter the function list (with the current selection exempt from the filter)!");
         static_assert(false, "Finish implementing Function: we need code for when the function combobox is changed!");
      #endif
   }
   {
      //
      // TODO: Parameters
      //
      #if !_DEBUG
         static_assert(false, "Finish implementing Parameters: the use aliases/packdata drop-down!");
         static_assert(false, "Finish implementing Parameters: displaying the values!");
         static_assert(false, "Finish implementing Parameters: editing the values!");
      #endif
   }
   {  // Result: Operator
      auto* widget = this->ui.op;
      widget->clear(); // clear anything that might've been done in Qt Designer
      widget->addItem(tr("==", "condition operator"), (int)condition_t::operator_t::equal);
      widget->addItem(tr("!=", "condition operator"), (int)condition_t::operator_t::not_equal);
      widget->addItem(tr(">",  "condition operator"), (int)condition_t::operator_t::greater);
      widget->addItem(tr(">=", "condition operator"), (int)condition_t::operator_t::greater_or_equal);
      widget->addItem(tr("<",  "condition operator"), (int)condition_t::operator_t::less);
      widget->addItem(tr("<=", "condition operator"), (int)condition_t::operator_t::less_or_equal);
      cobb::qt::bind(widget, condition.comparison.op);
   }
   {  // Result: Operand
      cobb::qt::remove_spinbox_bounds(this->ui.operandConstant);
      cobb::qt::bind(this->ui.operandConstant, condition.comparison.operand.constant);
      //
      #if !_DEBUG
         static_assert(false, "Finish implementing Compare To Global: the drop-down!");
         static_assert(false, "Finish implementing Compare To Global: the checkbox!");
      #endif
   }
   cobb::qt::bind(this->ui.flagOr, condition.flags, condition_t::flag::or_linked);
}

void ConditionEditDialog::_save() {
   //
   // TODO
   //
   #if !_DEBUG
      static_assert(false, "Finish implementing me!");
   #endif
}