#include "warning_or_error_to_string.h"
#include <QObject>
#include "../../helpers/qt/strings.h"
#include "../../dovah/notice_code_list.h"

namespace {
   QString _read_error_form_id_to_string(const dovah::detailed_notice::relevant_form& form) {
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

namespace editor_helpers {
   extern QString warning_or_error_to_string(const dovah::detailed_notice& notice) {
      using notice_code = dovah::notice_code;
      //
      QString text;
      //
      bool non_continuable_success = false;
      switch (notice.code) {
         case notice_code::form_override_has_type_mismatch:
            {
               text = QObject::tr("File %4 is attempting to override form %1 (defined in file %2) with form %3. The form types are mismatched; the override will not be loaded.", "log window");
               QString file_a = QObject::tr("<unknown filename>", "log window");
               QString file_b = file_a;
               QString form_a = QObject::tr("<unknown form>", "log window");
               QString form_b = form_a;
               //
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form_a = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file_a = QString::fromStdString(notice.cause_file);
               }
               if (!notice.relevant_forms.empty()) {
                  form_b = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               if (!notice.relevant_files.empty()) {
                  file_b = QString::fromStdString(notice.relevant_files[0]);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form_a = _read_error_form_id_to_string(notice.cause_form);
                  final_signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(notice.cause_form.type).signature);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file_a = QString::fromStdString(notice.cause_file);
               }
               if (!notice.relevant_forms.empty()) {
                  form_b = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               if (!notice.relevant_files.empty()) {
                  file_b = QString::fromStdString(notice.relevant_files[0]);
               }
               //
               text = text.arg(form_a).arg(file_a).arg(form_b).arg(file_b).arg(final_signature);
            }
            break;
         case notice_code::cell_flags_not_yet_found:
            {
               QString form      = QObject::tr("<unknown cell>", "log window");
               QString subrecord = QObject::tr("<unknown subrecord>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
               }
               //
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  QString file = QString::fromStdString(notice.cause_file);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  referrer = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form_type) {
                  desired = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(notice.cause_form_type).signature);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
               }
               if (!notice.relevant_forms.empty()) {
                  referent = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               //
               // %1: Referrer form
               // %2: Subrecord signature
               // %3: Referent form
               // %4: Desired form type
               // %5: Subrecord or referent index
               //
               bool has_desired_type = notice.flags & dovah::detailed_notice::flag::has_cause_form_type;
               bool has_index        = notice.flags & dovah::detailed_notice::flag::has_cause_subrecord_index;
               bool has_form_index   = notice.flags & dovah::detailed_notice::flag::has_cause_form_index;
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
               text = text.arg(referrer).arg(subrecord).arg(referent).arg(desired).arg(notice.cause_subrecord_index);
            }
            break;
         case notice_code::shout_has_wrong_word_count:
            {
               QString form = QObject::tr("<unknown shout>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               //
               auto word_count = notice.extra_integers[0];
               text = QObject::tr("Form %1 defined %2 words. A shout record must have exactly three words.")
                  .arg(form)
                  .arg(word_count);
            }
            break;
         case notice_code::package_event_dialogue_unrecognized_subrecord:
            {
               QString form      = QObject::tr("<unknown form>", "log window");
               QString subrecord = QObject::tr("<unknown subrecord>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               //
               text = QObject::tr("Record %1 has no editor ID (EDID) or an empty editor ID. Skyrim and the Creation Kit would skip it, so DovahKit is skipping it as well.").arg(form);
            }
            break;
         case notice_code::game_setting_record_is_misordered:
            {
               QString form = QObject::tr("<unknown GMST record>", "log window");
               QString name = QObject::tr("", "log window - missing editor ID");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
               }
               //
               text = QObject::tr("Record %1%2 has its EDID subrecord in the wrong place. The EDID subrecord must be the first subrecord in order for it to be properly seen.").arg(form).arg(name);
            }
            break;
         case notice_code::subrecord_has_extra_content:
            {
               QString form      = QObject::tr("<unknown form>", "log window");
               QString subrecord = QObject::tr("<unknown subrecord>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
               }
               //
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  QString file = QString::fromStdString(notice.cause_file);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form_a = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (!notice.relevant_forms.empty()) {
                  form_b = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_editor_id) {
                  name = QObject::tr(" (%1)", "log window - editor ID").arg(QString::fromStdString(notice.cause_editor_id));
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form_a = _read_error_form_id_to_string(notice.cause_form);
               }
               if (!notice.relevant_forms.empty()) {
                  form_b = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
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
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_signature) {
                  grup = cobb::qt::four_cc_to_string(notice.cause_signature);
               }
               //
               text = text.arg(file).arg(form).arg(grup);
            }
            break;
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
         case notice_code::file_has_too_many_dependencies:
            if (notice.context == dovah::detailed_notice::notice_context::file_load) {
               text = QObject::tr("The file claims to have more than 254 dependencies. This is impossible.", "read error");
            } else {
               text = QObject::tr("A file cannot have more than 254 dependencies.", "write error");
            }
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
         case notice_code::form_id_is_out_of_bounds:
            {
               QString form = QObject::tr("<unknown form ID>", "log window");
               QString file = QObject::tr("<unknown filename>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               //
               text = QObject::tr("File %2 is malformed: record %1 had a form ID whose load order prefix would place it out of bounds.", "read error").arg(form).arg(file);
            }
            break;
         case notice_code::post_save_none_stub_cleanup_failed:
            non_continuable_success = true;
            text = QObject::tr("The file was successfully saved, but internal errors occurred while trying to clean up information on dangling form-to-form references. Further editing is no longer possible; you can keep using DovahKit, but all currently loaded data will be unloaded. ", "write error");
            break;
         case notice_code::zero_is_not_an_allowed_form_id:
            {
               QString form = QObject::tr("<unknown form>", "log window");
               QString file = QObject::tr("<unknown filename>", "log window");
               QString pos  = QObject::tr("<unknown file offset>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_file_offset) {
                  pos = QString("0x%1").arg(notice.offset, 0, 16).toUpper();
               }
               //
               text = QObject::tr("File %2 is malformed: record %1 at offset %3 uses zero as its form ID.", "read error").arg(form).arg(file).arg(pos);
            }
            break;
         case notice_code::container_item_has_bad_owner_form_type:
            {
               text = QObject::tr("Form %1 in file %2 has a malformed entry in its inventory: the item's owner is form %3, which is not an ActorBase or Faction. Because the owner is of an invalid type, the additional four-byte value paired with it is also of an invalid type and will be mishandled by the editor.", "log window");
               QString file   = QObject::tr("<unknown filename>", "log window");
               QString form_a = QObject::tr("<unknown form>",     "log window");
               QString form_b = form_a;
               //
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form_a = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               if (!notice.relevant_forms.empty()) {
                  form_b = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               //
               text = text.arg(form_a).arg(file).arg(form_b);
            }
            break;
         case notice_code::active_file_is_dependency:
            text = QObject::tr("The active file is listed as another file's master. This load order is invalid, because we need the active file at the bottom of the load order.", "notice_code::active_file_is_dependency");
            break;
         case notice_code::load_order_would_have_too_many_files:
            {
               text = QObject::tr("We can't load this load order. It would have too many files.%1", "notice_code::load_order_would_have_too_many_files");
               //
               QString detail;
               auto total = notice.extra_integers[0];
               auto heavy = notice.extra_integers[1];
               auto light = notice.extra_integers[2];
               if (total) {
                  if (heavy && light) {
                     if (heavy > 255) {
                        detail = QObject::tr(" The load order contains %1 files, of which %2 are not light; there can be no more than 255 non-light files.", "notice_code::load_order_would_have_too_many_files (too many heavy)")
                           .arg(total).arg(heavy);
                     } else if (light > 4096) {
                        detail = QObject::tr(" The load order contains %1 files, of which %2 are light; there can be no more than 4096 light files.", "notice_code::load_order_would_have_too_many_files (too many light)")
                           .arg(total).arg(light);
                     }
                  } else {
                     detail = QObject::tr(" The load order contains %1 files, but the cap is 255 for games that support ESLs and 254 for games that don't.", "notice_code::load_order_would_have_too_many_files (too many total)")
                        .arg(total);
                  }
               }
               text = text.arg(detail);
            }
            break;
         case notice_code::filesystem_error:
            {
               text = QObject::tr("A filesystem error occurred.", "notice_code::filesystem_error");
               if (notice.flags & dovah::detailed_notice::flag::has_errno) {
                  QString en_text;
                  switch (notice.errno_value) {
                     case ENFILE:
                        en_text = QObject::tr("Too many files open (system-wide).", "notice::filesystem_error (errno text)");
                     case EMFILE:
                        en_text = QObject::tr("Too many files open (this process).", "notice::filesystem_error (errno text)");
                     case EINVAL:
                        en_text = QObject::tr("The file name or path may be invalid", "notice::filesystem_error (errno text)");
                     case ELOOP:
                        en_text = QObject::tr("The file was inaccessible due to a cyclical reference among symbolic links in the file path.", "notice::filesystem_error (errno text)");
                     case ENAMETOOLONG:
                        en_text = QObject::tr("The file was inaccessible; the path name (whether before or after symbolic links) is too long.", "notice::filesystem_error (errno text)");
                     case EACCES:
                        en_text = QObject::tr("The file is locked, or you do not have permission to access it.", "notice::filesystem_error (errno text)");
                     case EBUSY:
                        en_text = QObject::tr("The file is locked.", "notice::filesystem_error (errno text)");
                     case ENOENT:
                        en_text = QObject::tr("The file does not exist.", "notice::filesystem_error (errno text)");
                     case EROFS:
                        en_text = QObject::tr("The file exists on a read-only filesystem and cannot be opened for writing.", "notice::filesystem_error (errno text)");
                     case ENOMEM:
                        en_text = QObject::tr("Insufficient memory.", "notice::filesystem_error (errno text)");
                     case EISDIR:
                        en_text = QObject::tr("The 'file' is actually a directory and therefore cannot be opened for writing.", "notice::filesystem_error (errno text)");
                     case ENOTDIR:
                        en_text = QObject::tr("The specified path is not a directory.", "notice::filesystem_error (errno text)");
                  }
                  if (!en_text.isEmpty())
                     text = QObject::tr("A filesystem error occurred: %1", "notice_code::filesystem_error (errno)").arg(en_text);
               } else if (notice.flags & dovah::detailed_notice::flag::has_winapi_error_code) {
                  text = QObject::tr("A filesystem error occurred: %1", "notice_code::filesystem_error (WinAPI)")
                     .arg(cobb::qt::winapi_code_to_string(notice.winapi_error));
               }
            }
            break;
         case notice_code::interior_cell_block_has_no_parent_group:
            text = QObject::tr("The file is malformed: an interior cell block has no parent group.", "read error");
            break;
         case notice_code::interior_cell_block_group_badly_nested:
            text = QObject::tr("The file is malformed: an interior cell block is nested under a group of the wrong type or hierarchy.", "read error");
            break;
         case notice_code::invalid_record_signature:
            {
               text = QObject::tr("The file is malformed: a record had an unknown or suspicious signature.", "notice_code::invalid_record_signature (no details available)");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_signature) {
                  QString signature = cobb::qt::four_cc_to_string(notice.cause_signature);
                  bool printable = true;
                  for (auto c : signature) {
                     if (!c.isPrint()) {
                        printable = false;
                        break;
                     }
                  }
                  if (printable) {
                     text = QObject::tr("The file is malformed: a record had an unknown or suspicious signature: %1.", "notice_code::invalid_record_signature (printable)").arg(signature);
                  } else {
                     signature = QObject::tr("0x%1 0x%2 0x%3 0x%4", "hex codes")
                        .arg((notice.cause_signature >> 0x18) & 0xFF)
                        .arg((notice.cause_signature >> 0x10) & 0xFF)
                        .arg((notice.cause_signature >> 0x08) & 0xFF)
                        .arg((notice.cause_signature >> 0x00) & 0xFF);
                     text = QObject::tr("The file is malformed: a record had an unknown or suspicious signature. The signature contains unprintable characters; the raw bytes are %1.", "notice_code::invalid_record_signature (unprintable)").arg(signature);
                  }
               }
            }
            break;
         case notice_code::form_id_is_inside_of_a_missing_master:
            {
               QString form = QObject::tr("<unknown form ID>", "log window");
               QString file = QObject::tr("<unknown filename>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               //
               text = QObject::tr("File %2 is malformed: record %1 had a form ID whose load order prefix would place it inside of a missing master.", "read error").arg(form).arg(file);
            }
            break;
         case notice_code::unexpected_nested_group_in_simple_top_group:
            text = QObject::tr("A record was found in the wrong top-group.", "read error");
            break;
         case notice_code::extended_subrecord_with_no_length:
            text = QObject::tr("An extended subrecord did not supply an extended length.", "read error");
            break;
         case notice_code::form_initial_record_is_partial:
            {
               text = QObject::tr("Form %1 in file %2 is flagged as a partial record but is the first loaded record for this form. The \"partial\" flag will not be honored.", "notice_code::form_initial_record_is_partial");
               //
               QString form = QObject::tr("<unknown form>", "log window");
               QString file = QObject::tr("<unknown file>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               //
               text = text.arg(form).arg(file);
            }
            break;
         case notice_code::form_initial_record_is_partial_and_injected:
            {
               text = QObject::tr("Form %1 in file %2 is flagged as a partial record but is the first loaded record with this form ID; it's also an injected record. The game would skip this record because there is no already-loaded record for this form that isn't both partial and injected. We're skipping it as well: this form was not loaded.", "notice_code::form_initial_record_is_partial_and_injected");
               //
               QString form = QObject::tr("<unknown form>", "log window");
               QString file = QObject::tr("<unknown file>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               //
               text = text.arg(form).arg(file);
            }
            break;
         case notice_code::quest_objective_unexpected_subrecord:
            {
               QString form      = QObject::tr("<unknown form>", "log window");
               QString subrecord = QObject::tr("<unknown subrecord>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_subrecord) {
                  subrecord = cobb::qt::four_cc_to_string(notice.cause_subrecord);
               }
               //
               text = QObject::tr("A quest objective in form %1 contained at least one unrecognized subrecord with signature %2. This could be a serious problem, as a quest objective will blindly consume subrecords until it finds one it expects.")
                  .arg(form)
                  .arg(subrecord);
            }
            break;
         case notice_code::parent_form_is_missing:
            {
               text = QObject::tr("Form %1 in file %2 claims to belong to a non-existent parent form.", "notice_code::parent_form_is_missing");
               //
               QString form = QObject::tr("<unknown form>", "log window");
               QString file = QObject::tr("<unknown file>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file = QString::fromStdString(notice.cause_file);
               }
               //
               text = text.arg(form).arg(file);
            }
            break;
         case notice_code::partial_info_override_has_different_parent:
            {
               text = QObject::tr("TopicInfo %1, originally defined in file %3, has an override in file %2 that is flagged as partial "
                                  "and that moves the TopicInfo from Topic %4 to Topic %5. Skyrim does not properly handle partial-flagged "
                                  "TopicInfo overrides that re-parent the TopicInfo; depending on the precise circumstances under which "
                                  "this override is loaded, Skyrim may inadvertently associate the TopicInfo with multiple Topics, may "
                                  "desynchronize the TopicInfo such that it thinks it's inside of a different Topic than the one it's "
                                  "actually in, or may discard an unrelated TopicInfo from the Topic that this TopicInfo has been moved to. "
                                  "Though the effect that these issues have on game stability is not known as of this writing, this problem "
                                  "should be considered unsafe. DovahKit will interpret this data by re-parenting the TopicInfo as normal, "
                                  "but this is just the way the data is interpreted (in lieu of DovahKit actually trying to mimic the game's "
                                  "utter confusion) and is not an attempt at repairing the data. If circumstances allow, you should remove "
                                  "the \"partial\" flag from this override using xEdit or a similar tool.",
                  "notice_code::partial_info_override_has_different_parent"
               );
               //
               QString form          = QObject::tr("<unknown form>", "log window");
               QString file_initial  = QObject::tr("<unknown file>", "log window");
               QString file_sent_to  = file_initial;
               QString topic_initial = QObject::tr("<unknown topic>", "log window");
               QString topic_sent_to = topic_initial;
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               if (notice.flags & dovah::detailed_notice::flag::has_cause_file) {
                  file_sent_to = QString::fromStdString(notice.cause_file);
               }
               if (!notice.relevant_files.empty()) {
                  file_initial = QString::fromStdString(notice.relevant_files[0]);
               }
               if (!notice.relevant_forms.empty()) {
                  topic_initial = _read_error_form_id_to_string(notice.relevant_forms[0]);
                  if (notice.relevant_forms.size() > 1)
                     topic_sent_to = _read_error_form_id_to_string(notice.relevant_forms[1]);
               }
               //
               text = text.arg(form).arg(file_sent_to).arg(file_initial).arg(topic_initial).arg(topic_sent_to);
            }
            break;
         case notice_code::dialogue_branch_mishandled_owning_quest_id:
            {
               text = QObject::tr("DialogueBranch %1 uses Quest %2 as its owning quest. The game will not load this properly, because it  "
                                  "accidentally performs the local-to-global form ID conversion twice, and this form ID can't survive "
                                  "that conversion. DovahKit will not attempt to replicate this error -- we'll load the form ID properly -- "
                                  "but you should be aware that this value won't work. If a quest's local and global form IDs aren't the "
                                  "same, then it can't be safely used as a dialogue branch's owning quest.",
                  "notice_code::dialogue_branch_mishandled_owning_quest_id"
               );
               //
               QString dlbr = QObject::tr("<unknown form>", "log window");
               QString qust = dlbr;
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  dlbr = _read_error_form_id_to_string(notice.cause_form);
               }
               if (!notice.relevant_forms.empty()) {
                  qust = _read_error_form_id_to_string(notice.relevant_forms[0]);
               }
               //
               text = text.arg(dlbr).arg(qust);
            }
            break;
         case notice_code::worldspace_is_its_own_parent:
            {
               text = QObject::tr("Worldspace %1 is its own parent. The game will freeze when trying to load it.", "notice_code::worldspace_is_its_own_parent");
               //
               QString form = QObject::tr("<unknown form>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               //
               text = text.arg(form);
            }
            break;
         case notice_code::quest_has_phantom_script_data:
            {
               text = QObject::tr("Quest %1 contains script data for non-existent aliases or log entries, or for aliases from other quests. This data will be discarded by the editor.", "notice_code::quest_has_phantom_script_data");
               //
               QString form = QObject::tr("<unknown form>", "log window");
               if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                  form = _read_error_form_id_to_string(notice.cause_form);
               }
               //
               text = text.arg(form);
            }
            break;
            //
         case notice_code::unknown_error:
         default:
            text = QObject::tr("Unknown error.", "write error");
            break;
      }
      return text;
   }
}