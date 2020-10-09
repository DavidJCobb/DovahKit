#include "enumerate_bsa_contents.h"
#include "../../dovah/files/bsa/bsa_archive.h"
#include <QDebug>

namespace DovahKitDebug {
   void enumerate_bsa_contents(const std::filesystem::path& path) {
      dovah::bsa_archive archive;
      archive.open(path);
      if (archive.has_error()) {
         using error_code = dovah::bsa_archive::read_error_code;
         switch (archive.get_error()) {
            case error_code::bad_header_sentinel:
               qDebug() << "BSA file had a bad header sentinel.";
               break;
         }
         return;
      }
      archive.for_each_folder([&archive](const dovah::bsa_archive::folder_entry& folder) {
         qDebug() << "Folder: " << folder.name.c_str() << " (hash: " << QString("%1").arg(folder.hash.value, 16, 16, QChar('0')) << ")";
         qDebug() << folder.files.size() << " files.";
         archive.for_each_file_in_folder(folder, [&archive](const dovah::bsa_archive::folder_entry& containing, const dovah::bsa_archive::file_entry& file) {
            bool compressed = archive.file_is_compressed(file);
            //
            qDebug() << " - " << file.name.c_str() << " (hash: " << QString("%1").arg(file.hash.value, 16, 16, QChar('0')) << ") (compressed: " << compressed << ")";
            return false;
         });
         return false;
      });
   }
}