#pragma once
#include <QDialog>
#include "ui_FormSubdialogIdleNewGraph.h" // generated

class FormSubdialogIdleNewGraph : public QDialog {
   Q_OBJECT;
   public:
      FormSubdialogIdleNewGraph(QWidget* parent = nullptr);

      QString path() const;
      
   protected:
      Ui::FormSubdialogIdleNewGraph ui;

      void _update_ok_button_enable_state();
};