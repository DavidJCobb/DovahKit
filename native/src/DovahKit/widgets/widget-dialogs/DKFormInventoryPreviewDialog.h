#pragma once
#include <vector>
#include <QDialog>
#include <QTableView>
#include "dovah/utils/leveled_list_preview.h"

class DKFormInventoryPreviewDialog : public QDialog {
   Q_OBJECT;
   public:
      DKFormInventoryPreviewDialog(QWidget* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            Count,
            Form,
            Health,
            Owner,
            Value,
         };
      };

      void setContents(const std::vector<dovah::leveled_list_preview::entry>&);

   protected:
      struct {
         QTableView* table = nullptr;
      } subwidgets;
};