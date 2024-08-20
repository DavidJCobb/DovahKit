#pragma once
#include <cstdint>
#include <QDialog>
#include <QPointer>
#include "ui_log_window.h"

namespace dovahkit::subsystems::message_log {
   class model;
}

class LogWindow : public QWidget {
   Q_OBJECT;
   protected:
      using model_type = dovahkit::subsystems::message_log::model;

   public:
      LogWindow(QWidget* parent);
      ~LogWindow();

      bool hasFocus() const;
      
   protected:
      Ui::LogWindow ui;
      model_type* _model = nullptr; // non-owned

      QPointer<QWidget> _last_qmi_parent;

      virtual void changeEvent(QEvent* event) override;

      void _redraw_selected_entry();

      // The main window wraps its LogWindow instance in a QMdiSubWindow, so we also need to 
      // react to when THAT gains or loses focus.
      void _mdi_focus_handler(Qt::WindowStates prior, Qt::WindowStates after);
      void _update_mdi_focus_handler();
};
