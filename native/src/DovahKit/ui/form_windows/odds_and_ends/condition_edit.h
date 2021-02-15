#pragma once
#include <QSortFilterProxyModel>
#include <QTimer>
#include "ui_condition_edit.h"
#include "../../../dovah/forms/components/conditions.h"
#include "condition_param.h"

class ConditionEditDialog : public QDialog {
   Q_OBJECT
   protected:
      using condition_t   = dovah::loaded_forms::components::condition;
      using working_condition = dovah::loaded_forms::components::working_condition;
      using cnd_context_t = dovah::loaded_forms::components::condition_context;
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
      ConditionEditDialog(dovah::form_stub& containing_form, condition_t& condition, QWidget* parent = Q_NULLPTR);
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
         QWidget* holder = nullptr;
         ConditionParameterEditor* widget = nullptr;
      };
      std::array<_parameter, 3> parameters;
      bool did_param_holder_layout = false;
      condition_t::run_on_type last_run_on = condition_t::run_on_type::subject;

      void _updateRunOn(bool use_original = false);
      void _save();

      virtual void showEvent(QShowEvent* event) override;
};
