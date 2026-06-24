#include "./backend_error_to_string.h"
#include <QObject>
#include "helpers/dynamic_fast_cast.h"
#include "dovah/notices/_all_errors.h"

#include "./form_identifiers_to_string.h"
#include "./form_type_name_to_string.h"
#include "helpers/qt/strings.h"

#include "dovah/form_stub.h"

namespace {
   namespace file_load_errors {
      using namespace dovah::notices::file_load_errors;
   }
   namespace form_save_errors {
      using namespace dovah::notices::form_save_errors;
   }

   static QString _hex_offset_to_string(size_t offset) {
      return QObject::tr("0x%1", "hex prefix").arg(
         QString::number(offset, 16).toUpper()
      );
   }
}

namespace editor_helpers {
   extern QString backend_error_to_string(const dovah::notices::base_error& warning) {
      constexpr const char* disambig = "backend errors";

      #pragma region file load errors
         if (auto* base_cast = dynamic_cast<const dovah::notices::base_file_load_error*>(&warning)) {
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::extended_subrecord_marker_is_invalid*>(&warning)) {
               return QObject::tr(
                  "An extended subrecord did not supply an extended length.",
                  disambig
               );
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::file_header_lists_too_many_dependencies*>(&warning)) {
               return QObject::tr(
                  "A file claims to have more than 254 dependencies. This is impossible.",
                  disambig
               );
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::filesystem_error*>(&warning)) {
               QString detail;
               if (casted->errno_value.has_value()) {
                  auto code = casted->errno_value.value();
                  switch (code) {
                     case ENFILE:
                        detail = QObject::tr("Too many files open (system-wide).", disambig);
                        break;
                     case EMFILE:
                        detail = QObject::tr("Too many files open (this process).", disambig);
                        break;
                     case EINVAL:
                        detail = QObject::tr("The file name or path may be invalid", disambig);
                        break;
                     case ELOOP:
                        detail = QObject::tr("The file was inaccessible due to a cyclical reference among symbolic links in the file path.", disambig);
                        break;
                     case ENAMETOOLONG:
                        detail = QObject::tr("The file was inaccessible; the path name (whether before or after symbolic links) is too long.", disambig);
                        break;
                     case EACCES:
                        detail = QObject::tr("The file is locked, or you do not have permission to access it.", disambig);
                        break;
                     case EBUSY:
                        detail = QObject::tr("The file is locked.", disambig);
                        break;
                     case ENOENT:
                        detail = QObject::tr("The file does not exist.", disambig);
                        break;
                     case EROFS:
                        detail = QObject::tr("The file exists on a read-only filesystem and cannot be opened for writing.", disambig);
                        break;
                     case ENOMEM:
                        detail = QObject::tr("Insufficient memory.", disambig);
                        break;
                     case EISDIR:
                        detail = QObject::tr("The \"file\" is actually a directory and therefore cannot be opened for writing.", disambig);
                        break;
                     case ENOTDIR:
                        detail = QObject::tr("The specified path is not a directory.", disambig);
                        break;
                     default:
                        detail = QObject::tr("Errno value: %1", disambig).arg(code);
                        break;
                  }
               } else if (casted->winapi_error.has_value()) {
                  auto code = casted->winapi_error.value();
                  detail = cobb::qt::winapi_code_to_string(code);
               }

               QString format = QObject::tr("A filesystem error occurred. %1", disambig);
               if (casted->real_filename != casted->filename) {
                  format = QObject::tr("A filesystem error occurred when trying to open %1 after it was relocated to %2. %3", disambig)
                     .arg(casted->filename)
                     .arg(casted->real_filename);
               } else if (!casted->filename.empty()) {
                  format = QObject::tr("A filesystem error occurred when trying to open %1. %2", disambig)
                     .arg(casted->filename);
               }
               return format.arg(detail);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::form_id_is_invalid*>(&warning)) {
               auto type = form_type_name_to_string(casted->form.type);
               auto id   = form_id_to_string(casted->form.local_id);
               auto pos  = _hex_offset_to_string(casted->file_offset);

               switch (casted->problem) {
                  using enum std::decay_t<decltype(*casted)>::problem_code;
                  case missing_master: // this one can only happen if we failed to load a master, which implies that a file was edited between us checking the header and us loading it
                     return QObject::tr(
                        "File is malformed, or some of the loaded files were modified while DovahKit was loading them: A record of type %1 at file offset %3 had a local form ID (%2) whose load order prefix would place it inside of a missing master.",
                        disambig
                     ).arg(type).arg(id).arg(pos);

                  case out_of_bounds:
                     return QObject::tr(
                        "File is malformed: A record of type %1 at file offset %3 had a local form ID (%2) whose load order prefix would place it out of bounds.",
                        disambig
                     ).arg(type).arg(id).arg(pos);

                  case zero_is_not_allowed:
                     return QObject::tr(
                        "File is malformed: A form of type %1 at file offset %2 had zero as its form ID. This is not allowed.",
                        disambig
                     ).arg(type).arg(pos);
               }

               return QObject::tr(
                  "A form of type %1 at file offset %3 had a local form ID (%2) that was invalid in some way.",
                  disambig
               ).arg(type).arg(id).arg(pos);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::form_override_has_type_mismatch*>(&warning)) {
               auto overridden = QString("[%1:%2]")
                  .arg(form_type_name_to_string(casted->overridden_form.type))
                  .arg(form_id_to_string(casted->form_id));

               auto overriding = QString("[%1:%2]")
                  .arg(form_type_name_to_string(casted->overriding_form.type))
                  .arg(form_id_to_string(casted->form_id));

               auto overridden_file = QString::fromStdString(casted->overridden_form.source_file);
               auto overriding_file = QString::fromStdString(casted->overriding_form.source_file);

               return QObject::tr(
                  "Form %1 defined in file %2 would override form %3 defined in file %4, but their types don't "
                  "match.",
                  disambig
               ).arg(overriding).arg(overriding_file).arg(overridden).arg(overridden_file);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::form_record_present_in_game_setting_group*>(&warning)) {
               auto type = cobb::qt::four_cc_to_string(casted->record.signature);
               auto id   = form_id_to_string(casted->record.local_form_id);

               return QObject::tr(
                  "A record of type %1, with file-local form ID %2, was defined in the game setting (GMST) record group.",
                  disambig
               ).arg(type).arg(id);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::interior_cell_block_group_badly_nested*>(&warning)) {
               auto pos = _hex_offset_to_string(casted->file_offset);
               return QObject::tr(
                  "The file is malformed: an interior cell block located near %1 is nested under a group of the wrong type or hierarchy.",
                  disambig
               ).arg(pos);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::interior_cell_block_has_no_parent_group*>(&warning)) {
               auto pos = _hex_offset_to_string(casted->file_offset);
               return QObject::tr(
                  "The file is malformed: an interior cell block located near %1 has no parent group.",
                  disambig
               ).arg(pos);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::malformed_file_header*>(&warning)) {
               auto pos = _hex_offset_to_string(casted->file_offset);
               return QObject::tr(
                  "This is not a game data file, or the file header is malformed. A problem was encountered at file offset %1.",
                  disambig
               ).arg(pos);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::parent_form_is_missing*>(&warning)) {
               auto type = form_type_name_to_string(casted->form.type);
               auto id   = form_id_to_string(casted->form.local_id);

               return QObject::tr(
                  "A form of type %1, with file-local form ID %2, claims to belong to a non-existent parent "
                  "form.",
                  disambig
               ).arg(type).arg(id);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::record_decompression_failed*>(&warning)) {
               QString lead_in = QObject::tr(
                  "There was a problem with a record of type %1 and file-local form ID %2."
               ).arg(cobb::qt::four_cc_to_string(casted->record.signature)).arg(form_id_to_string(casted->record.local_form_id));

               QString explanation;
               switch (casted->problem) {
                  using enum std::decay_t<decltype(*casted)>::problem_code;
                  case claimed_size_is_too_huge_to_even_try:
                     explanation = QObject::tr("The record's expected size was too large to load.");
                     break;
                  case data_larger_than_expected:
                     explanation = QObject::tr("The record is compressed, and the uncompressed data is larger than was indicated in the file.");
                     break;
                  case data_corrupt_or_incomplete:
                     explanation = QObject::tr("The record is compressed, and the compressed data is either corrupt or incomplete.");
                     break;
                  case uncompressed_data_is_not_of_declared_size:
                     explanation = QObject::tr("The record is compressed, and the uncompressed data is not the size that the file told us to expect.");
                     break;
                  case zlib_memory_error:
                     explanation = QObject::tr("The record is compressed, and a memory error occurred while decompressing it.");
                     break;
                  case zlib_unknown_error:
                     explanation = QObject::tr("The record is compressed, and an unknown error occurred while decompressing it.");
                     break;
               }

               return QObject::tr(
                  "%1 %2",
                  disambig
               ).arg(lead_in).arg(explanation);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::record_has_invalid_signature*>(&warning)) {
               auto raw_signature = casted->record.signature;

               bool all_printable = true;
               for (size_t i = 0; i < 4; ++i) {
                  char c = (raw_signature >> (i * 8)) & 0xFF;
                  if (c >= '!' && c <= '~' && c != '"')
                     continue;
                  all_printable = false;
                  break;
               }

               QString code;
               if (all_printable) {
                  code = QObject::tr("\"%1\"").arg(cobb::qt::four_cc_to_string(raw_signature));
               } else {
                  for (size_t i = 0; i < 4; ++i) {
                     uint8_t octet = (raw_signature >> (i * 8)) & 0xFF;
                     if (octet < 16)
                        code += '0';
                     code += QString::number(octet, 16).toUpper();
                     if (i != 3)
                        code += ' ';
                  }
               }

               auto local_id = form_id_to_string(casted->record.local_form_id);
               auto pos      = _hex_offset_to_string(casted->file_offset);

               return QObject::tr(
                  "The record with file-local form ID %1, at offset %2, had an unrecognized signature: %3.",
                  disambig
               ).arg(local_id).arg(pos).arg(code);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::record_is_too_large*>(&warning)) {
               auto signature = cobb::qt::four_cc_to_string(casted->record.signature);
               auto local_id  = form_id_to_string(casted->record.local_form_id);
               auto pos       = _hex_offset_to_string(casted->file_offset);

               return QObject::tr(
                  "The %1 record with file-local form ID %2, at file offset %3, was too large to load.",
                  disambig
               ).arg(signature).arg(local_id).arg(pos);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const file_load_errors::unexpected_nested_group_in_simple_top_group*>(&warning)) {
               auto pos = _hex_offset_to_string(casted->file_offset);
               return QObject::tr(
                  "The file is malformed: a nested record group was found inside of a top-level record group that should not have nesting.",
                  disambig
               ).arg(pos);
            }
         }
      #pragma endregion

      #pragma region form save errors
         if (auto* base_cast = dynamic_cast<const dovah::notices::base_form_save_error*>(&warning)) {
            QString subject = form_identifiers_to_string(&base_cast->subject);

            if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::form_type_is_unimplemented*>(&warning)) {
               return QObject::tr(
                  "%1 is of a type that isn't yet implemented in DovahKit.",
                  disambig
               ).arg(subject);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::length_prefixed_string_is_too_long_to_serialize*>(&warning)) {
               QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
               //
               return QObject::tr(
                  "Failed to serialize %1 subrecord %2: a length-prefixed string had length %3, but the maximum supported length is %4.",
                  disambig
               ).arg(subject).arg(subrecord).arg(casted->size).arg(casted->max_serializable_size);
            }
            if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::unprefixed_string_is_too_long_to_serialize*>(&warning)) {
               QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
               
               QString format = QObject::tr("Failed to serialize %1 subrecord %2: a string had length %3, but the maximum supported length is %4.", disambig);
               if (casted->subject.form_type == dovah::form_type::statik) {
                  if (casted->subrecord_signature == 'MNAM') {
                     format = QObject::tr("Failed to serialize one of the LOD mesh paths for %1: a string had length %3, but the maximum supported length is %4.", disambig);
                  }
               }

               return format.arg(subject).arg(subrecord).arg(casted->size).arg(casted->max_serializable_size);
            }
            //
            #pragma region by form component
               #pragma region destruction data
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::destruction::too_many_stages*>(&warning)) {
                     return QObject::tr(
                        "Destruction data for form %1 contains %2 stages, but the file format can only encode %3 stages.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
               #pragma endregion
               #pragma region extra data
                  #pragma region room_ref_data
                     if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::extra_data::room_ref_data::too_many_linked_rooms*>(&warning)) {
                        return QObject::tr(
                           "Room-ref-data for form %1 contains %2 linked rooms, but the file format can only encode %3 linked rooms.",
                           disambig
                        ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                     }
                  #pragma endregion
               #pragma endregion
               #pragma region idle collection
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::idle_collection::too_many_idles*>(&warning)) {
                     return QObject::tr(
                        "Form %1 attempts to specify %2 idles, but the file format can only encode up to %3 "
                        "idles in that list.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
               #pragma endregion
               #pragma region leveled list
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::leveled_list::too_many_entries*>(&warning)) {
                     return QObject::tr(
                        "Leveled list data for form %1 contains %2 entries, but the file format can only encode %3 stages.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
               #pragma endregion
               #pragma region papyrus
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_perk_fragments*>(&warning)) {
                     return QObject::tr(
                        "Papyrus data for perk %1 defines %2 fragment scripts, but the file format can only encode %3 fragments.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_properties_on_script*>(&warning)) {
                     QString scriptname = QString::fromUtf8(QByteArray::fromStdString(casted->scriptname));
                     //
                     return QObject::tr(
                        "Papyrus data for form %1 attempts to define %3 properties on script %2, but the file format can "
                        "only encode %4 properties for a single attached script.",
                        disambig
                     ).arg(subject).arg(scriptname).arg(casted->size).arg(casted->max_serializable_size);
                  }
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_scene_phase_fragments*>(&warning)) {
                     return QObject::tr(
                        "Papyrus data for scene %1 attempts to define %2 scene phase fragments, but the file format can only "
                        "encode %3 fragments.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_component::papyrus::too_many_scripts*>(&warning)) {
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
               #pragma region package
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::package::too_many_procedure_tree_branch_children*>(&warning)) {
                     return QObject::tr(
                        "Package %1 contains a procedure tree branch with %2 children. The file format can only encode a "
                        "child count up to and including %3.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
               #pragma endregion
               #pragma region quest
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::quest::too_many_log_entry_papyrus_fragments*>(&warning)) {
                     return QObject::tr(
                        "Papyrus data for quest %1 attempts to define %2 log entry fragments, but the file format can only "
                        "encode %3 fragments.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::quest::too_many_scripted_aliases*>(&warning)) {
                     return QObject::tr(
                        "Quest %1 attempts to attach Papyrus scripts to %2 aliases, but the file format can only attach "
                        "scripts to up to %3 aliases at a time.",
                        disambig
                     ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
                  }
               #pragma endregion
               #pragma region race
                  if (auto* casted = cobb::dynamic_fast_cast<const form_save_errors::by_type::race::biped_object_name_is_too_long*>(&warning)) {
                     return QObject::tr(
                        "Biped Object Name #%2 in Race %1 has a name that is too long. The name is %3 bytes long, but the maximum supported "
                        "length is %4.",
                        disambig
                     )
                        .arg(subject)
                        .arg(casted->which)
                        .arg(casted->size)
                        .arg(casted->max_serializable_size);
                  }
               #pragma endregion
            #pragma endregion
         }
      #pragma endregion

      return QObject::tr("Unknown error.", disambig);
   }
}