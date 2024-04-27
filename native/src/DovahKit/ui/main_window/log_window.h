#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_log_window.h"

class LogWindow : public QWidget {
   Q_OBJECT;
   public:
      LogWindow(QWidget* parent);
      
   private:
      Ui::LogWindow ui;
};
