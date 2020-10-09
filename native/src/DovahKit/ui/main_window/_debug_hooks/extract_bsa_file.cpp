#include "extract_bsa_file.h"
#include "../../dovah/files/bsa/bsa_archive.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include <QDataStream>
#include <QDebug>
#include <QFile>

namespace DovahKitDebug {
   void enumerate_bsa_contents(const std::filesystem::path& bsa, const std::string& target, const std::filesystem::path& extract_to) {
      dovah::bsa_archive archive;
      archive.open(bsa);
      if (archive.has_error()) {
         using error_code = dovah::bsa_archive::read_error_code;
         switch (archive.get_error()) {
            case error_code::bad_header_sentinel:
               qDebug() << "BSA file had a bad header sentinel.";
               break;
         }
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