#pragma once
#include <cstdint>
#include <QDialog>
#include "../../dovah/form_stub.h"
#include "../../dovah/forms/WordOfPower.h"
#include "ui_word_of_power.h"

class FormDialogWordOfPower : public QDialog {
   Q_OBJECT
   //
   public:
      FormDialogWordOfPower(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
      void load();
      void save();
      //
   private slots:
      //
   private:
      Ui::FormDialogWordOfPower ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::WordOfPower> form;
};
