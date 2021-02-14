#include "condition_param.h"
#include <QVariant>
#include "../../../dovah/data/story_manager.h"
#include "../../../dovah/forms/Quest.h"

namespace {
   using _char_value_t = uint8_t;
   constexpr int _max_decimals_for_float = FLT_MAX_10_EXP + FLT_DIG; // per Qt documentation for QDoubleSpinBox

   namespace _arg_types {
      using namespace dovah::loaded_forms::components::condition_info::arg_types;
   }
}

ConditionParameterEditor::ConditionParameterEditor(form_stub& containing_form, condition_t& condition, int index, QWidget* parent) : QWidget(parent), condition(condition), context(containing_form) {
   this->parameter.index = index;
   //
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
      emit this->valueChanged(this->value());
   });
   QObject::connect(this->subwidgets.textbox, &QLineEdit::textChanged, this, [this]() {
      emit this->valueChanged(this->value());
   });
   QObject::connect(this->subwidgets.spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
      emit this->valueChanged(this->value());
   });
   QObject::connect(this->subwidgets.form, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      emit this->valueChanged(this->value());
   });
   QObject::connect(this->subwidgets.ref, &RefPickerButton::valueChanged, this, [this]() {
      emit this->valueChanged(this->value());
   });
}

void ConditionParameterEditor::overrideUnderlyingType(underlying_t uo) {
   if (this->parameter.u_override == uo)
      return;
   this->parameter.u_override = uo;
   if (auto* t = this->parameter.type) {
      if (!t->can_be_alias)
         return;
      this->_updateWidgetState();
   }
}
void ConditionParameterEditor::setEventParameter(int which) {
   this->parameter.is_event = true;
   this->parameter.index    = which;
   //
   // TODO: REDRAW/UPDATE
   //
}
void ConditionParameterEditor::setPrevious(ConditionParameterEditor& p) {
   if (this->parameter.previous)
      QObject::disconnect(this->parameter.previous, nullptr, this, nullptr);
   this->parameter.previous = &p;
   QObject::connect(&p, &ConditionParameterEditor::valueChanged, this, [this](const QVariant v) {
      if (this->parameter.underlying == underlying_t::quest_stage) {
         this->_updateWidgetState();
         return;
      }
      //
      auto* pt = this->parameter.previous->argType();
      auto  pv = this->parameter.previous->valueRaw();
      auto* t  = this->parameter.type;
      if (!t || !t->isUnion)
         return;
      auto* resolved_type = t->resolve_union(pt, &pv);
      this->parameter.resolved = resolved_type;
      if (resolved_type) {
         this->parameter.underlying = resolved_type->underlying;
      } else {
         this->parameter.underlying = underlying_t::none;
      }
      this->_updateWidgetState();
   });
}
void ConditionParameterEditor::setType(param_type_t* type, underlying_t under) {
   if (this->parameter.type == type)
      if (this->parameter.underlying == under)
         return;
   this->parameter.type       = type;
   this->parameter.underlying = under;
   this->_updateWidgetState();
}

QVariant ConditionParameterEditor::currentData() const noexcept {
   auto* csw = this->currentSubwidget();
   if (csw == this->subwidgets.combobox)
      return this->subwidgets.combobox->currentData();
   if (csw == this->subwidgets.spinbox) {
      auto* sb = this->subwidgets.spinbox;
      if (sb->decimals() == 0) {
         if (sb->minimum() >= 0.0F)
            return QVariant::fromValue<uint32_t>(sb->value());
         return QVariant::fromValue<int32_t>(sb->value());
      }
      return sb->value();
   }
   if (csw == this->subwidgets.textbox) {
      auto* widget  = this->subwidgets.textbox;
      bool  is_char = widget->maxLength() == 1;
      auto  text    = widget->text();
      if (is_char) {
         if (text.isEmpty())
            return _char_value_t(0);
         return _char_value_t(text[0].toLatin1());
      }
      return this->subwidgets.textbox->text();
   }
   if (csw == this->subwidgets.form)
      return QVariant::fromValue<void*>(this->subwidgets.form->formStub());
   if (csw == this->subwidgets.ref)
      return QVariant::fromValue<void*>(this->subwidgets.ref->value());
   return QVariant();
}
QWidget* ConditionParameterEditor::currentSubwidget() const noexcept {
   return this->stack->currentWidget();
}
ConditionParameterEditor::underlying_t ConditionParameterEditor::underlyingType() const noexcept {
   auto* type = this->parameter.resolved;
   if (!type)
      type = this->parameter.type;
   if (this->parameter.u_override != underlying_t::none && (!type || type->can_be_alias))
      return this->parameter.u_override;
   return this->parameter.underlying;
}
QVariant ConditionParameterEditor::value() const noexcept {
   switch (this->underlyingType()) {
      case underlying_t::aliasID:
         [[fallthrough]];
      case underlying_t::package_data:
         return this->subwidgets.combobox->currentData();
         //
      case underlying_t::character:
         if (this->currentSubwidget() == this->subwidgets.combobox)
            return this->subwidgets.combobox->currentData();
         {
            auto t = this->subwidgets.textbox->text();
            if (t.isEmpty())
               return '\0';
            return t[0].toLatin1();
         }
         //
      case underlying_t::event:
         return this->currentData();
         //
      case underlying_t::float32:
         return this->subwidgets.spinbox->value();
         //
      case underlying_t::formID:
         {
            auto v = this->currentData();
            if (v.type() != QMetaType::VoidStar)
               break;
            return v;
         }
         //
      case underlying_t::int_signed:
         return QVariant::fromValue<int32_t>(this->subwidgets.spinbox->value());
         //
      case underlying_t::int_unsigned:
         return QVariant::fromValue<uint32_t>(this->subwidgets.spinbox->value());
         //
      case underlying_t::none:
         break;
         //
      case underlying_t::quest_stage:
         if (this->currentSubwidget() == this->subwidgets.combobox)
            return this->subwidgets.combobox->currentData();
         return (int)this->subwidgets.spinbox->value();
         //
      case underlying_t::string:
         return this->subwidgets.textbox->text();
   }
   return QVariant();
}
ConditionParameterEditor::param_value_t ConditionParameterEditor::valueRaw() const noexcept {
   param_value_t out;
   switch (this->underlyingType()) {
      case underlying_t::aliasID:
      case underlying_t::package_data:
         out.dword = this->subwidgets.combobox->currentData().toInt();
         break;
      case underlying_t::character:
      case underlying_t::quest_stage:
         out.dword = this->value().toInt();
         break;
      case underlying_t::float32:
         out.float32 = this->subwidgets.spinbox->value();
         break;
      case underlying_t::int_signed:
         out.dword = (int32_t)this->subwidgets.spinbox->value();
         break;
      case underlying_t::int_unsigned:
         out.dword = (uint32_t)this->subwidgets.spinbox->value();
         break;
      case underlying_t::formID:
         if (this->currentSubwidget() == this->subwidgets.ref)
            out.form.unmanaged_set(this->subwidgets.ref->value());
         else
            out.form.unmanaged_set(this->subwidgets.form->formStub());
         break;
      case underlying_t::string:
         out.string = this->subwidgets.textbox->text().toStdString();
         break;
   }
   return out;
}

void ConditionParameterEditor::save() {
   auto* wc = this->context.owner->get_working_copy();
   assert(wc && "ConditionParameterEditor should always be operating on a form that has a working copy ready.");
   //
   if (this->parameter.is_event) {
      switch (this->parameter.index) {
         case 0:
            this->condition.event_parameters.function = this->subwidgets.combobox->currentData().toInt();
            break;
         case 1:
            this->condition.event_parameters.member = this->subwidgets.combobox->currentData().toInt();
            break;
         case 2:
            {
               dovah::form_stub* form = nullptr;
               QVariant v = this->value();
               if (v.type() == QMetaType::VoidStar)
                  form = (dovah::form_stub*)v.value<void*>();
               //
               if (wc)
                  this->condition.event_parameters.form.set(*wc, form);
            }
            break;
      }
      return;
   }
   //
   param_value_t* value = nullptr;
   {
      auto i = this->parameter.index;
      if (i >= 0 && i < this->condition.parameters.size())
         value = &this->condition.parameters[i];
      else
         return;
   }
   //
   if (this->underlyingType() == underlying_t::formID) {
      value->string.clear();
      if (wc)
         value->form.set(*wc, this->valueRaw().form);
   } else {
      value->form.set(*wc, nullptr); // properly sever use info
      *value = this->valueRaw();
   }
}

void ConditionParameterEditor::_updateWidgetState(bool pull_from_original) {
   param_value_t* value = nullptr;
   if (pull_from_original) {
      auto i = this->parameter.index;
      if (i >= 0 && i < this->condition.parameters.size())
         value = &this->condition.parameters[i];
      else
         pull_from_original = false;
   }
   //
   auto under = this->underlyingType();
   switch (under) {
      case underlying_t::none:
         this->stack->setCurrentWidget(this->subwidgets.blank);
         break;
      case underlying_t::aliasID:
         this->stack->setCurrentWidget(this->subwidgets.combobox);
         {
            auto* w = this->subwidgets.combobox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            if (auto* q = this->context.get_owning_quest()) {
               w->addItem(tr("NONE"), -1);
               //
               // TODO: Can we set up a custom proxy model to sort these?
               //
               for (auto* alias : q->aliases) {
                  if (alias->type != dovah::loaded_forms::Alias::alias_type::reference)
                     continue;
                  w->addItem(alias->name.c_str(), alias->id);
               }
            }
            if (pull_from_original) {
               w->setCurrentIndex(w->findData(value->dword));
            } else {
               w->setCurrentIndex(0);
            }
         }
         break;
      case underlying_t::character:
         if (this->parameter.type == &_arg_types::Axis) {
            auto* w = this->subwidgets.combobox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->addItem("X", _char_value_t('X'));
            w->addItem("Y", _char_value_t('Y'));
            w->addItem("Z", _char_value_t('Z'));
            this->stack->setCurrentWidget(w);
            //
            if (pull_from_original)
               w->setCurrentIndex(w->findData((uint8_t)value->dword));
         } else {
            auto* w = this->subwidgets.textbox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->setMaxLength(1);
            this->stack->setCurrentWidget(w);
            //
            if (pull_from_original)
               w->setText(QChar(value->dword & 0xFF));
         }
         break;
      case underlying_t::float32:
      case underlying_t::int_signed:
      case underlying_t::int_unsigned:
         {
            auto* w = this->subwidgets.spinbox;
            const auto blocker = QSignalBlocker(w);
            w->setValue(0.0F);
            //
            if (under == underlying_t::float32) {
               w->setDecimals(_max_decimals_for_float);
               w->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
               //
               if (pull_from_original)
                  w->setValue(value->float32);
            } else {
               w->setDecimals(0);
               if (under == underlying_t::int_signed) {
                  w->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
               } else {
                  w->setRange(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
               }
               //
               if (pull_from_original)
                  w->setValue(value->dword);
            }
            //
            this->stack->setCurrentWidget(w);
         }
         break;
      case underlying_t::formID:
         if (auto* t = this->parameter.type) {
            bool references = false;
            for (auto ft : t->allowedFormTypes) {
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
               if (pull_from_original)
                  w->setValue(value->form.get_form_stub());
               else
                  w->setValue(nullptr);
               //
               break;
            }
            auto* w = this->subwidgets.form;
            const auto blocker = QSignalBlocker(w);
            this->stack->setCurrentWidget(w);
            //
            QVector<uint8_t> al;
            if (t->allowedFormTypes.size()) {
               al.reserve(t->allowedFormTypes.size());
               for (auto i : t->allowedFormTypes)
                  al.push_back(i);
            }
            w->setAllowedFormTypes(al);
            w->setAllowNone(true);
            w->populate();
            //
            dovah::bare_form_id_t id = 0;
            if (pull_from_original) {
               if (auto* stub = value->form.get_form_stub())
                  id = stub->formID;
            }
            w->setFormByID(id);
         }
         break;
      case underlying_t::package_data:
         this->stack->setCurrentWidget(this->subwidgets.combobox);
         {
            auto* w = this->subwidgets.combobox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            if (auto* q = this->context.get_owning_package()) {
               w->addItem(tr("NONE"), -1);
               //
               // TODO: package data items
               //
               // TODO: Can we set up a custom proxy model to sort these?
               //
            }
            if (pull_from_original) {
               w->setCurrentIndex(w->findData(value->dword));
            } else {
               w->setCurrentIndex(0);
            }
         }
         break;
      case underlying_t::quest_stage:
         {
            if (auto* prev = this->parameter.previous) {
               auto pv = prev->value();
               if (pv.type() == QMetaType::VoidStar) { // form
                  auto* quest = (dovah::form_stub*) pv.value<void*>();
                  if (quest && quest->formType == dovah::form_type::quest) {
                     auto* w = this->subwidgets.combobox;
                     const auto blocker = QSignalBlocker(w);
                     w->clear();
                     //
                     using loaded_form_t = dovah::loaded_forms::Quest;
                     //
                     dovah::loaded_form_ptr<loaded_form_t> keep_alive;
                     loaded_form_t* q = quest->get_working_or_stable_copy(keep_alive);
                     //
                     if (q)
                        for (auto& s : q->stages)
                           w->addItem(QString::number(s.index), s.index);
                     //
                     this->stack->setCurrentWidget(w);
                     if (pull_from_original)
                        w->setCurrentIndex(w->findData(value->dword));
                     //
                     break;
                  }
               }
            }
            auto* w = this->subwidgets.spinbox;
            const auto blocker = QSignalBlocker(w);
            w->setDecimals(0);
            w->setRange(0, 65535);
            this->stack->setCurrentWidget(w);
            //
            if (pull_from_original)
               w->setValue(value->dword);
            else
               w->setValue(0.0F);
         }
         break;
      case underlying_t::string:
         {
            auto* w = this->subwidgets.textbox;
            const auto blocker = QSignalBlocker(w);
            w->clear();
            w->setMaxLength(32767); // Qt default
            //
            if (pull_from_original)
               w->setText(value->string.c_str());
         }
         break;
   }
   //
   // TODO: Under what circumstances should we emit valueChanged here?
   //
}