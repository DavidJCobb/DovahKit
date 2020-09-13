#pragma once
#include <cstdint>
#include <QtWidgets/QMainWindow>
#include "ui_main_window.h"
#include "main_window/object_window.h"

class MainWindow : public QMainWindow {
   Q_OBJECT
   //
   public:
      MainWindow(QWidget* parent = Q_NULLPTR); // needs to be public for Qt? but do not call; use the static getter
      //
      static MainWindow& get(); // done differently because the usual "static singleton getter" approach apparently causes Qt to crash on exit if applied to the main window
      //
   private slots:
      //
   private:
      Ui::MainWindow ui;
      ObjectWindow* object_window = nullptr;
};
