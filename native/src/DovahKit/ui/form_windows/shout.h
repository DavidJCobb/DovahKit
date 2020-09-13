#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_shout.h"

class FormDialogShout : public QDialog {
   Q_OBJECT
   //
   public:
      FormDialogShout(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
      void load();
      void save();
      //
   private slots:
      //
   private:
      Ui::FormDialogShout ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Shout> form;
};
