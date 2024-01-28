#pragma once
#include "ui_DKScriptObjectDialog.h"
#include <QDialog>

class DKFormVMADModel;

class DKScriptObjectDialog : public QDialog {
   Q_OBJECT;
   public:
      DKScriptObjectDialog(QWidget&, QModelIndex scriptModelIndex);

   protected:
      Ui::DKScriptObjectDialog ui;
      QPersistentModelIndex scriptQMI;

      QModelIndex _selectedPropertyQMI() const;

      void _showSelectedProperty();

      void _setCurrentlyFocusedValue(QVariant);
};