#include "log_window.h"
#include "../../helpers/qt/strings.h"
#include "../../editor/core.h"
#include "../../dovah/files/file_read_warning.h"
#include "../../dovah/notice_code_list.h"

LogWindow::LogWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::fileLoadWarningReceived, this, &LogWindow::loadWarningReceived);
   QObject::connect(&editor, &DovahKitCore::dataSaveImminent, this, [this]() {
      this->insertLogEntry(tr("Saving active file...", "log window"));
   });
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete, this, [this]() {
      this->insertLogEntry(tr("The active file has been successfully saved.", "log window"));
   });
   QObject::connect(&editor, &DovahKitCore::dataSaveFailed, this, &LogWindow::saveErrorReceived);
}

void LogWindow::insertLogEntry(const QString& text) {
   this->ui.list->addItem(text);
}
void LogWindow::clearLog() {
   this->ui.list->clear();
}

namespace {
   QString _read_error_form_id_to_string(const dovah::file_read_warning::relevant_form& form) {
      QString signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(form.type).signature);
      QString local     = QObject::tr("????????", "log window - missing form ID");
      QString fixed     = QObject::tr("--------", "log window - missing form ID");
      if (form.fixedID)
         fixed = QString("%1").arg(form.fixedID, 8, 16, QChar('0')).toUpper();
      if (form.localID)
         fixed = QString("%1").arg(form.localID, 8, 16, QChar('0')).toUpper();
      return QObject::tr("[%1:%2]{LoadedID:%3]").arg(signature).arg(local).arg(fixed);
   }
}

void LogWindow::loadWarningReceived(const dovah::file_read_warning& warning) {
   using notice_code = dovah::notice_code;
   //
   QString text;
   switch (warning.code) {
      case notice_code::form_override_has_type_mismatch:
         {
            text = tr("File %4 is attempting to override form %1 (defined in file %2) with form %3. The form types are mismatched; the override will not be loaded.", "log window");
            QString file_a = tr("<unknown filename>", "log window");
            QString file_b = file_a;
            QString form_a = tr("<unknown form>", "log window");
            QString form_b = form_a;
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form_a = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file_a = QString::fromStdString(warning.cause_file);
            }
            if (!warning.relevant_forms.empty()) {
               form_b = _read_error_form_id_to_string(warning.relevant_forms[0]);
            }
            if (!warning.relevant_files.empty()) {
               file_b = QString::fromStdString(warning.relevant_files[0]);
            }
            //
            text = text.arg(form_a).arg(file_a).arg(form_b).arg(file_b);
         }
         break;
   }
   if (text.isEmpty())
      return;
   //
   this->insertLogEntry(text);
}
void LogWindow::saveErrorReceived(const dovah::file_write_error& error) {
   using notice_code = dovah::notice_code;
   //
   bool    non_continuable_success = false;
   QString text;
   switch (error.code) {
      case notice_code::unknown_form_type:
         text = tr("One of the forms that needs to be saved is of a type that DovahKit has not yet been programmed to handle.", "write error");
         break;
      case notice_code::no_active_file:
         text = tr("You did not select an active file, and there is no room in this load order for another file.", "write error");
         break;
      case notice_code::cannot_save_right_now:
         text = tr("It is not safe to save right now, because DovahKit is currently performing some other operation (e.g. a load or save).", "write error");
         break;
      case notice_code::no_filename_specified:
         text = tr("The active file is implicit (nameless) and no filename was provided. (Wait, what? How did this happen? We should've made you either provide a name or cancel.)", "write error");
         break;
      case notice_code::save_complete_but_reopen_failed:
         non_continuable_success = true;
         text = tr("The file was successfully saved, but could not be reopened for editing after the save. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         break;
      case notice_code::out_of_memory:
         text = tr("An out-of-memory error occurred at some point during the save process, likely while trying to write a compressed record.", "write error");
         break;
      case notice_code::zlib_memory_error:
         text = tr("A zlib memory error occurred while trying to save a compressed record.", "write error");
         break;
      case notice_code::zlib_buffer_error:
         text = tr("A zlib buffer error occurred while trying to save a compressed record.", "write error");
         break;
      case notice_code::forms_out_of_esl_form_id_range:
         text = tr("You cannot convert a file to an ESL if any of its forms have IDs above XX000FFF.", "write error");
         break;
      case notice_code::too_many_dependencies:
         text = tr("A file cannot have more than 254 dependencies.", "write error");
         break;
      case notice_code::load_order_would_overflow_into_lights:
         text = tr("The current load order would not be possible in Skyrim Special. Too many files (besides the active file) are loaded; they are overflowing into the 0xFE slot.", "write error");
         break;
      case notice_code::load_order_contains_light_files:
         text = tr("The current load order would not be possible in Skyrim Classic. The load order contains ESL files (besides the active file).", "write error");
         break;
      case notice_code::game_conversion_form_cleanup_failed:
         non_continuable_success = true;
         text = tr("The file was successfully saved, but some forms were lost during the conversion. Internal errors occurred while trying to remove these forms from memory. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         break;
      default:
         text = tr("Unknown error.", "write error");
         break;
   }
   if (text.isEmpty())
      return;
   //
   if (non_continuable_success) {
      //
      // TODO: Log any appropriate save warning.
      //
      /*if (warning.code == notice_code::save_complete_but_to_temporary_file) {
         message += tr("\r\n\r\nAn additional problem occurred: DovahKit was unable to replace the old active file with the newly-written data. Your work has been saved to %1.").arg(warning.filename.c_str());
      }*/
   } else {
      QString text = QString("Unable to save the file. %1").arg(text);
      if (error.formID) {
         text += QString("<br/>Form ID: %2<br/>Form type: %3<br/>File offset: %4")
            .arg(error.formID)
            .arg(error.form_type)
            .arg(error.file_offset);
      } else {
         if (error.has_file_offset())
            text += tr("<br/>File offset: %1").arg(error.file_offset);
      }
   }
   //
   this->insertLogEntry(text);
}