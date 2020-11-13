#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_default_object_window.h"

class DefaultObjectWindow : public QDialog {
   Q_OBJECT
   //
   public:
      DefaultObjectWindow(QWidget* parent);
      //
   private:
      Ui::DefaultObjectWindow ui;
};
