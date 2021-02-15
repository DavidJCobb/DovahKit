#include "condition_param.h"
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

dovah::loaded_forms::components::condition_parameter& ConditionParameterEditor::_get_parameter() const noexcept {
   return this->working.parameters[this->parameter_index];
}
const dovah::condition_parameter_type* ConditionParameterEditor::_get_parameter_type() const noexcept {
   return this->working.get_argument_type(this->parameter_index);
}
dovah::loaded_forms::components::condition_parameter& ConditionParameterEditor::_get_previous_parameter() const noexcept {
   return this->working.parameters[this->parameter_index - 1];
}

ConditionParameterEditor::ConditionParameterEditor(dovah::form_stub& containing_form, dovah::loaded_forms::components::working_condition& condition, int index, QWidget* parent)
   :
   QWidget(parent),
   working(condition),
   context(containing_form),
   parameter_index(index)
{
   this->stack = new QStackedWidget(this);
   this->stack->addWidget(this->subwidgets.blank    = new QWidget);
   this->stack->addWidget(this->subwidgets.combobox = new QComboBox);
   this->stack->addWidget(this->subwidgets.textbox  = new QLineEdit);
   this->stack->addWidget(this->subwidgets.spinbox  = new QDoubleSpinBox);
   this->stack->addWidget(this->subwidgets.form     = new FormsOfTypeCombobox);
   this->stack->addWidget(this->subwidgets.ref      = new RefPickerButton);
   //
   this->subwidgets.spinbox->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
   this->subwidgets.form->setAllowNone(true);
   //
   QObject::connect(this->subwidgets.combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      auto& param = this->_get_parameter();
      switch (param.underlying) {
         case dovah::condition_parameter_underlying_type::aliasID:
         case dovah::condition_parameter_underlying_type::character:
         case dovah::condition_parameter_underlying_type::int_signed:
         case dovah::condition_parameter_underlying_type::package_data:
         case dovah::condition_parameter_underlying_type::quest_stage:
            param.integer = this->subwidgets.combobox->currentData().toInt();
            break;
         case dovah::condition_parameter_underlying_type::int_unsigned:
            param.dword = this->subwidgets.combobox->currentData().value<uint32_t>();
            break;
         case dovah::condition_parameter_underlying_type::float32:
            param.float32 = this->subwidgets.combobox->currentData().toFloat();
            break;
         default:
            return;
      }
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.textbox, &QLineEdit::textChanged, this, [this]() {
      this->_get_parameter().string = this->subwidgets.textbox->text().toStdString();
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
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
      this->_get_parameter().form = this->subwidgets.form->formStub();
      emit this->valueChanged();
   });
   QObject::connect(this->subwidgets.ref, &RefPickerButton::valueChanged, this, [this]() {
      this->_get_parameter().form = this->subwidgets.ref->value();
      emit this->valueChanged();
   });
}

QWidget* ConditionParameterEditor::currentSubwidget() const noexcept {
   return this->stack->currentWidget();
}

void ConditionParameterEditor::clear() {
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
   auto& param = this->_get_parameter();
   auto* type  = this->_get_parameter_type();
   switch (param.underlying) {
      case dovah::condition_parameter_underlying_type::none:
         this->stack->setCurrentWidget(this->subwidgets.blank);
         break;
      case dovah::condition_parameter_underlying_type::aliasID:
         this->stack->setCurrentWidget(this->subwidgets.combobox);
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
            this->stack->setCurrentWidget(w);
            //
            w->setCurrentIndex(w->findData((uint8_t)param.dword));
         } else {
            auto* w = this->subwidgets.textbox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->setMaxLength(1);
            this->stack->setCurrentWidget(w);
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
            this->stack->setCurrentWidget(w);
         }
         break;
      case dovah::condition_parameter_underlying_type::formID:
         if (!type) {
            this->stack->setCurrentWidget(this->subwidgets.blank);
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
               this->stack->setCurrentWidget(w);
               //
               w->setValue(param.form);
               break;
            } else {
               auto* w = this->subwidgets.form;
               const auto blocker = QSignalBlocker(w);
               this->stack->setCurrentWidget(w);
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
         this->stack->setCurrentWidget(this->subwidgets.combobox);
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
                  this->stack->setCurrentWidget(w);
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
            this->stack->setCurrentWidget(w);
         }
         break;
      case dovah::condition_parameter_underlying_type::string:
         {
            auto* w = this->subwidgets.textbox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->setMaxLength(32767); // Qt default
            w->setText(param.string.c_str());
            this->stack->setCurrentWidget(w);
         }
         break;
   }
}