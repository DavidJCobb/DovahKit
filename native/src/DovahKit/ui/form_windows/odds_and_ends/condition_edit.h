#pragma once
#include "ui_condition_edit.h"
#include "../../../dovah/forms/components/conditions.h"

class ConditionEditDialog : public QDialog {
   Q_OBJECT
   protected:
      using condition_t   = dovah::loaded_forms::components::condition;
      using cnd_context_t = dovah::loaded_forms::components::condition_context;
      using loaded_form_t = dovah::loaded_forms::Form;
      using underlying_t  = dovah::loaded_forms::components::condition_info::arg_underlying_type;
      using param_type_t  = dovah::loaded_forms::components::condition_info::arg_type;
      using param_value_t = dovah::loaded_forms::components::condition_arg_value;
   public:
      ConditionEditDialog(loaded_form_t& containing_form, condition_t& condition, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::ConditionEditDialog ui;
      condition_t&  condition;
      cnd_context_t context;
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
      condition_t::run_on_t last_run_on = condition_t::run_on_t::subject;
      
      void _buildParamControls(int which, underlying_t, param_type_t*, bool use_original = false);
      void _updateRunOn(bool use_original = false);
      void _save();

      virtual void showEvent(QShowEvent* event) override;
};
