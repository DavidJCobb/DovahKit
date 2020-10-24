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
      if (form.localID) {
         local = QString("%1").arg(form.localID, 8, 16, QChar('0')).toUpper();
         return QObject::tr("[%1][Local:%2][Loaded:%3]").arg(signature).arg(local).arg(fixed);
      }
      return QObject::tr("[%1:%2]").arg(signature).arg(fixed);
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
      case notice_code::form_override_has_armo_arma_mismatch:
         {
            text = tr("File %4 is attempting to override form %1 (defined in file %2) with form %3. The form types are mismatched, but Skyrim allows ARMA/ARMO msimatches as a legacy behavior from Fallout 3's GECK. This override will be loaded as %5.", "log window");
            QString file_a = tr("<unknown filename>", "log window");
            QString file_b = file_a;
            QString form_a = tr("<unknown form>", "log window");
            QString form_b = form_a;
            QString final_signature = tr("????", "log window - unknown signature");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form_a = _read_error_form_id_to_string(warning.cause_form);
               final_signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(warning.cause_form.type).signature);
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
            text = text.arg(form_a).arg(file_a).arg(form_b).arg(file_b).arg(final_signature);
         }
         break;
      case notice_code::cell_flags_not_yet_found:
         {
            QString form      = tr("<unknown cell>", "log window");
            QString subrecord = tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = tr("%1 contained subrecord %2, which is handled differently for interior and exterior cells; however, the cell's DATA subrecord has not yet appeared, so the cell will default to being an exterior.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::exterior_cell_data_in_interior_cell:
         {
            QString form      = tr("<unknown cell>", "log window");
            QString subrecord = tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = tr("%1 contained subrecord %2, exclusive to exterior cells, but the cell is flagged as an interior.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::interior_cell_data_in_exterior_cell:
         {
            QString form      = tr("<unknown cell>", "log window");
            QString subrecord = tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = tr("%1 contained subrecord %2, exclusive to interior cells, but the cell is flagged as an exterior.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::unrecognized_subrecord:
         {
            QString form      = tr("<unknown form>", "log window");
            QString subrecord = tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = tr("Form %1 contained unrecognized an subrecord with signature %2.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::form_reference_is_of_incorrect_type:
         {
            QString referer   = tr("<unknown form>", "log window");
            QString referent  = referer;
            QString subrecord = tr("<unknown subrecord>", "log window");
            QString desired   = tr("<unknown type", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               referer = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form_type) {
               desired = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(warning.cause_form_type).signature);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            if (!warning.relevant_forms.empty()) {
               referent = _read_error_form_id_to_string(warning.relevant_forms[0]);
            }
            //
            text = tr("Form %1 contained a %2 subrecord that referred to form %3, but was supposed to refer to a form of type %4.")
               .arg(referer)
               .arg(subrecord)
               .arg(referent)
               .arg(desired);
         }
         break;
      case notice_code::shout_has_wrong_word_count:
         {
            QString form = tr("<unknown shout>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            //
            auto word_count = warning.extra_integers[0];
            if (word_count > 3) {
               text = tr("Form %1 defined %2 words. A shout record must have exactly three words; if it has more than that, then Skyrim will write past the end of the in-memory shout's word list while loading and trash memory.")
                  .arg(form)
                  .arg(word_count);
            } else {
               text = tr("Form %1 defined %2 words. A shout must have exactly three words; in particular, if an override defines fewer than three words, then it will fail to override the words it leaves undefined.")
                  .arg(form)
                  .arg(word_count);
            }
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