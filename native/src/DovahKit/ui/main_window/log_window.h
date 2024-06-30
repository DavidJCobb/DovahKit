#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_log_window.h"

class LogListModel;

class LogWindow : public QWidget {
   Q_OBJECT;
   public:
      LogWindow(QWidget* parent);
      
   protected:
      Ui::LogWindow ui;
      LogListModel* _model = nullptr;

      void _redraw_selected_entry();
};
