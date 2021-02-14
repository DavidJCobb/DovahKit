#pragma once
#include <QSortFilterProxyModel>
#include <QTimer>
#include "ui_condition_edit.h"
#include "../../../dovah/forms/components/conditions.h"

class ConditionEditDialog : public QDialog {
   Q_OBJECT
   protected:
      using form_stub     = dovah::form_stub;
      using condition_t   = dovah::loaded_forms::components::condition;
      using working_condition = dovah::loaded_forms::components::working_condition;
      using cnd_context_t = dovah::loaded_forms::components::condition_context;
      using loaded_form_t = dovah::loaded_forms::Form;
      using underlying_t  = dovah::condition_parameter_underlying_type;
      using param_type_t  = dovah::condition_parameter_type;
      using param_value_t = dovah::loaded_forms::components::condition_parameter;
      //
      class _FunctionListProxy : public QSortFilterProxyModel {
         public:
            static constexpr int ExcludeRole = Qt::ItemDataRole::UserRole + 1;
            _FunctionListProxy(QObject* o) : QSortFilterProxyModel(o) {}
         protected:
            virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
      };
      //
   public:
      ConditionEditDialog(form_stub& containing_form, condition_t& condition, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::ConditionEditDialog ui;
      condition_t&      condition;
      cnd_context_t     context;
      working_condition working;
      //
      QTimer function_filter_update_throttle;
      //
      struct _parameter {
         enum class special_case_t {
            none,
            reference_pick_button,
         };
         //
         QWidget*       holder       = nullptr;
         QWidget*       widget       = nullptr;
         underlying_t   last_under   = underlying_t::none;
         param_type_t*  last_type    = nullptr;
         special_case_t last_special = special_case_t::none;
      };
      std::array<_parameter, 3> parameters;
      bool did_param_holder_layout = false;
      condition_t::run_on_type last_run_on = condition_t::run_on_type::subject;

      void _buildParamControls(int which, underlying_t, param_type_t*, bool use_original = false, bool force_update = false);
      void _updateRunOn(bool use_original = false);
      void _save();

      virtual void showEvent(QShowEvent* event) override;
};
