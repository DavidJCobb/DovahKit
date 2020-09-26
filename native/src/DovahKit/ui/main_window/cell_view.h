#pragma once
#include <cstdint>
#include <QAction>
#include <QDialog>
#include "ui_cell_view.h"

class CellViewWindow : public QWidget {
   Q_OBJECT
   //
   public:
      CellViewWindow(QWidget* parent = Q_NULLPTR);
      //
   private slots:
      void setAllEnableStates(bool);
      //
   private:
      Ui::CellViewWindow ui;
};
