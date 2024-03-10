#pragma once
#include "ui_DKScriptObjectDialog.h"
#include <optional>
#include <QDialog>

class DKBoundScriptModel;

class DKScriptObjectDialog : public QDialog {
   Q_OBJECT;
   public:
      DKScriptObjectDialog(QWidget&, QModelIndex scriptModelIndex);

   protected:
      Ui::DKScriptObjectDialog ui;
      QPersistentModelIndex    script_qmi;
      DKBoundScriptModel*      script_model = nullptr;

      std::optional<size_t> _currentArrayElementIndex() const;
      QModelIndex _selectedPropertyQMI() const;

      void _showSelectedProperty();

      void _setCurrentlyFocusedValue(QVariant);
};