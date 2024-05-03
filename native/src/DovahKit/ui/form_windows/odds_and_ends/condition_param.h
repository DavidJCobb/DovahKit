#pragma once
#include <array>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QStackedWidget>
#include <QWidget>
#include "dovah/data/conditions.h"
#include "dovah/forms/components/conditions.h"
#include "widgets/DKFormPicker.h"
#include "widgets/DKCompactObjectReferencePicker.h"

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
   public:
      ConditionParameterEditor(dovah::form_stub& containing_form, dovah::loaded_forms::components::working_condition& wc, int index, QWidget* parent = Q_NULLPTR);
      //
      QWidget* currentSubwidget() const noexcept;
      //
   signals:
      void valueChanged();
      //
   public slots:
      void clear();
      void rebuild();
      //
   protected:
      dovah::loaded_forms::components::condition_context  context;
      dovah::loaded_forms::components::working_condition& working;
      const int parameter_index = 0;
      //
      QStackedWidget* stack = nullptr;
      struct {
         QWidget*             blank    = nullptr;
         QComboBox*           combobox = nullptr;
         QLineEdit*           textbox  = nullptr;
         QDoubleSpinBox*      spinbox  = nullptr;
         DKFormPicker*        form     = nullptr;
         DKCompactObjectReferencePicker* ref = nullptr;
      } subwidgets;

      std::array<QSignalBlocker, 6> _getSubwidgetsBlocker();
      
      bool _is_event_parameter() const noexcept;
      dovah::loaded_forms::components::condition_parameter& _get_parameter() const noexcept;
      const dovah::condition_parameter_type* _get_parameter_type() const noexcept;
      const dovah::condition_function* _get_condition_function() const noexcept;
      dovah::loaded_forms::components::condition_parameter& _get_previous_parameter() const noexcept;

      void _setCurrentWidget(QWidget*);
      
      void _rebuildForEvents();
};