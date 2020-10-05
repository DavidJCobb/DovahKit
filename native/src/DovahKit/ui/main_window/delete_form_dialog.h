#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_delete_form_dialog.h"

class DeleteFormDialog : public QDialog {
   Q_OBJECT
   //
   public:
      DeleteFormDialog(QWidget* parent = Q_NULLPTR);
      //
   private:
      Ui::DeleteFormDialog ui;
};
