#pragma once
#include <cstdint>
#include <QDialog>
#include <QTimer>
#include "ui_file_metadata_window.h"

class FileMetadataWindow : public QDialog {
   Q_OBJECT
   //
   public:
      FileMetadataWindow(QWidget* parent = Q_NULLPTR);
      //
   private:
      Ui::FileMetadataWindow ui;

      void commit();
};
