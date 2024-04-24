#pragma once
#include <QDialog>
#include "ui_save_window.h"

class ActiveFileSaveDialog : public QDialog {
   Q_OBJECT
   //
   public:
      ActiveFileSaveDialog(QWidget* parent = Q_NULLPTR);
      //
   private:
      Ui::ActiveFileSaveDialog ui;

      void commit();
};
