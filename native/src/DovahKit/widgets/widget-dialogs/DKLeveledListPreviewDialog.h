#pragma once
#include <vector>
#include <QDialog>
#include <QTableView>
#include "dovah/utils/leveled_list_preview.h"

class DKLeveledListPreviewDialog : public QDialog {
   Q_OBJECT;
   public:
      DKLeveledListPreviewDialog(QWidget* parent = nullptr);

      void setContents(const std::vector<dovah::leveled_list_preview::entry>&);

   protected:
      struct {
         QTableView* table = nullptr;
      } subwidgets;
};