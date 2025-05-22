#pragma once
#include "ui_DKFormDestructionDataDialog.h" // generated
#include <optional>
#include <QAction>
#include <QDialog>
#include "../DKFormDestructionDataButton.h"

class DKFormDestructionStageListModel;

class DKFormDestructionDataDialog : public QDialog {
   Q_OBJECT;
   public:
      using DestructionData = DKFormDestructionDataButton::DestructionData;

   public:
      DKFormDestructionDataDialog(QWidget* parent = nullptr);

      [[nodiscard]] std::optional<DestructionData> data() const;
      void setData(const std::optional<DestructionData>&);

   protected:
      Ui::DKFormDestructionDataDialog  ui;
      DKFormDestructionStageListModel* _model = nullptr;
      struct {
         QAction* add  = nullptr;
         QAction* edit = nullptr;
         QAction* del  = nullptr;
      } _context;

      virtual bool eventFilter(QObject* target, QEvent*) override;

   protected slots:
      void _addNewStage();
      void _deleteSelectedStage();
      void _editSelectedStage();

      void _clearData();

      void _selectRow(const QModelIndex&);
};