#pragma once
#include <cstdint>
#include <QDialog>
#include <QTimer>
#include "ui_load_window.h"

class LoadOrderOpenDialog : public QDialog {
   Q_OBJECT
   //
   public:
      LoadOrderOpenDialog(bool is_skyrim_classic, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      void loadPoll();
      //
   private:
      Ui::LoadOrderOpenDialog ui;
      bool   _loading = false;
      QTimer _load_poller;

      struct {
         QAction* check   = nullptr;
         QAction* uncheck = nullptr;
      } file_list_actions;

      void blockUI();
      void commit(); // load the selected files. this is async; listen for DovahKitCore's dataAcquireComplete and dataAcquireFailed
};
