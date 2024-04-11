#pragma once
#include "ui_DKFormDestructionDataDialog.h" // generated
#include <optional>
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
      Ui::DKFormDestructionDataDialog ui;
      DKFormDestructionStageListModel* _model = nullptr;

   protected slots:
      void _deleteSelectedStage();
      void _editSelectedStage();
};