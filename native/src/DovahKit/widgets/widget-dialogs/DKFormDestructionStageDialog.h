#pragma once
#include "ui_DKFormDestructionStageDialog.h" // generated
#include <QDialog>
#include "../DKFormDestructionDataButton.h"

class DKFormDestructionStageDialog : public QDialog {
   Q_OBJECT;
   public:
      using DestructionStage      = DKFormDestructionDataButton::DestructionStage;
      using DestructionStageFlag  = DKFormDestructionDataButton::DestructionStageFlag;
      using DestructionStageFlags = DKFormDestructionDataButton::DestructionStageFlags;

   public:
      DKFormDestructionStageDialog(QWidget* parent = nullptr);

      DestructionStage value() const;
      void setValue(const DestructionStage&);

   protected:
      Ui::DKFormDestructionStageDialog ui;
      DestructionStage _value;
      
};
