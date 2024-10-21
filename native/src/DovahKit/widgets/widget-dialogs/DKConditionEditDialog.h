#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <array>
#include <QSortFilterProxyModel>
#include <QTimer>
#include "ui_DKConditionEditDialog.h" // generated
#include "dovah/data/conditions/parameter_underlying_type.h"
#include "dovah/forms/components/conditions.h"
#include "ui/types/conditions/condition.h"

namespace dovah::conditions {
   struct function_info;
   struct parameter_typeinfo;
}

class DKConditionEditDialog : public QDialog {
   Q_OBJECT;
   public:
      using Condition = ui::types::conditions::condition;

   protected:
      class _FunctionListProxy : public QSortFilterProxyModel {
         public:
            static constexpr int ExcludeRole = Qt::ItemDataRole::UserRole + 1;
            _FunctionListProxy(QObject* o) : QSortFilterProxyModel(o) {}
         protected:
            virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
      };
      
      struct _ParameterWidget {
         QWidget*        parent   = nullptr;
         QStackedWidget* stack    = nullptr;
         //              //
         QWidget*        blank    = nullptr;
         QComboBox*      combobox = nullptr;
         QLineEdit*      textbox  = nullptr;
         QDoubleSpinBox* spinbox  = nullptr;
         DKFormPicker*   form     = nullptr;
         DKCompactObjectReferencePicker* ref = nullptr;

         dovah::conditions::parameter_underlying_type last_shown_type     = dovah::conditions::parameter_underlying_type::none;
         const dovah::conditions::parameter_typeinfo* last_shown_typeinfo = nullptr;
      };
      
   public:
      DKConditionEditDialog(dovah::form_stub& containing_form, const Condition& condition, QWidget* parent = nullptr);

      constexpr const Condition& value() const noexcept { return this->_value; }
      
   private slots:
      void forceUpdateParameters();
      
   protected:
      Ui::DKConditionEditDialog ui;

      ui::types::conditions::context _context;
      Condition _value;
      
      QTimer function_filter_update_throttle;
      
      std::array<_ParameterWidget, 3> parameters;
      bool did_param_holder_layout = false;
      ui::types::conditions::run_on_type last_run_on = ui::types::conditions::run_on_type::subject;

      void _on_form_deleted(const dovah::form_stub&);
      void _on_form_modified(const dovah::form_stub&);

      void _update_comparison_operand_ui();
      void _update_run_on_ui();

      void _on_parameter_changed(size_t index, QVariant);

      void _renew_combobox_edit_handler(size_t index);
      bool _update_parameter_ui_for_special_case(size_t index); // returns `true` if special case applied
      void _update_parameter_ui(size_t index);
      void _update_all_parameters_ui();

      void _quest_stages_to_combobox(dovah::form_stub& quest, QComboBox*, bool maintain_selection);

      virtual void showEvent(QShowEvent* event) override;
};
