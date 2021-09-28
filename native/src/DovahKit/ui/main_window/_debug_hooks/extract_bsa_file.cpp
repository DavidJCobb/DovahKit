#include "extract_bsa_file.h"
#include "../../dovah/files/bsa/bsa_archive.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>

namespace DovahKitDebug::features {
   /*static*/ void extract_bsa_file::execute(QWidget* from) {
      auto name = QFileDialog::getOpenFileName(from, QObject::tr("Select BSA file", "debug"), "", "Bethesda Softworks Archives (*.bsa *.ba2)");
      if (name.isEmpty())
         return;
      auto entry = QInputDialog::getText(from, QObject::tr("Path of file to extract? Do not specify a Data prefix.", "debug"), QObject::tr("Path:"));
      if (entry.isEmpty())
         return;
      auto to = QFileDialog::getSaveFileName(from, QObject::tr("Select target file", "debug"));
      if (to.isEmpty())
         return;
      //
      std::filesystem::path bsa = (const char8_t*)name.toUtf8().constData();
      //
      dovah::bsa_archive archive;
      try {
         archive.open(bsa);
      } catch (const dovah::bsa_load_exception& e) {
         qDebug() << "BSA load exception: " << e.what();
         return;
      }
      auto* file = archive.lookup_file(entry.toStdString());
      if (!file) {
         qDebug() << "No such file.";
         return;
      }
      QFile output(to);
      if (!output.open(QIODevice::WriteOnly)) {
         qDebug() << "Unable to open file. " << output.errorString();
         return;
      }
      QDataStream stream(&output);
      stream.writeRawData((const char*)file->data(), file->size());
      qDebug() << "File extracted.";
      delete file;
   }
}