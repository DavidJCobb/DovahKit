#pragma once
#include <cstdint>
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/Color.h"
#include "ui_color.h"

class FormDialogColor :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Color, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogColor(dovah::form_stub& stub, QWidget* parent = Q_NULLPTR);
      
   private slots:
      
   protected:
      Ui::FormDialogColor ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
