#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_object_window.h"

class ObjectWindow : public QWidget {
   Q_OBJECT
   //
   public:
      ObjectWindow(QWidget* parent = Q_NULLPTR); // needs to be public for Qt? but do not call; use the static getter
      //
   private slots:
      //
   private:
      Ui::ObjectWindow ui;
};
