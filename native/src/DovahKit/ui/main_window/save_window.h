#pragma once
#include <cstdint>
#include <QDialog>
#include <QTimer>
#include "ui_save_window.h"

namespace dovah::tes_file_writing {
   class write_results;
}

class ActiveFileSaveDialog : public QDialog {
   Q_OBJECT
   //
   public:
      ActiveFileSaveDialog(QWidget* parent = Q_NULLPTR);
      //
   private:
      Ui::ActiveFileSaveDialog ui;

      void commit();
      void handleLastSaveError(const dovah::tes_file_writing::write_results&); // return (true) if the error prevented the save from working or made further editing impossible; false otherwise (e.g. warnings)
};
