#pragma once
#include <QDialog>
#include "ui_options_window.h"

// use QWidget because we're gonna stuff it into a QMdiSubWindow or whatever
class OptionsWindow : public QDialog {
   Q_OBJECT;
   public:
      OptionsWindow(QWidget* parent = nullptr);
      
   private:
      Ui::OptionsWindow ui;
};