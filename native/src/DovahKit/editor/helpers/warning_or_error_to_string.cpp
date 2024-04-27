#include "warning_or_error_to_string.h"
#include <QObject>
#include "../../helpers/qt/strings.h"
#include "../../dovah/notice_code_list.h"

#include "../../dovah/forms/Landscape.h"

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
            //
         case notice_code::havok_data_is_not_supported_here:
            {
               text = QObject::tr("DovahKit doesn't support this type of Havok data.", "notice_code::havok_data_is_not_supported_here");
               if (notice.context == dovah::detailed_notice::notice_context::form_save) {
                  text = QObject::tr("DovahKit failed to save Havok data for form %1.", "notice_code::havok_data_is_not_supported_here");
                  //
                  QString form = QObject::tr("<unknown form>", "log window");
                  if (notice.flags & dovah::detailed_notice::flag::has_cause_form) {
                     form = _read_error_form_id_to_string(notice.cause_form);
                     if (notice.cause_form.type == dovah::form_type::land) {
                        text = QObject::tr("DovahKit does not currently support generating compressed Havok collision data for landscapes, and so failed to save the data present for %1.", "notice_code::havok_data_is_not_supported_here");
                     }
                  }
                  //
                  text = text.arg(form);
               } else {
                  // ...
               }
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