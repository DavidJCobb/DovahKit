#include "compiled_papyrus_script_tests.h"
#include <string>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "../../../dovah/files/bsa/bsa_archived_file.h"
#include "../../../dovah/files/papyrus/compiled_script.h"
#include "../../../editor/core.h"

namespace DovahKitDebug {
   void compiled_papyrus_script_tests(QWidget* window) {
      auto path = QInputDialog::getText(window, QObject::tr("Path of PEX file to test? Do not specify a Data prefix.", "debug"), QObject::tr("Path:"));
      if (path.isEmpty())
         return;
      auto& editor = DovahKitCore::get();
      auto* file   = editor.lookup_game_asset(path.toStdString());
      if (!file) {
         //
         // TODO: try getting a loose file
         //
         QMessageBox::information(window,
            QObject::tr("Report"),
            QObject::tr("File not found."),
            QMessageBox::Ok
         );
         return;
      }
      //
      dovah::compiled_papyrus_script data;
      QString failure;
      try {
         data.read_file(file->data(), file->size());
      } catch (dovah::compiled_papyrus_script::not_a_papyrus_file_exception& e) {
         failure = "This file is not a compiled Papyrus script.";
      } catch (dovah::compiled_papyrus_script::invalid_opcode_exception& e) {
         failure = QString("The opcode at offset 0x%08X is not valid; it has ID 0x%02X.").arg(e.offset).arg(e.opcode);
      } catch (dovah::compiled_papyrus_script::unexpected_eof_exception& e) {
         failure = QString("The file ended at offset 0x%08X, but we expected to find at least 0x%02X more bytes.").arg(e.offset).arg(e.desired_size);
      } catch (dovah::compiled_papyrus_script::varargs_count_type_exception& e) {
         failure = QString("A varargs opcode's count, at offset 0x%08X, was not an integer as expected, but rather identified itself as type 0x%02X.").arg(e.offset).arg(e.type);
      } catch (...) {
         failure = "Unknown error.";
      }
      delete file;
      if (!failure.isEmpty()) {
         QMessageBox::information(window,
            QObject::tr("Report"),
            QObject::tr("Failed to load the script file. %1").arg(failure),
            QMessageBox::Ok
         );
         return;
      }
      //
      QString properties;
      if (data.objects.size()) {
         auto& o = data.objects[0];
         for (auto& p : o.properties) {
            if (!properties.isEmpty())
               properties += '\n';
            properties += QString::fromUtf8(p.type.c_str());
            properties += ' ';
            properties += QString::fromUtf8(p.name.c_str());
         }
      }
      if (properties.isEmpty())
         properties = "<none>";
      QMessageBox::information(window,
         QObject::tr("Report"),
         QObject::tr("Properties:\n\n%1").arg(properties),
         QMessageBox::Ok
      );
   }
}