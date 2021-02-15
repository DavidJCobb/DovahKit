#include "condition_param.h"
#include <QGridLayout>
#include <QVariant>
#include "../../../dovah/data/story_manager.h"
#include "../../../dovah/forms/Quest.h"

namespace {
   using _char_value_t = uint8_t;
   constexpr int _max_decimals_for_float = FLT_MAX_10_EXP + FLT_DIG; // per Qt documentation for QDoubleSpinBox

   namespace _arg_types {
      using namespace dovah::condition_parameter_types;
   }
}

std::array<QSignalBlocker, 6> ConditionParameterEditor::_getSubwidgetsBlocker() {
   return {
      QSignalBlocker(this->subwidgets.blank),
      QSignalBlocker(this->subwidgets.combobox),
      QSignalBlocker(this->subwidgets.form),
      QSignalBlocker(this->subwidgets.ref),
      QSignalBlocker(this->subwidgets.spinbox),
      QSignalBlocker(this->subwidgets.textbox),
   };
}

bool ConditionParameterEditor::_is_event_parameter() const noexcept {
   auto* func = dovah::condition_function::lookup_by_id(this->working.function);
   if (func && func->uses_event_data)
      return true;
   return false;
}
dovah::loaded_forms::components::condition_parameter& ConditionParameterEditor::_get_parameter() const noexcept {
   return this->working.parameters[this->parameter_index];
}
const dovah::condition_parameter_type* ConditionParameterEditor::_get_parameter_type() const noexcept {
   return this->working.get_argument_type(this->parameter_index);
}
const dovah::condition_function* ConditionParameterEditor::_get_condition_function() const noexcept {
   return dovah::condition_function::lookup_by_id(this->working.function);
}
dovah::loaded_forms::components::condition_parameter& ConditionParameterEditor::_get_previous_parameter() const noexcept {
   return this->working.parameters[this->parameter_index - 1];
}

void ConditionParameterEditor::_setCurrentWidget(QWidget* widget) {
   this->stack->setCurrentWidget(widget->parentWidget());
}

ConditionParameterEditor::ConditionParameterEditor(dovah::form_stub& containing_form, dovah::loaded_forms::components::working_condition& condition, int index, QWidget* parent)
   :
   QWidget(parent),
   working(condition),
   context(containing_form),
   parameter_index(index)
{
   {
      auto* layout = new QGridLayout;
      layout->setMargin(0);
      this->setLayout(layout);
   }
   this->stack = new QStackedWidget(this);
   this->layout()->addWidget(this->stack);
   auto lambda = [this](QWidget* widget) {
      auto* wrapper = new QWidget;
      auto* layout  = new QGridLayout;
      layout->setMargin(0);
      layout->addWidget(widget);
      wrapper->setLayout(layout);
      this->stack->addWidget(wrapper);
   };
   lambda(this->subwidgets.blank    = new QWidget);
   lambda(this->subwidgets.combobox = new QComboBox);
   lambda(this->subwidgets.textbox  = new QLineEdit);
   lambda(this->subwidgets.spinbox  = new QDoubleSpinBox);
   lambda(this->subwidgets.form     = new FormsOfTypeCombobox);
   lambda(this->subwidgets.ref      = new RefPickerButton);
   //
   this->subwidgets.spinbox->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
   this->subwidgets.form->setAllowNone(true);
   //
   QObject::connect(this->subwidgets.combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      auto* widget = this->subwidgets.combobox;
      //
      if (this->_is_event_parameter()) {
         auto& ep = this->working.event_parameters;
         switch (this->parameter_index) {
            case 0:
               ep.function = this->subwidgets.combobox->currentData().toInt();
               break;
            case 1:
               ep.member = this->subwidgets.combobox->currentData().toInt();
               break;
         }
         emit this->valueChanged();
         return;
      }
      if (this->parameter_index >= 2)
         return;
      //
      auto& param = this->_get_parameter();
      switch (param.underlying) {
         case dovah::condition_parameter_underlying_type::aliasID:
         case dovah::condition_parameter_underlying_type::character:
         case dovah::condition_parameter_underlying_type::int_signed:
         case dovah::condition_parameter_underlying_type::package_data:
         case dovah::condition_parameter_underlying_type::quest_stage:
            param.integer = widget->currentData().toInt();
            break;
         case dovah::condition_parameter_underlying_type::int_unsigned:
            param.dword = widget->currentData().value<uint32_t>();
            break;
         case dovah::condition_parameter_underlying_type::float32:
            param.float32 = widget->currentData().toFloat();
            break;
         default:
            return;
      }
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.textbox, &QLineEdit::textChanged, this, [this]() {
      if (this->_is_event_parameter()) {
         return;
      }
      if (this->parameter_index >= 2)
         return;
      //
      this->_get_parameter().string = this->subwidgets.textbox->text().toStdString();
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
      if (this->_is_event_parameter()) {
         return;
      }
      if (this->parameter_index >= 2)
         return;
      //
      auto& param = this->_get_parameter();
      switch (param.underlying) {
         case dovah::condition_parameter_underlying_type::float32:
            param.float32 = this->subwidgets.spinbox->value();
            break;
         case dovah::condition_parameter_underlying_type::int_signed:
            param.integer = this->subwidgets.spinbox->value();
            break;
         case dovah::condition_parameter_underlying_type::int_unsigned:
         case dovah::condition_parameter_underlying_type::quest_stage:
            param.dword = this->subwidgets.spinbox->value();
            break;
         default:
            return;
      }
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.form, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      auto* stub = this->subwidgets.form->formStub();
      //
      if (this->_is_event_parameter()) {
         if (this->parameter_index == 2) {
            this->working.event_parameters.form = stub;
            emit this->valueChanged();
         }
         return;
      }
      if (this->parameter_index >= 2)
         return;
      //
      this->_get_parameter().form = stub;
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.ref, &RefPickerButton::valueChanged, this, [this]() {
      auto* stub = this->subwidgets.ref->value();
      //
      if (this->_is_event_parameter()) {
         if (this->parameter_index == 2) {
            this->working.event_parameters.form = stub;
            emit this->valueChanged();
         }
         return;
      }
      if (this->parameter_index >= 2)
         return;
      //
      this->_get_parameter().form = stub;
      emit this->valueChanged();
   });
}

QWidget* ConditionParameterEditor::currentSubwidget() const noexcept {
   return this->stack->currentWidget();
}

void ConditionParameterEditor::clear() {
   auto* func = this->_get_condition_function();
   if (func && func->uses_event_data) {
      switch (this->parameter_index) {
         case 0:
            this->working.event_parameters.function = 0;
            break;
         case 1:
            this->working.event_parameters.member = 0;
            break;
         case 2:
            this->working.event_parameters.form = nullptr;
            break;
      }
      this->rebuild();
      return;
   }
   if (this->parameter_index >= 2)
      return;
   //
   auto& param = this->_get_parameter();
   param.form  = nullptr;
   param.string.clear();
   switch (param.underlying) {
      case dovah::condition_parameter_underlying_type::aliasID:
      case dovah::condition_parameter_underlying_type::package_data:
         param.dword = -1;
         break;
      default:
         param.dword = 0;
   }
   this->rebuild();
}
void ConditionParameterEditor::rebuild() {
   auto  blocker = this->_getSubwidgetsBlocker();
   auto* func    = this->_get_condition_function();
   if (func && func->uses_event_data) {
      this->_rebuildForEvents();
      return;
   }
   if (this->parameter_index >= this->working.parameters.size()) {
      this->_setCurrentWidget(this->subwidgets.blank);
      return;
   }
   //
   auto& param = this->_get_parameter();
   auto* type  = this->_get_parameter_type();
   if (!type)
      type = &_arg_types::None;
   if (type->is_enum) {
      this->_setCurrentWidget(this->subwidgets.combobox);
      //
      auto* w = this->subwidgets.combobox;
      const auto blocker = QSignalBlocker(w);
      w->clear();
      //
      int i;
      if (type->underlying == dovah::condition_parameter_underlying_type::int_unsigned) {
         for (const auto& e : type->enum_values)
            w->addItem(e.name, (uint32_t)e.value);
         i = w->findData(param.dword);
      } else {
         for (const auto& e : type->enum_values)
            w->addItem(e.name, e.value);
         i = w->findData(param.integer);
      }
      if (i < 0)
         i = 0;
      w->setCurrentIndex(i);
      //
      return;
   }
   switch (param.underlying) {
      case dovah::condition_parameter_underlying_type::none:
         this->_setCurrentWidget(this->subwidgets.blank);
         break;
      case dovah::condition_parameter_underlying_type::aliasID:
         this->_setCurrentWidget(this->subwidgets.combobox);
         {
            auto* w = this->subwidgets.combobox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->addItem(tr("NONE"), -1);
            if (auto* q = this->context.get_owning_quest()) {
               //
               // TODO: Can we set up a custom proxy model to sort these?
               //
               q->for_each_alias_of_type(dovah::loaded_forms::Alias::alias_type::reference, [w](dovah::loaded_forms::Alias* alias) {
                  w->addItem(alias->name.c_str(), alias->id);
                  return false;
               });
            }
            auto i = w->findData(param.dword);
            if (i < 0)
               i = 0;
            w->setCurrentIndex(i);
         }
         break;
      case dovah::condition_parameter_underlying_type::character:
         if (type == &_arg_types::Axis) {
            auto* w = this->subwidgets.combobox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->addItem("X", _char_value_t('X'));
            w->addItem("Y", _char_value_t('Y'));
            w->addItem("Z", _char_value_t('Z'));
            this->_setCurrentWidget(w);
            //
            auto i = w->findData((uint8_t)param.dword);
            if (i < 0)
               i = 0;
            w->setCurrentIndex(i);
         } else {
            auto* w = this->subwidgets.textbox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->setMaxLength(1);
            this->_setCurrentWidget(w);
            //
            w->setText(QChar(param.dword & 0xFF));
         }
         break;
      case dovah::condition_parameter_underlying_type::float32:
      case dovah::condition_parameter_underlying_type::int_signed:
      case dovah::condition_parameter_underlying_type::int_unsigned:
         {
            auto* w = this->subwidgets.spinbox;
            const auto blocker = QSignalBlocker(w);
            w->setValue(0.0F);
            if (param.underlying == dovah::condition_parameter_underlying_type::float32) {
               w->setDecimals(_max_decimals_for_float);
               w->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
               //
               w->setValue(param.float32);
            } else {
               w->setDecimals(0);
               if (param.underlying == dovah::condition_parameter_underlying_type::int_signed) {
                  w->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
                  w->setValue(param.integer);
               } else {
                  w->setRange(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
                  w->setValue(param.dword);
               }
            }
            this->_setCurrentWidget(w);
         }
         break;
      case dovah::condition_parameter_underlying_type::formID:
         if (!type) {
            this->_setCurrentWidget(this->subwidgets.blank);
            break;
         } else {
            bool references = false;
            for (auto ft : type->allowed_form_types) {
               if (dovah::form_type_info::form_type_is_reference(ft)) {
                  references = true;
                  break;
               }
            }
            if (references) {
               auto* w = this->subwidgets.ref;
               const auto blocker = QSignalBlocker(w);
               this->_setCurrentWidget(w);
               //
               w->setValue(param.form);
               break;
            } else {
               auto* w = this->subwidgets.form;
               const auto blocker = QSignalBlocker(w);
               this->_setCurrentWidget(w);
               //
               QVector<uint8_t> al;
               if (auto s = type->allowed_form_types.size()) {
                  al.reserve(s);
                  for (auto i : type->allowed_form_types)
                     al.push_back(i);
               }
               w->setAllowedFormTypes(al);
               w->setAllowNone(true);
               w->populate();
               //
               dovah::bare_form_id_t id = 0;
               if (auto* stub = param.form)
                  id = stub->formID;
               w->setFormByID(id);
            }
         }
         break;
      case dovah::condition_parameter_underlying_type::package_data:
         this->_setCurrentWidget(this->subwidgets.combobox);
         {
            auto* w = this->subwidgets.combobox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->addItem(tr("NONE"), -1);
            if (auto* p = this->context.get_owning_package()) {
               //
               // TODO: package data items
               //
               // TODO: Can we set up a custom proxy model to sort these?
               //
            }
            auto i = w->findData(param.dword);
            if (i < 0)
               i = 0;
            w->setCurrentIndex(i);
         }
         break;
      case dovah::condition_parameter_underlying_type::quest_stage:
         if (this->parameter_index > 0) {
            auto& prev = this->_get_previous_parameter();
            if (prev.underlying == dovah::condition_parameter_underlying_type::formID && prev.form && prev.form->formType == dovah::form_type::quest) {
               using loaded_form_t = dovah::loaded_forms::Quest;
               //
               dovah::loaded_form_ptr<loaded_form_t> keep_alive;
               loaded_form_t* q = prev.form->get_working_or_stable_copy(keep_alive);
               //
               if (q) {
                  auto* w = this->subwidgets.combobox;
                  const auto blocker = QSignalBlocker(w);
                  w->clear();
                  //
                  for (auto& s : q->stages)
                     w->addItem(QString::number(s.index), s.index);
                  //
                  this->_setCurrentWidget(w);
                  w->setCurrentIndex(w->findData(param.dword));
                  break;
               }
            }
         }
         {
            auto* w = this->subwidgets.spinbox;
            const auto blocker = QSignalBlocker(w);
            w->setDecimals(0);
            w->setRange(0, 65535);
            w->setValue(param.dword);
            this->_setCurrentWidget(w);
         }
         break;
      case dovah::condition_parameter_underlying_type::string:
         {
            auto* w = this->subwidgets.textbox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->setMaxLength(32767); // Qt default
            w->setText(param.string.c_str());
            this->_setCurrentWidget(w);
         }
         break;
   }
}
void ConditionParameterEditor::_rebuildForEvents() {
   constexpr int i_event_function = 0;
   constexpr int i_event_member   = 1;
   constexpr int i_event_form     = 2;
   //
   switch (this->parameter_index) {
      case 0: // event function
         this->_setCurrentWidget(this->subwidgets.combobox);
         {
            auto* w = this->subwidgets.combobox;
            w->clear();
            w->addItem("GetIsID",      dovah::condition_event_function::GetIsID);
            w->addItem("GetItemValue", dovah::condition_event_function::GetItemValue);
            w->addItem("GetValue",     dovah::condition_event_function::GetValue);
            w->addItem("HasKeyword",   dovah::condition_event_function::HasKeyword);
            w->addItem("IsInList",     dovah::condition_event_function::IsInList);
            //
            w->setCurrentIndex(w->findData(this->working.event_parameters.function));
         }
         break;
      case 1: // event member
         this->_setCurrentWidget(this->subwidgets.combobox);
         {
            auto* w = this->subwidgets.combobox;
            w->clear();
            //
            bool member_must_be_form = false; // NOTE: this is not a limitation that the CK enforces
            bool member_cant_be_refr = true;  // NOTE: the CK always enforces this limitation, for all functions
            switch (this->working.event_parameters.function) {
               case dovah::condition_event_function::GetIsID:
               case dovah::condition_event_function::GetItemValue:
               case dovah::condition_event_function::HasKeyword:
               case dovah::condition_event_function::IsInList:
                  member_must_be_form = true;
                  break;
            }
            //
            if (auto* q = this->context.get_owning_quest()) {
               if (auto* e = dovah::story_event_definition::lookup(q->event)) {
                  for (auto& m : e->members) {
                     if (member_cant_be_refr && m.can_only_be_reference())
                        continue;
                     if (member_must_be_form && !m.is_form())
                        continue;
                     w->addItem(m.name, m.signature);
                  }
               }
            }
            //
            w->setCurrentIndex(w->findData(this->working.event_parameters.member));
         }
         break;
      case 2: // event form
         {
            QVector<dovah::form_type_t> allowed;
            switch (this->working.event_parameters.function) {
               case dovah::condition_event_function::GetIsID:
                  {
                     bool known       = false;
                     bool use_default = false;
                     if (auto* q = this->context.get_owning_quest()) {
                        if (auto* e = dovah::story_event_definition::lookup(q->event)) {
                           if (auto* m = e->member_by_signature(this->working.event_parameters.member)) {
                              known = true;
                              //
                              auto& list = m->allowed_form_types;
                              if (!list.empty()) {
                                 if (list.size() == 1 && list[0] == dovah::form_type::none) {
                                    use_default = true;
                                 } else {
                                    for (auto ft : list) {
                                       if (!dovah::form_type_info::form_type_is_reference(ft))
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
               case dovah::condition_event_function::HasKeyword:
                  allowed.push_back(dovah::form_type::keyword);
                  break;
               case dovah::condition_event_function::IsInList:
                  allowed.push_back(dovah::form_type::formlist);
                  break;
            }
            if (allowed.isEmpty()) {
               this->_setCurrentWidget(this->subwidgets.blank);
               break;
            } else {
               this->_setCurrentWidget(this->subwidgets.form);
               auto* w = this->subwidgets.form;
               w->setAllowedFormTypes(allowed);
               w->populate();
               //
               dovah::bare_form_id_t id = 0;
               if (auto* stub = this->working.event_parameters.form)
                  id = stub->formID;
               w->setFormByID(id);
            }
         }
         break;
   }
}