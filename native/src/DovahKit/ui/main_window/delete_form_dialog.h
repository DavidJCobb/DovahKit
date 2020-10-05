#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_delete_form_dialog.h"

namespace dovah {
   class form_deletion_request;
}

class DeleteFormDialog : public QDialog {
   Q_OBJECT
   //
   public:
      DeleteFormDialog(QWidget* parent = Q_NULLPTR);
      //
      void updateFromDeletionRequest(const dovah::form_deletion_request&);
      //
   private:
      Ui::DeleteFormDialog ui;
};
