#pragma once
#include "./_base.h"
#include "dovah/forms/Scroll.h"
#include "ui_scroll.h" // generated

class FormDialogScroll :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Scroll, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogScroll(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogScroll ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      QString _make_concatenated_description() const;
      void _update_auto_calc();
      void _update_effect_parameters_enable_states();
};
