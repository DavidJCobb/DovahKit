#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_log_window.h"

namespace dovah {
   struct detailed_notice;
}

class LogWindow : public QWidget {
   Q_OBJECT
   //
   public:
      LogWindow(QWidget* parent);
      //
   private:
      Ui::LogWindow ui;
};
