#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_log_window.h"

namespace dovah {
   struct file_read_warning;
   class  file_write_error;
}

class LogWindow : public QWidget {
   Q_OBJECT
   //
   public:
      LogWindow(QWidget* parent);
      //
   public slots:
      void insertLogEntry(const QString&);
      //
   private:
      Ui::LogWindow ui;
};
