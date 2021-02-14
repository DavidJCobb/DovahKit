#pragma once
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QStackedWidget>
#include <QWidget>
#include "../../generic/FormsOfTypeCombobox.h"
#include "../../generic/RefPickerButton.h"
#include "../../../dovah/forms/components/conditions.h"

class ConditionParameterEditor : public QWidget {
   Q_OBJECT
   //
   // TODO:
   //
   //  - event parameter support (currently VERY partial; state members exist and save() supports it; nothing else does)
   //
   //  - changing the current displayed type should emit valueChanged after we update our controls
   //
   //  - if we have a union type and the previous condition arg is altered, when changing what the union resolves to 
   //    we should emit valueChanged after we update our controls
   //
   protected:
      using form_stub     = dovah::form_stub;
      using condition_t   = dovah::loaded_forms::components::condition;
      using cnd_context_t = dovah::loaded_forms::components::condition_context;
      //
      using underlying_t  = dovah::loaded_forms::components::condition_info::arg_underlying_type;
      using param_type_t  = dovah::loaded_forms::components::condition_info::arg_type;
      using param_value_t = dovah::loaded_forms::components::condition_arg_value;
      //
   public:
      ConditionParameterEditor(form_stub& containing_form, condition_t& condition, int index, QWidget* parent = Q_NULLPTR);
      //
      void overrideUnderlyingType(underlying_t);
      void setEventParameter(int which);
      void setPrevious(ConditionParameterEditor&);
      void setType(param_type_t*, underlying_t);
      //
      inline param_type_t* argType() const noexcept { return this->parameter.type; }
      QVariant currentData() const noexcept;
      QWidget* currentSubwidget() const noexcept;
      underlying_t underlyingType() const noexcept;
      QVariant value() const noexcept;
      param_value_t valueRaw() const noexcept;
      //
   signals:
      void valueChanged(const QVariant);
      //
   public slots:
      void save();
      //
   protected:
      condition_t&  condition;
      cnd_context_t context;
      //
      QStackedWidget* stack = nullptr;
      struct {
         QWidget*             blank    = nullptr;
         QComboBox*           combobox = nullptr;
         QLineEdit*           textbox  = nullptr;
         QDoubleSpinBox*      spinbox  = nullptr;
         FormsOfTypeCombobox* form     = nullptr;
         RefPickerButton*     ref      = nullptr;
      } subwidgets;
      struct {
         int  index    = -1;
         bool is_event = false;
         ConditionParameterEditor* previous = nullptr;
         underlying_t  underlying = underlying_t::none;
         underlying_t  u_override = underlying_t::none;
         param_type_t* type       = nullptr;
         param_type_t* resolved   = nullptr; // if (type) is a union type
      } parameter;
      //
      void _updateWidgetState(bool pull_from_original = false);
};