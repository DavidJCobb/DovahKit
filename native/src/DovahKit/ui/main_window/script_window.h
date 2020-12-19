#pragma once
#include <cstdint>
#include <QDialog>
#include <QTimer>
#include "ui_script_window.h"

class EditorScriptWindow : public QDialog {
   Q_OBJECT
   //
   public:
      EditorScriptWindow(QWidget* parent = Q_NULLPTR);
      //
   private:
      Ui::EditorScriptWindow ui;
      //
      void _onScriptStartStop(bool script_running);
};
