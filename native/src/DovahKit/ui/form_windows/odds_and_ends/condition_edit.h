#pragma once
#include "ui_condition_edit.h"
#include "../../../dovah/forms/components/conditions.h"

class ConditionEditDialog : public QDialog {
   Q_OBJECT
   protected:
      using condition_t   = dovah::loaded_forms::components::condition;
      using loaded_form_t = dovah::loaded_forms::Form;
   public:
      ConditionEditDialog(loaded_form_t& containing_form, condition_t& condition, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::ConditionEditDialog ui;
      loaded_form_t& form;
      condition_t&   condition;
      //
      void _save();
};
