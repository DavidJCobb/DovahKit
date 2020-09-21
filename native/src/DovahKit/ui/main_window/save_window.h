#pragma once
#include <cstdint>
#include <QDialog>
#include <QTimer>
#include "ui_save_window.h"

class ActiveFileSaveDialog : public QDialog {
   Q_OBJECT
   //
   public:
      ActiveFileSaveDialog(QWidget* parent = Q_NULLPTR); // needs to be public for Qt? but do not call; use the static getter
      //
   private:
      Ui::ActiveFileSaveDialog ui;

      void commit();
      void reportLastSaveError();
};
