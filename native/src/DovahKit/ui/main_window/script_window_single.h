#pragma once
#include <QMainWindow>
#include "../../widgets/DKUnreadCountBadgePaneHeader.h"
#include "ui_script_window_single.h"

class EditorSingleScriptWindow : public QMainWindow {
   Q_OBJECT
   //
   public:
      EditorSingleScriptWindow(QWidget* parent = Q_NULLPTR);
      
   private:
      Ui::EditorSingleScriptWindow ui;
      struct {
         bool eval_pending   = false;
         bool script_running = false;
      } state;
      struct {
         DKUnreadCountBadgePaneHeader* log_header = nullptr;
      } subwidgets;
      
      void _onScriptStartStop(bool script_running);

      void _updateEvalEnableState();

      bool _checkAllowClose();

      virtual void closeEvent(QCloseEvent* event) override;
      //virtual void reject() override; // override needed to handle Esc key

   signals:
      void closed();
};
