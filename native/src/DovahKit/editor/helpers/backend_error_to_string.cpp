#include "./backend_error_to_string.h"
#include <QObject>
#include "helpers/dynamic_fast_cast.h"
#include "dovah/notices/_all_errors.h"

#include "./form_identifiers_to_string.h"
#include "./form_type_name_to_string.h"
#include "helpers/qt/strings.h"

#include "dovah/form_stub.h"

namespace {
   namespace form_save_errors {
      using namespace dovah::notices::form_save_errors;
   }
}

namespace editor_helpers {
   extern QString backend_error_to_string(const dovah::notices::base_error& warning) {
      constexpr const char* disambig = "backend errors";

      #pragma region form save errors
         if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::form_type_is_unimplemented*>(&warning)) {
            QString subject = form_identifiers_to_string(&casted->subject);
            //
            return QObject::tr(
               "%1 is of a type that isn't yet implemented in DovahKit.",
               disambig
            ).arg(subject);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::length_prefixed_string_is_too_long_to_serialize*>(&warning)) {
            QString subject   = form_identifiers_to_string(&casted->subject);
            QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
            //
            return QObject::tr(
               "Failed to serialize %1 subrecord %2: a length-prefixed string had length %3, but the maximum supported length is %4.",
               disambig
            ).arg(subject).arg(subrecord).arg(casted->size).arg(casted->max_serializable_size);
         }
         //
         #pragma region by form component
            #pragma region destruction data
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::destruction::too_many_stages*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Destruction data for form %1 contains %2 stages, but the file format can only encode %3 stages.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
            #pragma endregion
            #pragma region papyrus
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_perk_fragments*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Papyrus data for perk %1 defines %2 fragment scripts, but the file format can only encode %3 fragments.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_properties_on_script*>(&warning)) {
                  QString subject    = form_identifiers_to_string(&casted->subject);
                  QString scriptname = QString::fromUtf8(QByteArray::fromStdString(casted->scriptname));
                  //
                  return QObject::tr(
                     "Papyrus data for form %1 attempts to define %3 properties on script %2, but the file format can "
                     "only encode %4 properties for a single attached script.",
                     disambig
                  ).arg(subject).arg(scriptname).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_scene_phase_fragments*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Papyrus data for scene %1 attempts to define %2 scene phase fragments, but the file format can only "
                     "encode %3 fragments.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_scripts*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Papyrus data for %1 has %2 attached scripts, but the file format can only encode %3 scripts.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
            #pragma endregion
         #pragma endregion
         #pragma region by form type
            #pragma region landscape
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::landscape::heightmap_contains_too_steep_a_slope*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Landspace %1 contains too steep a slope, from vertex #%2 (height %3) to vertex #%4 (height %5). The "
                     "allowed height range is [%6, %7].",
                     disambig
                  )
                     .arg(subject)
                     .arg(casted->vertex_index_a)
                     .arg(casted->vertex_height_a)
                     .arg(casted->vertex_index_b)
                     .arg(casted->vertex_height_b)
                     .arg(form_save_errors::by_type::landscape::heightmap_contains_too_steep_a_slope::allowed_height_range.first)
                     .arg(form_save_errors::by_type::landscape::heightmap_contains_too_steep_a_slope::allowed_height_range.second);
               }
            #pragma endregion
            #pragma region quest
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::quest::too_many_log_entry_papyrus_fragments*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Papyrus data for quest %1 attempts to define %2 log entry fragments, but the file format can only "
                     "encode %3 fragments.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::quest::too_many_scripted_aliases*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Quest %1 attempts to attach Papyrus scripts to %2 aliases, but the file format can only attach "
                     "scripts to up to %3 aliases at a time.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
            #pragma endregion
         #pragma endregion
      #pragma endregion

      return QObject::tr("Unknown error.", disambig);
   }
}