#include "condition_edit.h"
#include <QShowEvent>
#include "../../../helpers/qt/basic_bindings.h"
#include "../../../helpers/qt/spinbox.h"
#include "../../../helpers/qt/strings.h"
#include "../../generic/FormsOfTypeCombobox.h"
#include "../../generic/RefPickerButton.h"

namespace {
   constexpr int RunOnTypeRole           = Qt::ItemDataRole::UserRole;
   constexpr int RunOnFormIDRole         = Qt::ItemDataRole::UserRole + 1;
   constexpr int RunOnPlayerSentinelRole = Qt::ItemDataRole::UserRole + 2;
   
   namespace _arg_types {
      using namespace dovah::loaded_forms::components::condition_info::arg_types;
   }
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
      if (condition.flags & condition_t::flag::use_aliases)
         widget->setCurrentIndex(1);
      if (condition.flags & condition_t::flag::use_packdata)
         widget->setCurrentIndex(2);
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         int   id   = this->ui.function->currentData().toInt();
         auto* func = dovah::loaded_forms::components::condition_info::function::lookup_by_id(id);
         if (func) {
            bool update = false;
            for (auto* type : func->argument_types) {
               if (type && type->can_be_alias) {
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
                  if (under_override != underlying_t::none && type->can_be_alias)
                     a = under_override;
               }
               if (auto* type = func->argument_types[1]) {
                  b = type->underlying;
                  if (under_override != underlying_t::none && type->can_be_alias)
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
         if (func.valid)
            widget->addItem(func.name, func.id);
      for (auto& func : dovah::loaded_forms::components::condition_info::extended_function_list)
         if (func.valid)
            widget->addItem(func.name, func.id);
      if (auto* model = widget->model())
         model->sort(0);
      //
      widget->setCurrentIndex(widget->findData(condition.function));
      //
      // TODO: code to filter the function list
      //
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         int   id   = this->ui.function->currentData().toInt();
         auto* func = dovah::loaded_forms::components::condition_info::function::lookup_by_id(id);
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
               if (under_override != underlying_t::none && type->can_be_alias)
                  a = under_override;
            }
            if (auto* type = func->argument_types[1]) {
               b = type->underlying;
               if (under_override != underlying_t::none && type->can_be_alias)
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
      this->ui.operandGlobal->setAllowedFormType(dovah::form_type::global);
      this->ui.operandGlobal->populate();
      if (auto* stub = condition.comparison.operand.global.get_form_stub()) {
         if (!stub->is_none_stub())
            this->ui.operandGlobal->setFormByID(stub->formID);
      }
      //
      bool use_global = (condition.flags & condition_t::flag::compare_to_global);
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
   cobb::qt::bind(this->ui.flagOr, condition.flags, condition_t::flag::or_linked);
}

void ConditionEditDialog::_buildParamControls(int which, underlying_t under, param_type_t* type, bool use_original) {
   if (which < 0 || which > this->parameters.size())
      return;
   auto& p = this->parameters[which];
   if (p.last_type == type && p.last_under == under)
      return;
   //
   if (which >= 2)
      use_original = false;
   if (type && type->can_be_alias) {
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
      for (auto ft : type->allowedFormTypes) {
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
            w->setValue(this->condition.parameters[which].form.get_form_stub());
      }
   }
   //
   if (!p.widget) {
      switch (under) {
         case underlying_t::aliasID:
            {
               auto* w = new QComboBox;
               //
               // TODO: alias names and IDs
               //
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(this->condition.parameters[which].dword));
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
                  w->setCurrentIndex(w->findData(this->condition.parameters[which].dword));
            } else {
               auto* w = new QLineEdit;
               w->setMaxLength(1);
               p.widget = w;
               //
               if (use_original)
                  w->setText(QChar(this->condition.parameters[which].dword & 0xFF));
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
                  w->setValue((int32_t)this->condition.parameters[which].float32);
            }
            break;
         case underlying_t::formID:
            {
               auto* w = new FormsOfTypeCombobox;
               if (type && type->allowedFormTypes.size()) {
                  QVector<uint8_t> al;
                  al.reserve(type->allowedFormTypes.size());
                  for (auto i : type->allowedFormTypes)
                     al.push_back(i);
                  w->setAllowedFormTypes(al);
               }
               w->setAllowNone(true);
               w->populate();
               p.widget = w;
               //
               if (use_original) {
                  dovah::bare_form_id_t id = 0;
                  auto* stub = this->condition.parameters[which].form.get_form_stub();
                  if (stub)
                     id = stub->formID;
                  w->setFormByID(id);
               }
            }
            break;
         case underlying_t::int_signed:
            if (type && type->isEnum) {
               auto* w = new QComboBox;
               for (auto& evd : type->enumValues)
                  w->addItem(evd.string, evd.value);
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(this->condition.parameters[which].dword));
            } else {
               auto* w = new QSpinBox;
               w->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
               p.widget = w;
               //
               if (use_original)
                  w->setValue((int32_t)this->condition.parameters[which].dword);
            }
            break;
         case underlying_t::int_unsigned:
            {
               auto* w = new QSpinBox;
               w->setRange(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
               p.widget = w;
               //
               if (use_original)
                  w->setValue(this->condition.parameters[which].dword);
            }
            break;
         case underlying_t::none:
            break;
         case underlying_t::package_data:
            {
               auto* w = new QComboBox;
               //
               // TODO: package data names and indices
               //
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(this->condition.parameters[which].dword));
            }
            break;
         case underlying_t::quest_stage:
            {
               auto* w = new QComboBox;
               //
               // TODO: quest stage numbers
               //
               p.widget = w;
               //
               if (use_original)
                  w->setCurrentIndex(w->findData(this->condition.parameters[which].dword));
            }
            break;
         case underlying_t::string:
            {
               auto* w = new QLineEdit;
               p.widget = w;
               //
               if (use_original)
                  w->setText(this->condition.parameters[which].string.c_str());
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