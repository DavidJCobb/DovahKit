#pragma once
#include <cstdint>
#include <QDialog>
#include "../../dovah/form_stub.h"
#include "../../dovah/forms/Color.h"
#include "ui_color.h"

class FormDialogColor : public QDialog {
   Q_OBJECT
   //
   public:
      FormDialogColor(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
      void load();
      void save();
      //
   private slots:
      //
   private:
      Ui::FormDialogColor ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Color> form;
};
