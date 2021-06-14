#include "extract_bsa_file.h"
#include "../../dovah/files/bsa/bsa_archive.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include <QDataStream>
#include <QDebug>
#include <QFile>

namespace DovahKitDebug {
   void extract_bsa_file(const std::filesystem::path& bsa, const std::string& target, const std::filesystem::path& extract_to) {
      dovah::bsa_archive archive;
      try {
         archive.open(bsa);
      } catch (const dovah::bsa_load_exception& e) {
         qDebug() << "BSA load exception: " << e.what();
         return;
      }
      auto* file = archive.lookup_file(target);
      if (!file) {
         qDebug() << "No such file.";
         return;
      }
      QFile output(QString::fromStdWString(extract_to.c_str()));
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