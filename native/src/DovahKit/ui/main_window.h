#pragma once
#include <cstdint>
#include <QtWidgets/QMainWindow>
#include <QtWinExtras/qwintaskbarbutton.h> // this probably isn't the right way to include this, but Visual Studio and Qt Tools are not being cooperative.
#include "ui_main_window.h"
#include "main_window/object_window.h"

class FileMetadataWindow;

class MainWindow : public QMainWindow {
   Q_OBJECT
   //
   public:
      MainWindow(QWidget* parent = Q_NULLPTR);
      //
      static MainWindow& get(); // done differently because the usual "static singleton getter" approach apparently causes Qt to crash on exit if applied to the main window
      //
   public slots:
      void setProgressBounds(int, int);
      void setProgressStep(int);
      void setProgressEnableState(bool);
      //
   signals:
      void shown();
      //
   private:
      Ui::MainWindow ui;
      QWinTaskbarButton*  taskbar_button  = nullptr;
      ObjectWindow*       object_window   = nullptr;
      FileMetadataWindow* metadata_window = nullptr;
      //
   protected:
      virtual void closeEvent(QCloseEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;
};
