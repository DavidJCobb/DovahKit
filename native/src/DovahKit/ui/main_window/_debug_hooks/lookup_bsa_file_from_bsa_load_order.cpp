#pragma once
#include "lookup_bsa_file_from_bsa_load_order.h"
#include <string>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "../../../dovah/files/bsa/bsa_archived_file.h"
#include "../../../editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void lookup_bsa_file_from_bsa_load_order::execute(QWidget* from) {
      auto path = QInputDialog::getText(from, QObject::tr("Path of file to extract? Do not specify a Data prefix.", "debug"), QObject::tr("Path:", "debug"));
      if (path.isEmpty())
         return;
      auto& editor = DovahKitCore::get();
      auto* file   = editor.lookup_game_asset((const char8_t*)path.toUtf8().constData());
      if (!file) {
         QMessageBox::information(from,
            QObject::tr("Report"),
            QObject::tr("File not found."),
            QMessageBox::Ok
         );
         return;
      }
      auto result = QMessageBox::question(from,
         QObject::tr("Report", "debug"),
         QObject::tr("File found. Size is %1 bytes. Extract?", "debug").arg(file->size()),
         QMessageBox::Yes | QMessageBox::No
      );
      if (result == QMessageBox::Yes) {
         auto to = QFileDialog::getSaveFileName(from, QObject::tr("Select target file", "debug"));
         if (to.isEmpty()) {
            delete file;
            return;
         }
         QFile output(to);
         if (!output.open(QIODevice::WriteOnly)) {
            QMessageBox::information(from,
               QObject::tr("Report", "debug"),
               QObject::tr("Failed to open target output file for writing.", "debug"),
               QMessageBox::Ok
            );
            delete file;
            return;
         }
         QDataStream stream(&output);
         stream.writeRawData((const char*)file->data(), file->size());
         QMessageBox::information(from,
            QObject::tr("Report", "debug"),
            QObject::tr("File extracted.", "debug"),
            QMessageBox::Ok
         );
      }
      delete file;
   }
}