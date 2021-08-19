#pragma once
#include <QMainWindow>
#include "ui_script_window_single.h"

class EditorSingleScriptWindow : public QMainWindow {
   Q_OBJECT
   //
   public:
      EditorSingleScriptWindow(QWidget* parent = Q_NULLPTR);
      
   private:
      Ui::EditorSingleScriptWindow ui;
      
      void _onScriptStartStop(bool script_running);

      bool _checkAllowClose();

      virtual void closeEvent(QCloseEvent* event) override;
      //virtual void reject() override; // override needed to handle Esc key

   signals:
      void closed();
};
