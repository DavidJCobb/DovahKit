#pragma once
#include <cstdint>
#include <QAction>
#include <QDialog>
#include "ui_object_window.h"

class ObjectWindow : public QWidget {
   Q_OBJECT
   //
   public:
      ObjectWindow(QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   private:
      Ui::ObjectWindow ui;
      QAction* _actionCreateForm       = nullptr;
      QAction* _formActionEdit         = nullptr;
      QAction* _formActionDuplicate    = nullptr;
      QAction* _formActionShowUseInfo  = nullptr;
      QAction* _formActionRecalcBounds = nullptr;
      QAction* _formActionRenumber     = nullptr;
      QAction* _formActionDelete       = nullptr;
      QAction* _separator = nullptr;
};
