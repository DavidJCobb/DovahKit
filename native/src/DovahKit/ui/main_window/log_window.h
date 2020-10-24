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
      void loadWarningReceived(const dovah::file_read_warning&);
      void saveErrorReceived(const dovah::file_write_error&);
      void clearLog();
      void insertLogEntry(const QString&);
      //
   private:
      Ui::LogWindow ui;
};
