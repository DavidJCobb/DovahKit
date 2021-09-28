#include "enumerate_bsa_contents.h"
#include <filesystem>
#include <QDebug>
#include <QFileDialog>
#include "../../dovah/files/bsa/bsa_archive.h"

namespace DovahKitDebug::features {
   /*static*/ void enumerate_bsa_contents::execute(QWidget* from) {
      auto name = QFileDialog::getOpenFileName(from, QObject::tr("Select BSA file", "debug"), "", "Bethesda Softworks Archives (*.bsa *.ba2)");
      if (name.isEmpty())
         return;
      std::filesystem::path path = std::u8string((const char8_t*)name.toUtf8().constData());
      //
      dovah::bsa_archive archive;
      try {
         archive.open(path);
      } catch (const dovah::bsa_load_exception& e) {
         qDebug() << "BSA load exception: " << e.what();
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