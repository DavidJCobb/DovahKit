#pragma once
#include <array>
#include "./_base.h"
#include "dovah/forms/Class.h"
#include "ui_class.h" // generated

#include "dovah/data/skills.h"

class FormDialogClass :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Class, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogClass(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogClass ui;
      std::array<QSpinBox*, dovah::skill_count> ui_skill_weights = { 0 };
      std::array<QSpinBox*, 3> ui_attr_weights = { 0 };
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
