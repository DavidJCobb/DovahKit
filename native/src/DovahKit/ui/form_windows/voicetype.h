#pragma once
#include <cstdint>
#include <QDialog>
#include "../../dovah/form_stub.h"
#include "../../dovah/forms/Voicetype.h"
#include "ui_voicetype.h"

class FormDialogVoicetype : public QDialog {
   Q_OBJECT
   //
   public:
      FormDialogVoicetype(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
      void load();
      void save();
      //
   private slots:
      //
   private:
      Ui::FormDialogVoicetype ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Voicetype> form;
};
