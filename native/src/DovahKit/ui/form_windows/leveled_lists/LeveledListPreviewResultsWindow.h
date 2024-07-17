#pragma once
#include <vector>
#include <QDialog>
#include <QTableView>
#include "dovah/utils/leveled_list_preview.h"

class LeveledListPreviewResultsWindow : public QDialog {
   Q_OBJECT;
   public:
      LeveledListPreviewResultsWindow(QWidget* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            Count,
            Form,
            Health,
            Owner
         };
      };

      void setContents(const std::vector<dovah::leveled_list_preview::entry>&);

   protected:
      struct {
         QTableView* table = nullptr;
      } subwidgets;
};