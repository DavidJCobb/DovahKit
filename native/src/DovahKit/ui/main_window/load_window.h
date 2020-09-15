#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_load_window.h"

class LoadOrderOpenDialog : public QDialog {
   Q_OBJECT
   //
   public:
      LoadOrderOpenDialog(QWidget* parent = Q_NULLPTR); // needs to be public for Qt? but do not call; use the static getter
      //
   private slots:
      //
   private:
      Ui::LoadOrderOpenDialog ui;
      bool _loading = false;

      void blockUI();
      void commit(); // load the selected files. this is async; listen for DovahKitCore's dataAcquireComplete and dataAcquireFailed
};
