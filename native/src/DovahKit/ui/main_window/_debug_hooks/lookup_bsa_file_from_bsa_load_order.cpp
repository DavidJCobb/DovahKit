#pragma once
#include <string>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "../../../dovah/files/bsa/bsa_archived_file.h"
#include "../../../editor/core.h"

namespace DovahKitDebug {
   void lookup_bsa_file_from_bsa_load_order(QWidget* window) {
      auto path = QInputDialog::getText(window, QObject::tr("Path of file to extract? Do not specify a Data prefix.", "debug"), QObject::tr("Path:"));
      if (path.isEmpty())
         return;
      auto& editor = DovahKitCore::get();
      auto* file   = editor.lookup_game_asset(path.toStdString());
      if (!file) {
         QMessageBox::information(window,
            QObject::tr("Report"),
            QObject::tr("File not found."),
            QMessageBox::Ok
         );
         return;
      }
      auto result = QMessageBox::question(window,
         QObject::tr("Report"),
         QObject::tr("File found. Size is %1 bytes. Extract?").arg(file->size()),
         QMessageBox::Yes | QMessageBox::No
      );
      if (result == QMessageBox::Yes) {
         auto to = QFileDialog::getSaveFileName(window, QObject::tr("Select target file", "debug"));
         if (to.isEmpty()) {
            delete file;
            return;
         }
         QFile output(to);
         if (!output.open(QIODevice::WriteOnly)) {
            QMessageBox::information(window,
               QObject::tr("Report"),
               QObject::tr("Failed to open target output file for writing."),
               QMessageBox::Ok
            );
            delete file;
            return;
         }
         QDataStream stream(&output);
         stream.writeRawData((const char*)file->data(), file->size());
         QMessageBox::information(window,
            QObject::tr("Report"),
            QObject::tr("File extracted."),
            QMessageBox::Ok
         );
      }
      delete file;
   }
}