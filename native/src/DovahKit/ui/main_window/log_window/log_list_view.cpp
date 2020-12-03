#include "log_list_view.h"
#include <QHeaderView>
#include <QLineEdit>
#include "../../../helpers/qt/strings.h"
#include "../../../dovah/notice_code_list.h"
#include "../../../editor/core.h"
#include "../../../editor/open_window_for_form.h"

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

#pragma region LogListModelItem
LogListModelItem::LogListModelItem(const QString& t) {
   this->type = type_t::text;
   this->text = t;
}
LogListModelItem::LogListModelItem(const dovah::file_read_warning& warning) {
   using notice_code = dovah::notice_code;
   //
   this->data = warning;
   this->type = type_t::file_read_warning;
   if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
      this->file = QString::fromStdString(warning.cause_file);
   }
   //
   switch (warning.code) {
      case notice_code::form_override_has_type_mismatch:
         {
            text = QObject::tr("File %4 is attempting to override form %1 (defined in file %2) with form %3. The form types are mismatched; the override will not be loaded.", "log window");
            QString file_a = QObject::tr("<unknown filename>", "log window");
            QString file_b = file_a;
            QString form_a = QObject::tr("<unknown form>", "log window");
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
            text = QObject::tr("File %4 is attempting to override form %1 (defined in file %2) with form %3. The form types are mismatched, but Skyrim allows ARMA/ARMO mismatches as a legacy behavior from Fallout 3's GECK. This override will be loaded as %5.", "log window");
            QString file_a = QObject::tr("<unknown filename>", "log window");
            QString file_b = file_a;
            QString form_a = QObject::tr("<unknown form>", "log window");
            QString form_b = form_a;
            QString final_signature = QObject::tr("????", "log window - unknown signature");
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
            QString form      = QObject::tr("<unknown cell>", "log window");
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = QObject::tr("%1 contained subrecord %2, which is handled differently for interior and exterior cells; however, the cell's DATA subrecord has not yet appeared, so the cell will default to being an exterior.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::exterior_cell_data_in_interior_cell:
         {
            QString form      = QObject::tr("<unknown cell>", "log window");
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = QObject::tr("%1 contained at least one subrecord %2, exclusive to exterior cells, but the cell is flagged as an interior.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::interior_cell_data_in_exterior_cell:
         {
            QString form      = QObject::tr("<unknown cell>", "log window");
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = QObject::tr("%1 contained at least one subrecord %2, exclusive to interior cells, but the cell is flagged as an exterior.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::unrecognized_subrecord:
         {
            QString form      = QObject::tr("<unknown form>", "log window");
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               QString file = QString::fromStdString(warning.cause_file);
               text = QObject::tr("Form %1 in file %3 contained at least one unrecognized subrecord with signature %2.")
                  .arg(form).arg(subrecord).arg(file);
            } else {
               text = QObject::tr("Form %1 contained at least one unrecognized subrecord with signature %2.")
                  .arg(form).arg(subrecord);
            }
         }
         break;
      case notice_code::form_reference_is_of_incorrect_type:
         {
            QString referrer  = QObject::tr("<unknown form>", "log window");
            QString referent  = referrer;
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            QString desired   = QObject::tr("<unknown type", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               referrer = _read_error_form_id_to_string(warning.cause_form);
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
            // %1: Referrer form
            // %2: Subrecord signature
            // %3: Referent form
            // %4: Desired form type
            // %5: Subrecord or referent index
            //
            bool has_desired_type = warning.flags & dovah::file_read_warning::flag::has_cause_form_type;
            bool has_index        = warning.flags & dovah::file_read_warning::flag::has_cause_subrecord_index;
            bool has_form_index   = warning.flags & dovah::file_read_warning::flag::has_cause_form_index;
            if (has_form_index) {
               if (has_desired_type)
                  text = QObject::tr("Form %1 refers to multiple forms using %2 subrecord(s); referent form #%5 was %3, but was supposed to refer to a form of type %4.");
               else
                  text = QObject::tr("Form %1 refers to multiple forms using %2 subrecord(s); referent form #%5 was %3, which is not the correct type.");
            } else if (has_index) {
               if (has_desired_type)
                  text = QObject::tr("Form %1 contained multiple %2 subrecords; %2[%5] referred to form %3, but was supposed to refer to a form of type %4.");
               else
                  text = QObject::tr("Form %1 contained multiple %2 subrecords; %2[%5] referred to form %3, which is not the correct type.");
            } else {
               if (has_desired_type)
                  text = QObject::tr("Form %1 contained a %2 subrecord that referred to form %3, but was supposed to refer to a form of type %4.");
               else
                  text = QObject::tr("Form %1 contained a %2 subrecord that referred to form %3, which is not the correct type.");
            }
            //
            text = text.arg(referrer).arg(subrecord).arg(referent).arg(desired).arg(warning.cause_subrecord_index);
         }
         break;
      case notice_code::shout_has_wrong_word_count:
         {
            QString form = QObject::tr("<unknown shout>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            //
            auto word_count = warning.extra_integers[0];
            if (word_count > 3) {
               text = QObject::tr("Form %1 defined %2 words. A shout record must have exactly three words; if it has more than that, then Skyrim will write past the end of the in-memory shout's word list while loading and trash memory.")
                  .arg(form)
                  .arg(word_count);
            } else {
               text = QObject::tr("Form %1 defined %2 words. A shout must have exactly three words; in particular, if an override defines fewer than three words, then it will fail to override the words it leaves undefined.")
                  .arg(form)
                  .arg(word_count);
            }
         }
         break;
      case notice_code::package_event_dialogue_unrecognized_subrecord:
         {
            QString form      = QObject::tr("<unknown form>", "log window");
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            text = QObject::tr("A piece of package event dialogue data in form %1 contained at least one unrecognized subrecord with signature %2. This could be a serious problem, as package event dialogue data will blindly consume subrecords until it finds one it expects.")
               .arg(form)
               .arg(subrecord);
         }
         break;
      case notice_code::game_setting_record_is_nameless:
         {
            QString form = QObject::tr("<unknown GMST record>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            //
            text = QObject::tr("Record %1 has no editor ID (EDID) or an empty editor ID. Skyrim and the Creation Kit would skip it, so DovahKit is skipping it as well.").arg(form);
         }
         break;
      case notice_code::game_setting_record_is_misordered:
         {
            QString form = QObject::tr("<unknown GMST record>", "log window");
            QString name = QObject::tr("", "log window - missing editor ID");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = QObject::tr("Record %1%2 has its EDID subrecord in the wrong place. The EDID subrecord must be the first subrecord in order for it to be properly seen.").arg(form).arg(name);
         }
         break;
      case notice_code::subrecord_has_extra_content:
         {
            QString form      = QObject::tr("<unknown form>", "log window");
            QString subrecord = QObject::tr("<unknown subrecord>", "log window");
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_subrecord) {
               subrecord = cobb::qt::four_cc_to_string(warning.cause_subrecord);
            }
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               QString file = QString::fromStdString(warning.cause_file);
               text = QObject::tr("Form %1 in file %3 contained a %2 subrecord with extra bytes at the end.")
                  .arg(form).arg(subrecord).arg(file);
            } else {
               text = QObject::tr("Form %1 contained a %2 subrecord with extra bytes at the end.")
                  .arg(form).arg(subrecord);
            }
         }
         break;
      case notice_code::game_setting_record_is_redundant:
         {
            text = QObject::tr("Record %1%4 in file %3 redundantly defines the same setting as record %2 in the same file. The setting will use the last-loaded value and form ID.", "log window");
            QString file   = QObject::tr("<unknown filename>",    "log window");
            QString form_a = QObject::tr("<unknown GMST record>", "log window");
            QString form_b = form_a;
            QString name   = QObject::tr("", "log window - missing editor ID");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form_a = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (!warning.relevant_forms.empty()) {
               form_b = _read_error_form_id_to_string(warning.relevant_forms[0]);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = text.arg(form_a).arg(form_b).arg(file).arg(name);
         }
         break;
      case notice_code::game_setting_record_has_bad_form_id:
         {
            text = QObject::tr("Record %1%3 in file %2 has an out-of-bounds or otherwise invalid form ID. This is incorrect, but the setting should still load properly.", "log window");
            QString file = QObject::tr("<unknown filename>",    "log window");
            QString form = QObject::tr("<unknown GMST record>", "log window");
            QString name = QObject::tr("", "log window - missing editor ID");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = text.arg(form).arg(file).arg(name);
         }
         break;
      case notice_code::game_setting_record_has_bad_type:
         {
            text = QObject::tr("Record %1%3 in file %2 is of an unidentified type. Its value could not be loaded.", "log window");
            QString file = QObject::tr("<unknown filename>",    "log window");
            QString form = QObject::tr("<unknown GMST record>", "log window");
            QString name = QObject::tr("", "log window - missing editor ID");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = text.arg(form).arg(file).arg(name);
         }
         break;
      case notice_code::game_setting_name_is_unrecognized:
         {
            text = QObject::tr("Record %1%3 in file %2 defines an unrecognized game setting.", "log window");
            QString file = QObject::tr("<unknown filename>",    "log window");
            QString form = QObject::tr("<unknown GMST record>", "log window");
            QString name = QObject::tr("", "log window - missing editor ID");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = text.arg(form).arg(file).arg(name);
         }
         break;
      case notice_code::game_setting_record_has_no_data:
         {
            text = QObject::tr("Record %1%3 in file %2 is missing its DATA subrecord.", "log window");
            QString file = QObject::tr("<unknown filename>",    "log window");
            QString form = QObject::tr("<unknown GMST record>", "log window");
            QString name = QObject::tr("", "log window - missing editor ID");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = text.arg(form).arg(file).arg(name);
         }
         break;
      case notice_code::game_setting_record_unreadable_data:
         {
            text = QObject::tr("Record %1%3 in file %2 contained a DATA subrecord that could not be read, possibly because it was too small.", "log window");
            QString file = QObject::tr("<unknown filename>",    "log window");
            QString form = QObject::tr("<unknown GMST record>", "log window");
            QString name = QObject::tr("", "log window - missing editor ID");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_editor_id) {
               name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(warning.cause_editor_id));
            }
            //
            text = text.arg(form).arg(file).arg(name);
         }
         break;
      case notice_code::singleton_form_is_redundantly_defined:
         {
            text = QObject::tr("File %1 contains multiple records that define the same singleton: %2 and %3. The game only allows one form of this type to exist; all records are loaded into that one form, so there's no need to have multiple records.", "log window");
            QString file   = QObject::tr("<unknown filename>",    "log window");
            QString form_a = QObject::tr("<unknown GMST record>", "log window");
            QString form_b = form_a;
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form_a = _read_error_form_id_to_string(warning.cause_form);
            }
            if (!warning.relevant_forms.empty()) {
               form_b = _read_error_form_id_to_string(warning.relevant_forms[0]);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            //
            text = text.arg(file).arg(form_a).arg(form_b);
         }
         break;
      case notice_code::record_found_in_wrong_top_level_group:
         {
            text = QObject::tr("File %1 contained form %2 inside of the %3 GRUP.", "log window");
            QString file = QObject::tr("<unknown filename>",     "log window");
            QString form = QObject::tr("<unknown form>",         "log window");
            QString grup = QObject::tr("<unknown record group>", "log window");
            //
            if (warning.flags & dovah::file_read_warning::flag::has_cause_form) {
               form = _read_error_form_id_to_string(warning.cause_form);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_file) {
               file = QString::fromStdString(warning.cause_file);
            }
            if (warning.flags & dovah::file_read_warning::flag::has_cause_signature) {
               grup = cobb::qt::four_cc_to_string(warning.cause_signature);
            }
            //
            text = text.arg(file).arg(form).arg(grup);
         }
         break;
   }
}
LogListModelItem::LogListModelItem(const dovah::detailed_notice& error) {
   using notice_code = dovah::notice_code;
   //
   bool non_continuable_success = false;
   switch (error.code) {
      case notice_code::unknown_form_type:
         text = QObject::tr("One of the forms that needs to be saved is of a type that DovahKit has not yet been programmed to handle.", "write error");
         break;
      case notice_code::no_active_file:
         text = QObject::tr("You did not select an active file, and there is no room in this load order for another file.", "write error");
         break;
      case notice_code::cannot_save_right_now:
         text = QObject::tr("It is not safe to save right now, because DovahKit is currently performing some other operation (e.g. a load or save).", "write error");
         break;
      case notice_code::no_filename_specified:
         text = QObject::tr("The active file is implicit (nameless) and no filename was provided. (Wait, what? How did this happen? We should've made you either provide a name or cancel.)", "write error");
         break;
      case notice_code::save_complete_but_reopen_failed:
         non_continuable_success = true;
         text = QObject::tr("The file was successfully saved, but could not be reopened for editing after the save. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         break;
      case notice_code::out_of_memory:
         text = QObject::tr("An out-of-memory error occurred at some point during the save process, likely while trying to write a compressed record.", "write error");
         break;
      case notice_code::zlib_memory_error:
         text = QObject::tr("A zlib memory error occurred while trying to save a compressed record.", "write error");
         break;
      case notice_code::zlib_buffer_error:
         text = QObject::tr("A zlib buffer error occurred while trying to save a compressed record.", "write error");
         break;
      case notice_code::forms_out_of_esl_form_id_range:
         text = QObject::tr("You cannot convert a file to an ESL if any of its forms have IDs above XX000FFF.", "write error");
         break;
      case notice_code::too_many_dependencies:
         text = QObject::tr("A file cannot have more than 254 dependencies.", "write error");
         break;
      case notice_code::load_order_would_overflow_into_lights:
         text = QObject::tr("The current load order would not be possible in Skyrim Special. Too many files (besides the active file) are loaded; they are overflowing into the 0xFE slot.", "write error");
         break;
      case notice_code::load_order_contains_light_files:
         text = QObject::tr("The current load order would not be possible in Skyrim Classic. The load order contains ESL files (besides the active file).", "write error");
         break;
      case notice_code::game_conversion_form_cleanup_failed:
         non_continuable_success = true;
         text = QObject::tr("The file was successfully saved, but some forms were lost during the conversion. Internal errors occurred while trying to remove these forms from memory. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         break;
      case notice_code::post_save_none_stub_cleanup_failed:
         non_continuable_success = true;
         text = QObject::tr("The file was successfully saved, but internal errors occurred while trying to clean up information on dangling form-to-form references. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
         break;
      default:
         text = QObject::tr("Unknown error.", "write error");
         break;
   }
}
bool LogListModelItem::compare(const dovah::file_read_warning& warning) const noexcept {
   if (this->type != type_t::file_read_warning)
      return false;
   return this->data == warning;
}
bool LogListModelItem::empty() const noexcept {
   return this->text.isEmpty();
}
#pragma endregion

#pragma region LogListModel
LogListModel::LogListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::fileLoadWarningReceived, this, &LogListModel::loadWarningReceived);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,     this, &LogListModel::dataAcquireComplete);
   QObject::connect(&editor, &DovahKitCore::dataSaveImminent,        this, &LogListModel::dataSaveImminent);
   QObject::connect(&editor, &DovahKitCore::dataSaveComplete,        this, &LogListModel::dataSaveComplete);
   QObject::connect(&editor, &DovahKitCore::dataSaveFailed,          this, &LogListModel::saveErrorReceived);
}

void LogListModel::dataAcquireComplete() {
   int none_stubs = 0;
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::none, [&none_stubs](dovah::form_stub* stub) {
      if (stub->is_none_stub())
         ++none_stubs;
      return false;
   });
   if (none_stubs) {
      this->addTextEntry(
         tr("Forms in the loaded files contain dangling references to %1 non-existent form(s). Check the \"Missing\" category in the Object Window for a list of the missing forms' form IDs, and view the Use Info on entries to see what's referring to them. It's normal for official game files to have this problem.", "log window")
            .arg(none_stubs)
      );
   }
   //
   this->addTextEntry(tr("All files have been loaded.", "log window"));
}
void LogListModel::dataSaveImminent() {
   this->addTextEntry(tr("Saving active file...", "log window"));
}
void LogListModel::dataSaveComplete() {
   this->addTextEntry(tr("The active file has been successfully saved.", "log window"));
}
void LogListModel::saveErrorReceived(const dovah::detailed_notice& error) {
   auto* item = new item_type(error);
   if (item->empty()) {
      delete item;
      return;
   }
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}
void LogListModel::loadWarningReceived(const dovah::file_read_warning& warning) {
   using flag = dovah::file_read_warning::flag;
   //
   if (warning.context == dovah::file_read_warning::context_t::on_demand_form_load) {
      //
      // Don't log warnings from on-demand form loads, unless the specific data is 
      // coalesced.
      //
      if (!warning.is_winning_record())
         return;
   }
   //
   auto& map = this->warnings_cause_by_form;
   auto  cause_form_id  = warning.cause_form.fixedID;
   bool  has_cause_form = warning.flags & flag::has_cause_form;
   if (has_cause_form && cause_form_id) {
      auto it = map.find(cause_form_id);
      if (it != map.end()) {
         auto& list = *it;
         for (auto* item : list) {
            if (item->compare(warning))
               return;
         }
      }
   }
   auto* item = new item_type(warning);
   if (item->empty()) {
      delete item;
      return;
   }
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   //
   this->children.push_back(item);
   if (has_cause_form && cause_form_id) {
      auto& list = map[cause_form_id];
      list.push_back(item);
   }
   //
   this->endInsertRows();
}
void LogListModel::gameSettingValueChangeFailed(const char* name, dovah::notice_code_t code) {
   auto* item = new item_type();
   switch (code) {
      case dovah::notice_code::form_id_unavailable_for_game_setting:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. DovahKit was unable to allocate a form ID for the setting.").arg(name);
         break;
      case dovah::notice_code::game_setting_edit_request_lacked_id:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. No form ID was allocated for the setting.").arg(name);
         break;
      case dovah::notice_code::form_id_is_already_in_use:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. The desired form ID is already in use by another (non-setting) form.").arg(name);
         break;
      case dovah::notice_code::form_id_is_reserved_for_other_process:
         item->text = tr("An error occurred while trying to modify the value of game setting %1. The desired form ID is reserved for use in some other process, such as form creation or form renumbering.").arg(name);
         break;
      default:
         item->text = tr("An unknown error occurred while trying to modify the value of game setting %1.").arg(name);
         break;
   }
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}

QModelIndex LogListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex LogListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int LogListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int LogListModel::columnCount(const QModelIndex& item) const {
   return 2;
}
Qt::ItemFlags LogListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant LogListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case 0:
         if (role == Qt::DisplayRole)
            return item->text;
         break;
      case 1:
         if (role == Qt::DisplayRole)
            return item->file;
         break;
   }
   return QVariant();
}
inline const LogListModel::item_type* LogListModel::row(int rowIndex) const noexcept {
   return this->children.value(rowIndex);
}
//
QVariant LogListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Text", "log window header");
            case 1: return tr("File", "log window header");
         }
         break;
   }
   return QVariant();
}

void LogListModel::addTextEntry(const QString& text) {
   auto* item = new item_type(text);
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted;
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted);
   this->children.push_back(item);
   this->endInsertRows();
}
void LogListModel::clear() {
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->warnings_cause_by_form.clear();
   this->endResetModel();
}
#pragma endregion

#pragma region LogList
LogList::LogList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type(this));
   this->verticalHeader()->setDefaultSectionSize(0);
   //
   // The next call is needed for proper word-wrapping in table cells. The "wordWrap" 
   // property on table cells enables word-wrapping if the cell is tall enough, but 
   // doesn't actually resize table cells, so by default, the table behaves exactly 
   // as if word-wrapping were disabled. The next call automatically resizes cells 
   // by way of the vertical header: even if we disable the vertical header, every 
   // row still has a vertical header section associated with it, and that can be 
   // configured to resize.
   //
   // Naturally, pretty much none of this information is mentioned in the Qt docs 
   // for QTableView::setWordWrap, at least as of this writing.
   //
   this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for proper word-wrapping in table cells
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
};
#pragma endregion