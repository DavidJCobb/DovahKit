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

ConditionEditDialog::ConditionEditDialog(form_stub& containing_form, condition_t& c, QWidget* parent) : QDialog(parent), context(containing_form), condition(c), working(c.make_working_copy()) {
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
   this->parameters[0].holder = this->ui.param1Holder;
   this->parameters[1].holder = this->ui.param2Holder;
   this->parameters[2].holder = this->ui.param3Holder;
   for (auto& p : this->parameters) {
      auto* layout = new QGridLayout;
      layout->setMargin(0);
      p.holder->setLayout(layout);
   }
   {
      auto* widget = this->ui.paramFlags;
      widget->clear();
      widget->addItem(tr("Default", "condition param override flags"), (int)underlying_t::none);
      widget->addItem(tr("Use Aliases", "condition param override flags"), (int)underlying_t::aliasID);
      widget->addItem(tr("Use Package Data", "condition param override flags"), (int)underlying_t::package_data);
      //
      if (working.flags & condition_t::flag::use_aliases)
         widget->setCurrentIndex(1);
      if (working.flags & condition_t::flag::use_package_data)
         widget->setCurrentIndex(2);
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         int   id   = this->ui.function->currentData().toInt();
         auto* func = dovah::condition_function::lookup_by_id(id);
         if (func) {
            bool update = false;
            for (auto* type : func->argument_types) {
               if (type && type->allow_overrides) {
                  update = true;
                  break;
               }
            }
            if (update) {
               underlying_t under_override = (underlying_t)this->ui.paramFlags->currentIndex();
               //
               underlying_t a = underlying_t::none;
               underlying_t b = underlying_t::none;
               if (auto* type = func->argument_types[0]) {
                  a = type->underlying;
                  if (under_override != underlying_t::none && type->allow_overrides)
                     a = under_override;
               }
               if (auto* type = func->argument_types[1]) {
                  b = type->underlying;
                  if (under_override != underlying_t::none && type->allow_overrides)
                     b = under_override;
               }
               //
               this->_buildParamControls(0, a, func->argument_types[0]);
               this->_buildParamControls(1, b, func->argument_types[1]);
               this->_buildParamControls(2, underlying_t::none, nullptr);
            }
         }
      });
   }
   //
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
      if (this->working.run_on.type == condition_t::run_on_type::reference) {
         auto* stub = this->working.run_on.reference;
         if (stub && stub->formID == dovah::hardcoded_form_ids::PlayerRef) {
            widget->setCurrentIndex(widget->findData(true, RunOnPlayerSentinelRole));
         } else {
            widget->setCurrentIndex(widget->findData((int)this->working.run_on.type, RunOnTypeRole));
         }
      } else {
         widget->setCurrentIndex(widget->findData((int)this->working.run_on.type));
      }
      this->_updateRunOn(true);
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         this->_updateRunOn();
      });
      QObject::connect(this->ui.runOnButton, &RefPickerButton::valueChanged, this, [this](dovah::form_stub* stub) {
         if (!stub)
            return;
         if (stub->formID == dovah::hardcoded_form_ids::PlayerRef) {
            auto* widget = this->ui.runOn;
            const auto blocker = QSignalBlocker(widget);
            widget->setCurrentIndex(widget->findData(true, RunOnPlayerSentinelRole));
         }
      });
   }
   cobb::qt::bind(this->ui.flagSwapSubjectAndTarget, this->working.flags, condition_t::flag::swap_subject_and_target);
   {
      auto* widget = this->ui.function;
      widget->clear(); // clear anything that might've been done in Qt Designer
      //
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
      widget->setCurrentIndex(widget->findData(condition.get_function_id()));
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
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         int   id   = this->ui.function->currentData().toInt();
         auto* func = dovah::condition_function::lookup_by_id(id);
         if (!func) {
            this->_buildParamControls(0, underlying_t::none, nullptr);
            this->_buildParamControls(1, underlying_t::none, nullptr);
            this->_buildParamControls(2, underlying_t::none, nullptr);
         }
         if (func->uses_event_data) {
            this->_buildParamControls(0, underlying_t::event, nullptr);
            this->_buildParamControls(1, underlying_t::event, nullptr);
            this->_buildParamControls(2, underlying_t::event, nullptr);
         } else {
            underlying_t under_override = (underlying_t)this->ui.paramFlags->currentIndex();
            //
            underlying_t a = underlying_t::none;
            underlying_t b = underlying_t::none;
            if (auto* type = func->argument_types[0]) {
               a = type->underlying;
               if (under_override != underlying_t::none && type->allow_overrides)
                  a = under_override;
            }
            if (auto* type = func->argument_types[1]) {
               b = type->underlying;
               if (under_override != underlying_t::none && type->allow_overrides)
                  b = under_override;
            }
            //
            this->_buildParamControls(0, a, func->argument_types[0]);
            this->_buildParamControls(1, b, func->argument_types[1]);
            this->_buildParamControls(2, underlying_t::none, nullptr);
         }
      });
      #if !_DEBUG
         static_assert(false, "Finish implementing Function: you need to be able to filter the function list (with the current selection exempt from the filter)!");
         static_assert(false, "Finish implementing Function: we need code for when the function combobox is changed!");
      #endif
   }
   {
      for (int i = 0; i < this->parameters.size(); ++i) {
         underlying_t  under = condition.get_argument_underlying_type(i);
         param_type_t* type  = condition.get_argument_type(i);
         this->_buildParamControls(i, under, type, true);
      }
      #if !_DEBUG
         static_assert(false, "Finish implementing Parameters: editing the values!");
      #endif
   }
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
   {  // Result: Operand
      cobb::qt::remove_spinbox_bounds(this->ui.operandConstant);
      cobb::qt::bind(this->ui.operandConstant, this->working.comparison.operand.constant);
      //
      this->ui.operandGlobal->setAllowedFormType(dovah::form_type::global);
      this->ui.operandGlobal->populate();
      if (auto* stub = condition.get_comparison().operand.global.get_form_stub()) {
         if (!stub->is_none_stub())
            this->ui.operandGlobal->setFormByID(stub->formID);
      }
      //
      bool use_global = (condition.get_flags() & condition_t::flag::compare_to_global);
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
   cobb::qt::bind(this->ui.flagOr, this->working.flags, condition_t::flag::or_linked);
}

void ConditionEditDialog::_buildParamControls(int which, underlying_t under, param_type_t* type, bool use_original, bool force_update) {
   if (which < 0 || which > this->working.parameters.size())
      return;
   auto& p     = this->parameters[which];
   auto  value = this->condition.get_parameter(which);
   if (!force_update && p.last_type == type && p.last_under == under)
      return;
   //
   if (which >= 2)
      use_original = false;
   if (type && type->allow_overrides) {
      auto index = this->ui.paramFlags->currentIndex();
      if (index == 1)
         under = underlying_t::aliasID;
      else if (index == 2)
         under = underlying_t::package_data;
   }
   //
   if (p.widget) {
      p.widget->deleteLater();
      p.widget = nullptr;
   }
   //
   _parameter::special_case_t sc = _parameter::special_case_t::none;
   if (type && under == underlying_t::formID) {
      bool references = false;
      for (auto ft : type->allowed_form_types) {
         if (dovah::form_type_info::form_type_is_reference(ft)) {
            references = true;
            break;
         }
      }
      if (references) {
         sc = _parameter::special_case_t::reference_pick_button;
         //
         auto* w = new RefPickerButton;
         auto& p = this->parameters[which];
         p.widget = w;
         //
         if (use_original)
            w->setValue(value.form);
      }
   }
   //
   if (!p.widget) {
      switch (under) {
         case underlying_t::aliasID:
            {
               auto* w = new QComboBox;
               if (auto* q = this->context.get_owning_quest()) {
                  w->addItem(tr("NONE"), -1);
                  for (auto* alias : q->aliases) {
                     if (alias->type != dovah::loaded_forms::Alias::alias_type::reference)
                        continue;
                     w->addItem(alias->name.c_str(), alias->id);
                  }
               }
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(value.dword));
            }
            break;
         case underlying_t::character:
            if (type == &_arg_types::Axis) {
               auto* w = new QComboBox;
               w->addItem("X", uint8_t('X'));
               w->addItem("Y", uint8_t('Y'));
               w->addItem("Z", uint8_t('Z'));
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(value.dword));
            } else {
               auto* w = new QLineEdit;
               w->setMaxLength(1);
               p.widget = w;
               //
               if (use_original)
                  w->setText(QChar(value.dword & 0xFF));
            }
            break;
         case underlying_t::event:
            switch (which) {
               case 0:
                  //
                  // TODO: event function
                  //
                  break;
               case 1:
                  //
                  // TODO: event member
                  //
                  break;
               case 2:
                  //
                  // TODO: event form
                  //
                  break;
            }
            break;
         case underlying_t::float32:
            {
               auto* w = new QDoubleSpinBox;
               w->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
               p.widget = w;
               //
               if (use_original)
                  w->setValue((int32_t)value.float32);
            }
            break;
         case underlying_t::formID:
            {
               auto* w = new FormsOfTypeCombobox;
               if (type && type->allowed_form_types.size()) {
                  QVector<uint8_t> al;
                  al.reserve(type->allowed_form_types.size());
                  for (auto i : type->allowed_form_types)
                     al.push_back(i);
                  w->setAllowedFormTypes(al);
               }
               w->setAllowNone(true);
               w->populate();
               p.widget = w;
               //
               if (use_original) {
                  dovah::bare_form_id_t id = 0;
                  auto* stub = value.form;
                  if (stub)
                     id = stub->formID;
                  w->setFormByID(id);
               }
            }
            break;
         case underlying_t::int_signed:
            if (type && type->is_enum) {
               auto* w = new QComboBox;
               for (auto& evd : type->enum_values)
                  w->addItem(evd.name, evd.value);
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(value.integer));
            } else {
               auto* w = new QSpinBox;
               w->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
               p.widget = w;
               //
               if (use_original)
                  w->setValue((int32_t)value.integer);
            }
            break;
         case underlying_t::int_unsigned:
            {
               auto* w = new QSpinBox;
               w->setRange(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
               p.widget = w;
               //
               if (use_original)
                  w->setValue(value.dword);
            }
            break;
         case underlying_t::none:
            break;
         case underlying_t::package_data:
            {
               auto* w = new QComboBox;
               if (this->context.package) {
                  w->addItem(tr("NONE"), -1);
                  //
                  // TODO: package data names and indices
                  //
               }
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(value.dword));
            }
            break;
         case underlying_t::quest_stage:
            {
               if (which > 0 && this->parameters[which - 1].last_under == underlying_t::formID) {
                  auto* w = new QComboBox;
                  //
                  auto* prev  = dynamic_cast<FormsOfTypeCombobox*>(this->parameters[which - 1].widget);
                  assert(prev);
                  auto* quest = prev->formStub();
                  if (quest && quest->formType == dovah::form_type::quest) {
                     using loaded_form_t = dovah::loaded_forms::Quest;
                     //
                     dovah::loaded_form_ptr<loaded_form_t> keep_alive;
                     loaded_form_t* q = quest->get_working_or_stable_copy(keep_alive);
                     //
                     if (q)
                        for (auto& s : q->stages)
                           w->addItem(QString::number(s.index), s.index);
                  }
                  //
                  p.widget = w;
                  //
                  if (use_original)
                     w->setCurrentIndex(w->findData(value.dword));
               }
               {
                  auto* w = new QSpinBox;
                  w->setRange(0, 65535);
                  p.widget = w;
                  //
                  if (use_original)
                     w->setValue(value.dword);
               }
            }
            break;
         case underlying_t::string:
            {
               auto* w = new QLineEdit;
               p.widget = w;
               //
               if (use_original)
                  w->setText(value.string.c_str());
            }
            break;
      }
   }
   if (p.widget) {
      p.holder->layout()->addWidget(p.widget);
   }
   p.last_type    = type;
   p.last_under   = under;
   p.last_special = sc;
}

void ConditionEditDialog::_updateRunOn(bool use_original) {
   struct {
      int combobox;
      dovah::form_stub* reference = nullptr;
   } _prior;
   _prior.combobox  = this->ui.runOnDropdown->currentData().toInt();
   _prior.reference = this->ui.runOnButton->value();
   //
   this->ui.runOnDropdown->clear();
   //
   auto ro = (condition_t::run_on_type) this->ui.runOn->currentData().toInt();
   auto is_player = this->ui.runOn->currentData(RunOnPlayerSentinelRole).toBool();
   dovah::bare_form_id_t refID = this->ui.runOn->currentData(RunOnFormIDRole).toInt();
   switch (ro) {
      case condition_t::run_on_type::subject:
      case condition_t::run_on_type::target:
      case condition_t::run_on_type::linked_ref:
      case condition_t::run_on_type::combat_target:
         {
            this->ui.runOnDropdown->setEnabled(false);
            this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         }
         return;
      case condition_t::run_on_type::reference:
         {
            dovah::form_stub* stub = nullptr;
            if (use_original) {
               stub = this->working.run_on.reference;
            } else if (refID) {
               stub = DovahKitCore::get().get_form(refID);
            }
            this->ui.runOnButton->setValue(stub);
            this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageButton);
            //
            if (stub && stub->formID == dovah::hardcoded_form_ids::PlayerRef) {
               if (!is_player) {
                  const auto blocker = QSignalBlocker(this->ui.runOn);
                  this->ui.runOn->setCurrentIndex(this->ui.runOn->findData(true, RunOnPlayerSentinelRole));
               }
               this->ui.runOnDropdown->setEnabled(false);
               this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
            }
         }
         return;
      case condition_t::run_on_type::quest_alias:
         this->ui.runOnDropdown->setEnabled(true);
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* q = this->context.get_owning_quest()) {
            for (auto* alias : q->aliases) {
               if (alias->type != dovah::loaded_forms::Alias::alias_type::reference)
                  continue;
               this->ui.runOnDropdown->addItem(alias->name.c_str(), alias->id);
            }
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         if (use_original) {
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(this->working.run_on.index));
         }
         return;
      case condition_t::run_on_type::package_data:
         this->ui.runOnDropdown->setEnabled(true);
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
         this->ui.runOnDropdown->addItem(tr("NONE"), -1);
         if (auto* p = this->context.get_owning_package()) {
            //
            // TODO: package data
            //
         } else {
            this->ui.runOnDropdown->setEnabled(false);
         }
         if (use_original) {
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(this->working.run_on.index));
         }
         return;
      case condition_t::run_on_type::event_data:
         this->ui.runOnStack->setCurrentWidget(this->ui.runOnPageDropdown);
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
         if (use_original) {
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(this->working.run_on.index));
         }
         return;
   }
   if (ro == this->last_run_on) {
      switch (ro) {
         case condition_t::run_on_type::reference:
            this->ui.runOnButton->setValue(_prior.reference);
            break;
         case condition_t::run_on_type::quest_alias:
         case condition_t::run_on_type::package_data:
         case condition_t::run_on_type::event_data:
            this->ui.runOnDropdown->setCurrentIndex(this->ui.runOnDropdown->findData(_prior.combobox));
            break;
      }
   }
   this->last_run_on = ro;
}

void ConditionEditDialog::_save() {
   //
   // TODO
   //
   #if !_DEBUG
      static_assert(false, "Finish implementing me!");
   #endif
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
   for (auto& p : this->parameters) {
      p.holder->setMinimumHeight(height);
   }
}