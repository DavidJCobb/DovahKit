#include "./backend_warning_to_string.h"
#include <QObject>
#include "helpers/dynamic_fast_cast.h"
#include "dovah/data/actor_values.h"
#include "dovah/notices/_all_warnings.h"

#include "./face_fx_phoneme_name.h"
#include "./form_identifiers_to_string.h"
#include "./form_type_name_to_string.h"
#include "../localize/package_data_type.h"
#include "helpers/qt/strings.h"

#include "dovah/form_stub.h"

namespace {
   namespace file_load_warnings {
      using namespace dovah::notices::file_load_warnings;
   }
   namespace form_load_warnings {
      using namespace dovah::notices::form_load_warnings;
   }
   namespace form_save_warnings {
      using namespace dovah::notices::form_save_warnings;
   }
}

namespace editor_helpers {
   extern QString backend_warning_to_string(const dovah::notices::base_warning& warning) {
      constexpr const char* disambig = "backend warnings";

      #pragma region file load warnings
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::form_initial_record_is_partial*>(&warning)) {
            auto subject = QString("[%1:%2]")
               .arg(editor_helpers::form_type_name_to_string(casted->record.form_type))
               .arg(editor_helpers::form_id_to_string(casted->record.global_id));

            auto file = QString::fromStdString(casted->source_file);
            if (file.isEmpty())
               file = QObject::tr("<unknown file>");

            if (casted->record_is_injected) {
               return QObject::tr(
                  "Form %1 in file %2 is flagged as a partial record but is the first loaded record for this form. "
                  "It's also injected. These three facts combined will cause the game to skip loading this record, "
                  "so DovahKit is skipping it as well."
               ).arg(subject).arg(file);
            }

            return QObject::tr(
               "Form %1 in file %2 is flagged as a partial record but is the first loaded record for this form. "
               "The \"partial\" flag will not be honored."
            ).arg(subject).arg(file);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::form_override_has_armo_arma_mismatch*>(&warning)) {
            auto overridden_form = form_identifiers_to_string(&casted->overridden_form.stub);
            auto overriding_form = QString("[%1:%2]")
               .arg(casted->overridden_form.stub.form_type == dovah::form_type::armor ? "ARMA" : "ARMO")
               .arg(editor_helpers::form_id_to_string(casted->overriding_form.form_ids.global));

            auto overridden_file = QString::fromStdString(casted->overridden_form.source_file);
            auto overriding_file = QString::fromStdString(casted->overriding_form.source_file);

            return QObject::tr(
               "File %4 is attempting to override form %1 (defined in file %2) with form %3. The form types are "
               "mismatched, but Skyrim allows ARMA/ARMO mismatches as a legacy behavior from Fallout 3's GECK. "
               "This override will be loaded as %5."
            ).arg(overridden_form).arg(overridden_file).arg(overriding_form).arg(overriding_file).arg(form_signature_to_string(&casted->overridden_form.stub));
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_has_multiple_records_in_a_file*>(&warning)) {
            bool has_current_global_id = casted->current_form_ids.global.has_value();

            QString name     = QString::fromStdString(casted->setting_name);
            QString prior_id = editor_helpers::form_id_to_string(casted->last_seen_form_id);
            QString local_id = editor_helpers::form_id_to_string(casted->current_form_ids.local);
            
            if (has_current_global_id) {
               auto global_id = editor_helpers::form_id_to_string(casted->current_form_ids.global.value());
               return QObject::tr(
                  "Found a game setting record (GMST) for setting \"%1\" with form ID %2, but this setting already has "
                  "at least one other record in the same file. The last seen record had form ID %3. When DovahKit "
                  "finishes loading all files, this setting will use whatever form ID we last saw it with by then."
               ).arg(name).arg(global_id).arg(prior_id);
            }
            return QObject::tr(
               "Found a game setting record (GMST) for setting \"%1\" with out-of-bounds or otherwise invalid form "
               "ID %2, but this setting already has at least one other record in the same file. The last seen record "
               "had form ID %3. When DovahKit finishes loading all files, this setting will use whatever form ID we "
               "last saw it with by then."
            ).arg(name).arg(local_id).arg(prior_id);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_name_is_unrecognized*>(&warning)) {
            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->form_ids.local);

            if (auto& global_id_opt = casted->form_ids.global; global_id_opt.has_value()) {
               auto global_id = editor_helpers::form_id_to_string(global_id_opt.value());
               return QObject::tr(
                  "Game setting \"%1\" (defined by a GMST form with ID %2) has an unrecognized name."
               ).arg(name).arg(global_id);
            }
            return QObject::tr(
               "Game setting \"%1\" (defined by a GMST form with file-local form ID %2) has an unrecognized name."
            ).arg(name).arg(local_id);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_overrides_a_real_form*>(&warning)) {
            QString name      = QString::fromStdString(casted->setting_name);
            auto    form      = form_identifiers_to_string(&casted->overridden_form);
            auto    form_file = QString::fromStdString(casted->overridden_file);

            return QObject::tr(
               "A definition for game setting \"%1\" attempts to override %2 (defined in file %3). This definition "
               "will not be loaded."
            ).arg(name).arg(form).arg(form_file);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_record_has_bad_form_id*>(&warning)) {
            bool no_name              = casted->setting_name.empty();
            bool id_failed_to_resolve = !casted->form_ids.global.has_value();
            bool id_is_none           = casted->form_ids.global.value_or(1) == 0;

            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->form_ids.local);
            QString global_id;
            if (!id_failed_to_resolve) {
               global_id = editor_helpers::form_id_to_string(casted->form_ids.global.value());
            }
            
            if (id_failed_to_resolve) {
               return QObject::tr(
                  "A game setting record (GMST) for setting \"%1\" has a bad form ID (local ID %2). This is incorrect, "
                  "but the setting will still load properly."
               ).arg(name).arg(local_id);
            } else if (id_is_none) {
               return QObject::tr(
                  "A game setting (GMST) for setting \"%1\" has a local form ID (%2) that resolves to NONE (00000000). "
                  "This is incorrect, but the setting will still load properly."
               ).arg(name).arg(local_id);
            } else {
               return QObject::tr(
                  "There's something wrong with the form ID for a game setting record (GMST) for setting \"%1\". The "
                  "local ID was %2 and the global ID was %3. The setting will still load properly."
               ).arg(name).arg(local_id).arg(global_id);
            }
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_record_has_no_name*>(&warning)) {
            if (!casted->form_ids.global.has_value()) {
               QString local_id = editor_helpers::form_id_to_string(casted->form_ids.local);
               return QObject::tr(
                  "A game setting record (GMST) has no name (i.e. no EDID subrecord or an empty EDID subrecord), "
                  "so neither Skyrim, the Creation Kit, nor DovahKit will load it. The record in question also "
                  "had an out-of-bounds or otherwise invalid form ID (%1)."
               ).arg(local_id);
            }

            QString global_id = editor_helpers::form_id_to_string(casted->form_ids.global.value());
            return QObject::tr(
               "The game setting record (GMST) with form ID %1 has no name (i.e. no EDID subrecord or an empty "
               "EDID subrecord), so neither Skyrim, the Creation Kit, nor DovahKit will load it."
            ).arg(global_id);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_record_has_unrecognized_subrecord*>(&warning)) {
            QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);

            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->form_ids.local);
            QString global_id;
            if (auto& id = casted->form_ids.global; id.has_value()) {
               global_id = editor_helpers::form_id_to_string(id.value());
            }

            if (name.isEmpty()) {
               if (!global_id.isEmpty()) {
                  return QObject::tr(
                     "A game setting record (GMST) with no editor ID and form ID %1 contained unrecognized "
                     "subrecord %2."
                  ).arg(global_id).arg(subrecord);
               }
               return QObject::tr(
                  "A game setting record (GMST) with no editor ID and file-local form ID %1 contained unrecognized "
                  "subrecord %2."
               ).arg(local_id).arg(subrecord);
            }
            if (!global_id.isEmpty()) {
               return QObject::tr(
                  "A game setting record (GMST) for setting \"%1\" with file-local form ID %2 contained unrecognized "
                  "subrecord %3."
               ).arg(name).arg(local_id).arg(subrecord);
            }
            return QObject::tr(
               "A game setting record (GMST) for setting \"%1\" with form ID %2 contained unrecognized "
               "subrecord %3."
            ).arg(name).arg(global_id).arg(subrecord);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_record_is_misordered*>(&warning)) {
            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->record.local_id);
            QString global_id;
            if (auto& id = casted->record.global_id; id.has_value()) {
               global_id = editor_helpers::form_id_to_string(id.value());
            }

            if (name.isEmpty()) {
               if (!global_id.isEmpty()) {
                  return QObject::tr(
                     "The game setting record (GMST) with form ID %1 has its subrecords in the wrong order. EDID must come "
                     "first, and DATA must come somewhere after EDID."
                  ).arg(global_id);
               }
               return QObject::tr(
                  "The game setting record (GMST) with file-local form ID %1 has its subrecords in the wrong order. EDID must "
                  "come first, and DATA must come somewhere after EDID."
               ).arg(local_id);
            }
            if (!global_id.isEmpty()) {
               return QObject::tr(
                  "The game setting record (GMST) for setting \"%1\" with file-local form ID %2 has its subrecords in the "
                  "wrong order. EDID must come first, and DATA must come somewhere after EDID."
               ).arg(name).arg(local_id);
            }
            return QObject::tr(
               "The game setting record (GMST) for setting \"%1\" with form ID %2 has its subrecords in the wrong order. "
               "EDID must come first, and DATA must come somewhere after EDID."
            ).arg(name).arg(global_id);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_value_has_extra_content*>(&warning)) {
            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->record.local_id);
            QString global_id;
            if (auto& id = casted->record.global_id; id.has_value()) {
               global_id = editor_helpers::form_id_to_string(id.value());
            }

            if (name.isEmpty()) {
               if (!global_id.isEmpty()) {
                  return QObject::tr(
                     "A game setting record (GMST) with no editor ID and form ID %1 contained %2 extra bytes at the "
                     "end of its DATA subrecord (%3 bytes expected; %4 total)."
                  ).arg(global_id).arg(casted->data_size - casted->expected_size).arg(casted->expected_size).arg(casted->data_size);
               }
               return QObject::tr(
                  "A game setting record (GMST) with no editor ID and file-local form ID %1 contained %2 extra bytes "
                  "at the end of its DATA subrecord (%3 bytes expected; %4 total)."
               ).arg(local_id).arg(casted->data_size - casted->expected_size).arg(casted->expected_size).arg(casted->data_size);
            }
            if (!global_id.isEmpty()) {
               return QObject::tr(
                  "A game setting record (GMST) for setting \"%1\" with file-local form ID %2 contained %3 extra bytes "
                  "at the end of its DATA subrecord (%4 bytes expected; %5 total)."
               ).arg(name).arg(local_id).arg(casted->data_size - casted->expected_size).arg(casted->expected_size).arg(casted->data_size);
            }
            return QObject::tr(
               "A game setting record (GMST) for setting \"%1\" with form ID %2 contained %3 extra bytes at the "
               "end of its DATA subrecord (%4 bytes expected; %5 total)."
            ).arg(name).arg(global_id).arg(casted->data_size - casted->expected_size).arg(casted->expected_size).arg(casted->data_size);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_value_type_unknown*>(&warning)) {
            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->record.local_id);
            QString global_id;
            if (auto& id = casted->record.global_id; id.has_value()) {
               global_id = editor_helpers::form_id_to_string(id.value());
            }

            if (!name.isEmpty()) {
               if (!global_id.isEmpty()) {
                  return QObject::tr(
                     "Unable to load the value of a game setting record (GMST) for setting \"%1\" with file-local form ID %2. "
                     "The game setting's value type (bool/float/int/etc.) is unknown."
                  ).arg(name).arg(global_id);
               }
               return QObject::tr(
                  "Unable to load the value of a game setting record (GMST) for setting \"%1\" with form ID %2. The game setting's "
                  "value type (bool/float/int/etc.) is unknown."
               ).arg(name).arg(local_id);
            }
            //
            // We should never emit this error for nameless settings, because we know a priori that we can't identify 
            // their value type. The value type is based on the first letter of the setting name (i.e. Hungarian 
            // notation).
            //
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::game_setting_value_unreadable*>(&warning)) {
            QString name     = QString::fromStdString(casted->setting_name);
            QString local_id = editor_helpers::form_id_to_string(casted->record.local_id);
            QString global_id;
            if (auto& id = casted->record.global_id; id.has_value()) {
               global_id = editor_helpers::form_id_to_string(id.value());
            }

            QString explanation = QObject::tr("The data is truncated or malformed.");
            if (!casted->subrecord_is_present)
               explanation = QObject::tr("The DATA subrecord is missing.");

            if (name.isEmpty()) {
               if (!global_id.isEmpty()) {
                  return QObject::tr(
                     "Failed to load the value of the game setting record (GMST) with form ID %1. %2"
                  ).arg(global_id).arg(explanation);
               }
               return QObject::tr(
                  "Failed to load the value of the game setting record (GMST) with file-local form ID %1. %2"
               ).arg(local_id).arg(explanation);
            }
            if (!global_id.isEmpty()) {
               return QObject::tr(
                  "Failed to load the value of the game setting record (GMST) for setting \"%1\" with file-local form ID %2. %3"
               ).arg(name).arg(local_id).arg(explanation);
            }
            return QObject::tr(
               "Failed to load the value of the game setting record (GMST) for setting \"%1\" with form ID %2. %3"
            ).arg(name).arg(global_id).arg(explanation);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::partial_info_override_has_different_parent*>(&warning)) {
            auto subject = QString("[%1:%2]")
               .arg(editor_helpers::form_type_name_to_string(casted->record.form_type))
               .arg(editor_helpers::form_id_to_string(casted->record.global_id));

            auto override_file = QString::fromStdString(casted->source_file);
            if (override_file.isEmpty())
               override_file = QObject::tr("<unknown file>");

            auto original_file = QString::fromStdString(casted->source_file_for_overridden);
            if (original_file.isEmpty())
               original_file = QObject::tr("<unknown file>");

            auto parent_prior = form_identifiers_to_string(casted->parent_of_overridden);
            auto parent_after = form_identifiers_to_string(casted->parent_of_overriding);

            return QObject::tr(
               "TopicInfo %1, originally defined in file %3, has an override in file %2 that is flagged as partial "
               "and that moves the TopicInfo from Topic %4 to Topic %5. Skyrim does not properly handle partial-flagged "
               "TopicInfo overrides that re-parent the TopicInfo; depending on the precise circumstances under which "
               "this override is loaded, Skyrim may inadvertently associate the TopicInfo with multiple Topics, may "
               "desynchronize the TopicInfo such that it thinks it's inside of a different Topic than the one it's "
               "actually in, or may discard an unrelated TopicInfo from the Topic that this TopicInfo has been moved to. "
               "Though the effect that these issues have on game stability is not known as of this writing, this problem "
               "should be considered unsafe. DovahKit will interpret this data by re-parenting the TopicInfo as normal, "
               "but this is just the way the data is interpreted (in lieu of DovahKit actually trying to mimic the game's "
               "utter confusion) and is not an attempt at repairing the data. If circumstances allow, you should remove "
               "the \"partial\" flag from this override using xEdit or a similar tool."
            ).arg(subject).arg(override_file).arg(original_file).arg(parent_prior).arg(parent_after);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::record_found_in_wrong_top_level_group*>(&warning)) {
            QString expected = cobb::qt::four_cc_to_string(casted->top_level_group_label);
            QString found    = cobb::qt::four_cc_to_string(casted->record.signature);

            if (casted->record.global_id.has_value()) {
               QString form_id = form_id_to_string(casted->record.global_id.value());
               return QObject::tr(
                  "Found record [%1:%2] in the record group for %3 records."
               ).arg(found).arg(form_id).arg(expected);
            }

            QString form_id = form_id_to_string(casted->record.local_id);
            return QObject::tr(
               "Found a %1 record with file-local form ID %2 inside the record group for %3 records."
            ).arg(found).arg(form_id).arg(expected);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::singleton_form_is_redundantly_defined*>(&warning)) {
            auto subject = QString("[%1:%2]")
               .arg(editor_helpers::form_type_name_to_string(casted->record.form_type))
               .arg(editor_helpers::form_id_to_string(casted->record.global_id));

            auto prior_id = editor_helpers::form_id_to_string(casted->previous_form_id);

            auto file = QString::fromStdString(casted->source_file);
            if (file.isEmpty())
               file = QObject::tr("<unknown file>");

            return QObject::tr(
               "Form %2 is a \"singleton\" form: the game only allows one form of this type to exist, and treats all "
               "records of this type as overrides of that one single form, regardless of their form ID. However, this "
               "particular record is redundant: it exists in file %1, which already defined this singleton form using "
               "form ID %3."
            ).arg(file).arg(subject).arg(prior_id);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::the_game_doesnt_load_new_actor_value_infos*>(&warning)) {
            auto subject = QString("[%1:%2]%3")
               .arg(editor_helpers::form_signature_to_string(&casted->subject))
               .arg(editor_helpers::form_id_to_string(casted->subject_form_id))
               .arg(casted->subject.get_editor_id());

            return QObject::tr(
               "Actor value info %1 is not an override; DovahKit will load it, but Skyrim will not. "
               "Skyrim only loads overrides of the hardcoded actor value infos."
            ).arg(subject);
         }
      #pragma endregion

      #pragma region form load warnings
         if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::form_reference_type_mismatch*>(&warning)) {
            QString subject   = form_identifiers_to_string(&casted->subject);
            QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
            QString target    = form_identifiers_to_string(&casted->target);
            
            QString desired;
            switch (casted->desired.size()) {
               case 0:
                  desired = QString("some type other than the one it happens to refer to (no error information available; tell DovahKit's developer about this)");
                  break;
               case 1:
                  desired = QString("type ") + form_type_name_to_string(casted->desired[0]);
                  break;
               case 2:
                  desired = QString("types %1 or %2")
                     .arg(form_type_name_to_string(casted->desired[0]))
                     .arg(form_type_name_to_string(casted->desired[1]))
                  ;
                  break;
               default:
                  desired = QString("types ");
                  {
                     auto& list = casted->desired;
                     auto  size = list.size();
                     for (size_t i = 0; i < size; ++i) {
                        desired += form_type_name_to_string(casted->desired[i]);
                        if (i + 1 < size) {
                           desired += ", ";
                           if (i + 2 == size)
                              desired += "or ";
                        }
                     }
                  }
                  break;
            }
            
            // Messages for special cases
            if (casted->metadata.nth_reference.has_value()) {
               const auto nth_reference = casted->metadata.nth_reference.value();

               switch (casted->subrecord_signature) { // form components
                  case 'DSTD': // destruction stage
                     return QObject::tr(
                        "Destruction stage %2 in %1 referred to form %4 when it was expected to refer to a form of %3.",
                        disambig
                     ).arg(subject).arg(nth_reference).arg(desired).arg(target);

                  case 'KWDA': // keyword list entry
                     return QObject::tr(
                        "%1 contains a keyword list whose %2th entry referred to form %4 when it was expected to refer to a form of %3.",
                        disambig
                     ).arg(subject).arg(nth_reference).arg(desired).arg(target);

                  case 'XCLR': // extra-data: cell region list entry
                     return QObject::tr(
                        "The region list for cell %1 contains an entry at position %2 which referred to form %4 when it was expected to refer to a form of %3.",
                        disambig
                     ).arg(subject).arg(nth_reference).arg(desired).arg(target);
               }
               switch (casted->subject.form_type) {
                  case dovah::form_type::shout:
                     switch (casted->subrecord_signature) {
                        case 'SNAM':
                           return QObject::tr(
                              "The %2th word in Shout %1 referred to form %4 when it was expected to refer to a form of %3.",
                              disambig
                           ).arg(subject).arg(nth_reference).arg(desired).arg(target);
                     }
                     break;
               }
            }

            return QObject::tr(
               "%1 contained subrecord %2, expected to refer to a form of %3; it instead referred to %4.",
               disambig
            ).arg(subject).arg(subrecord).arg(desired).arg(target);
         }
         if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::unrecognized_subrecord*>(&warning)) {
            QString subject   = form_identifiers_to_string(&casted->subject);
            QString subrecord = cobb::qt::four_cc_to_string(casted->signature);
            //
            return QObject::tr(
               "%1 contained unrecognized subrecord %2.",
               disambig
            ).arg(subject).arg(subrecord);
         }
         //
         // Specific warnings:
         //
         #pragma region by form component
            #pragma region attack data
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::attack_data::expected_event_subrecord*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "An ATKR subrecord in form %1 was followed by a subrecord with signature %2, instead of the "
                     "expected ATKE subrecord. That subrecord will be misinterpreted as an ATKE subrecord.",
                     disambig
                  ).arg(subject).arg(subrecord);
               }
            #pragma endregion
            #pragma region container
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::container::item_has_bad_owner_form_type*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString owner   = form_identifiers_to_string(&casted->item_owner);
                  //
                  return QObject::tr(
                     "Form %1 has a malformed entry in its inventory: the item's owner is form %2, which is not an "
                     "ActorBase or Faction. Because the owner is of an invalid type, the additional four-byte value "
                     "paired with it is also of an invalid type and will be mishandled by the editor.",
                     disambig
                  ).arg(subject).arg(owner);
               }
            #pragma endregion
            #pragma region destruction data
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::destruction::stage_serialized_index_out_of_bounds*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Destruction data for form %1 lists itself as containing %3 stages, but has a stage requesting "
                     "slot %2. That stage has been discarded.",
                     disambig
                  ).arg(subject).arg(casted->serialized_stage_index).arg(casted->stage_count);
               }
            #pragma endregion
            #pragma region extra data
               #pragma region room_ref_data
                  if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::extra_data::room_ref_data_insufficient_rooms*>(&warning)) {
                     QString subject = form_identifiers_to_string(&casted->subject);
                     //
                     return QObject::tr(
                        "The room-ref-data for %1 declared that it would have %2 linked rooms, but only had %3.",
                        disambig
                     ).arg(subject).arg(casted->expected).arg(casted->found);
                  }
                  if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::extra_data::room_ref_data_swallowed_subrecord*>(&warning)) {
                     using expected_field_type = std::decay_t<decltype(*casted)>::expected_field_type;

                     QString subject   = form_identifiers_to_string(&casted->subject);
                     QString expected;
                     QString signature = cobb::qt::four_cc_to_string(casted->signature_seen);
                     QString ordinal;

                     switch (casted->expected_field) {
                        case expected_field_type::imagespace:
                           expected = QObject::tr("INAM (imagespace");
                           break;
                        case expected_field_type::lighting_template:
                           expected = QObject::tr("LNAM (lighting template");
                           break;
                        case expected_field_type::linked_room:
                           expected = QObject::tr("XLRM (linked room");
                           break;
                     }

                     if (casted->expected_field == expected_field_type::linked_room) {
                        if (casted->signature_seen == 'XRMR') {
                           return QObject::tr(
                              "The room-ref-data for %1 is malformed. Subrecord %2 was expected, but subrecord %3 was found.",
                              disambig
                           ).arg(subject).arg(expected).arg(signature);
                        }

                        if (casted->is_nth_linked_room.has_value()) {
                           ordinal = QString::number(casted->is_nth_linked_room.value());
                        } else {
                           ordinal = QObject::tr("?", "XLRM subrecord swallow: missing linked room index");
                        }
                        return QObject::tr(
                           "The room-ref-data for %1 is malformed. Subrecord %2 was expected, but subrecord %3 was found. The game "
                           "doesn't double-check the signature, so it will blindly swallow the found subrecord for use as linked "
                           "room #%4.",
                           disambig
                        ).arg(subject).arg(expected).arg(signature).arg(ordinal);
                     }

                     return QObject::tr(
                        "The room-ref-data for %1 is malformed. Subrecord %2 was expected, but subrecord %3 was found. The game "
                        "doesn't double-check the signature, so it will blindly swallow the found subrecord.",
                        disambig
                     ).arg(subject).arg(expected).arg(signature);
                  }
               #pragma endregion
            #pragma endregion
            #pragma region idle collection
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::idle_collection::incorrect_idle_count*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Form %1 claims to have %2 idles, but its data supplies %3 idles. The game will detect "
                     "this discrepancy and refuse to load the animation list.",
                     disambig
                  ).arg(subject).arg(casted->count_expected).arg(casted->count_seen);
               }
            #pragma endregion
            #pragma region leveled list
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::leveled_list::leading_coed_bleedthrough*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "The leveled list data in form %1 contained a COED (Container Object Extra Data) subrecord before any "
                     "of its LVLO (Leveled Object) subrecords. This data will not load properly in-game due to a bug in how "
                     "some leveled lists are loaded. When the game loads this form, it may accidentally apply this wayward "
                     "COED subrecord to the last entry in the previous leveled list that was loaded, if that leveled list is "
                     "of the same type and if its last entry had no COED (i.e. owner or health changes). DovahKit will instead "
                     "skip loading this wayward COED subrecord, which seems to have been what Bethesda intended.",
                     disambig
                  ).arg(subject);
               }
            #pragma endregion
            #pragma region magic effect list
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::magic_effect_list::expected_effect_item_subrecord*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "Form %1 contains an EFID subrecord followed by a %2 subrecord. The game only expects to see EFIT or "
                     "CTDA here, and will misread the %2 subrecord as if it were EFIT.",
                     disambig
                  ).arg(subject).arg(subrecord);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::magic_effect_list::misplaced_effect_item_subrecord*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Form %1 contains an EFIT subrecord that isn't placed immediately after an EFID subrecord. The game "
                     "may fail to load this subrecord properly.",
                     disambig
                  ).arg(subject);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::magic_effect_list::too_many_effects*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);

                  QString format;
                  if (casted->effects_allowed == casted->hard_maximum) {
                     format = QObject::tr(
                        "Form %1 has too many magic effects. It contains %2 effects, but the game will only load up to %4 "
                        "effects.",
                        disambig
                     );
                  } else if (casted->effects_contained > casted->hard_maximum) {
                     format = QObject::tr(
                        "Form %1 has too many magic effects. It contains %2 effects, but this form type only allows %3 "
                        "effects, and the game itself won't load more than %4 effects regardless of form type.",
                        disambig
                     );
                  } else {
                     format = QObject::tr(
                        "Form %1 has too many magic effects. It contains %2 effects, but this form type only allows %3 "
                        "effects.",
                        disambig
                     );
                  }
                  return format.arg(subject).arg(casted->effects_contained).arg(casted->effects_allowed).arg(casted->hard_maximum);
               }
            #pragma endregion
            #pragma region package event dialogue
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_component::package_event_dialogue::unrecognized_subrecord*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString subrecord = cobb::qt::four_cc_to_string(casted->signature);
                  //
                  return QObject::tr(
                     "A piece of package event dialogue data in form %1 contained at least one unrecognized subrecord "
                     "with signature %2. This could potentially be a serious problem, as package event dialogue data "
                     "will blindly consume subrecords until it finds one it expects.",
                     disambig
                  ).arg(subject).arg(subrecord);
               }
            #pragma endregion
         #pragma endregion
         #pragma region by form type
            #pragma region actor value info
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::actor_value_info::unterminated_perk_tree_node*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString perk    = form_identifiers_to_string(casted->perk);
                  return QObject::tr(
                     "Actor Value Info %1 contains an unterminated perk tree node (for %2), i.e. a node missing "
                     "its INAM subrecord."
                  ).arg(subject).arg(perk);
               }
            #pragma endregion
            #pragma region cell
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::cell::cell_type_not_yet_known*>(&warning)) {
                  QString form      = form_identifiers_to_string(&casted->subject);
                  QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "%1 contained subrecord %2, which is handled differently for interior and exterior cells; "
                     "however, the cell's DATA subrecord has not yet appeared, so the cell will default to being "
                     "an exterior.",
                     disambig
                  ).arg(form).arg(subrecord);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::cell::data_for_wrong_cell_type*>(&warning)) {
                  QString form      = form_identifiers_to_string(&casted->subject);
                  QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  if (casted->current_cell_type == form_load_warnings::by_type::cell::data_for_wrong_cell_type::cell_type::interior) {
                     return QObject::tr(
                        "%1 contained subrecord %2, which is only valid for exterior cells. This cell is an interior.",
                        disambig
                     ).arg(form).arg(subrecord);
                  }
                  return QObject::tr(
                     "%1 contained subrecord %2, which is only valid for interior cells. This cell is an exterior.",
                     disambig
                  ).arg(form).arg(subrecord);
               }
            #pragma endregion
            #pragma region dialogue branch
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::dialogue_branch::mishandled_owning_quest_id*>(&warning)) {
                  QString form     = form_identifiers_to_string(&casted->subject);
                  QString intended = form_identifiers_to_string(&casted->intended_owning_quest);
                  //
                  return QObject::tr(
                     "Dialogue branch %1 is meant to use %2 as its owning quest. However, The game will not load this "
                     "properly. The game has to convert file-local form IDs to global form IDs, but in this very specific "
                     "case, Skyrim performs the conversion twice by accident. Some form IDs will be completely trashed as "
                     "a result, and the form ID of this quest is one of those. DovahKit will not attempt to replicate this "
                     "error (we'll load the form ID properly) but you should be aware that this value won't work. If a "
                     "quest's local and global form IDs aren't the same, then it can't be safely used as a dialogue branch's "
                     "owning quest.",
                     disambig
                  ).arg(form).arg(intended);
               }
            #pragma endregion
            #pragma region faction
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::faction::interrupt_override_target_not_in_a_package*>(&warning)) {
                  QString format;
                  if (casted->subject.form_type == dovah::form_type::faction) {
                     format = QObject::tr(
                        "Faction %1 specifies an invalid vendor location -- specifically, one which only makes sense "
                        "for Packages that can serve as interrupt overrides.",
                        disambig
                     );
                  } else {
                     format = QObject::tr(
                        "Form %1 is not a Package, but has data which specifies a world location, and the location "
                        "in question only makes sense for Packages that can serve as interrupt overrides.",
                        disambig
                     );
                  }
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return format.arg(subject);
               }
            #pragma endregion
            #pragma region footstep set
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::footstep_set::footstep_count_mismatch*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  size_t  total_expected = 0;
                  for (auto expected : casted->expected)
                     total_expected += expected;
                  //
                  return QObject::tr(
                     "Footstep Set %1 appears to contain %2 footsteps. It expected a total of %3 footsteps.",
                     disambig
                  ).arg(subject).arg(casted->total_found).arg(total_expected);
               }
            #pragma endregion
            #pragma region landscape
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::landscape::excess_layers_per_quad*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString texture = form_identifiers_to_string(casted->texture);
                  //
                  return QObject::tr(
                     "Landspace %1 quad %2 attempted to specify land-texture %4 for layer #%3, but a quad can only have "
                     "six layers. In-game, this layer will overwrite layer #6.",
                     disambig
                  ).arg(subject).arg(casted->quad).arg(casted->layer).arg(texture);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::landscape::invalid_quad_for_land_texture*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  if (casted->type == form_load_warnings::by_type::landscape::invalid_quad_for_land_texture::texture_type::default_texture) {
                     return QObject::tr(
                        "Form %1 attempted to assign a default land texture for landscape quad index %2, but quad indices "
                        "must be between 0 and 3 inclusive.",
                        disambig
                     ).arg(subject).arg(casted->quad);
                  }

                  QString layer = "?";
                  if (casted->layer.has_value()) [[likely]]
                     layer = QString::number(casted->layer.value());

                  return QObject::tr(
                     "Form %1 attempted to blend a land texture onto quad index %2, layer %3, but quad indices must be between 0 and 3 "
                     "inclusive.",
                     disambig
                  ).arg(subject).arg(casted->quad).arg(layer);
               }
            #pragma endregion
            #pragma region head part
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::head_part::invalid_morph_type*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "HeadPart %1 contained a morph with an invalid type %2.",
                     disambig
                  ).arg(subject).arg(casted->morph_type);
               }
            #pragma endregion
            #pragma region idle
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::idle::event_name_too_long*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "The event name for Idle Animation %1 is too long: it's %2 bytes long, but the Creation Kit "
                     "and the game will only load the first %3 bytes.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::idle::filename_too_long*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "The filename for Idle Animation %1 is too long: it's %2 bytes long, but the Creation Kit "
                     "and the game will only load the first %3 bytes.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
            #pragma endregion
            #pragma region impact data set
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::impact_data_set::mapping_missing_data*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString matt    = form_identifiers_to_string(casted->material_type);
                  QString impact  = form_identifiers_to_string(casted->impact_data);
                  //
                  if (casted->material_type) {
                     return QObject::tr(
                        "Mapping #%2 in Impact Data Set %1 is missing data: it maps material type %3 to no impact data.",
                        disambig
                     ).arg(subject).arg(casted->which).arg(matt);
                  } else if (casted->impact_data) {
                     return QObject::tr(
                        "Mapping #%2 in Impact Data Set %1 is missing data: it maps a missing material type to impact data %3.",
                        disambig
                     ).arg(subject).arg(casted->which).arg(impact);
                  } else {
                     return QObject::tr(
                        "Mapping #%2 in Impact Data Set %1 is empty.",
                        disambig
                     ).arg(subject).arg(casted->which);
                  }
               }
            #pragma endregion
            #pragma region magic_effect
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::magic_effect::counters_itself*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);

                  return QObject::tr(
                     "Magic Effect %1 lists itself in its Counter Effects list. This is not the intended way to make a Magic "
                     "Effect only apply once to a given target; prefer the No Recast flag.",
                     disambig
                  ).arg(subject);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::magic_effect::invalid_actor_value_index*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString which = QObject::tr("?");
                  switch (casted->which) {
                     using enum form_load_warnings::by_type::magic_effect::invalid_actor_value_index::which_type;
                     case magic_skill:
                        which = QObject::tr("Magic Skill");
                        break;
                     case resist:
                        which = QObject::tr("Resistance");
                        break;
                     case assoc_item_1:
                        which = QObject::tr("Associated Item 1");
                        break;
                     case assoc_item_2:
                        which = QObject::tr("Associated Item 2");
                        break;
                  }

                  return QObject::tr(
                     "Magic Effect %1 specified an invalid actor value index for its %2. The index has been corrected to None.",
                     disambig
                  ).arg(subject).arg(which);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::magic_effect::redundant_sound*>(&warning)) {
                  QString subject    = form_identifiers_to_string(&casted->subject);
                  QString type       = QObject::tr("?");
                  QString descriptor = form_identifiers_to_string(casted->descriptor);
                  switch (casted->type) {
                     using enum form_load_warnings::by_type::magic_effect::redundant_sound::effect_sound_type;
                     case draw_sheathe:
                        type = QObject::tr("Draw/Sheathe", "MagicEffect sound type for form load warnings");
                        break;
                     case charge:
                        type = QObject::tr("Charge", "MagicEffect sound type for form load warnings");
                        break;
                     case ready:
                        type = QObject::tr("Ready", "MagicEffect sound type for form load warnings");
                        break;
                     case release:
                        type = QObject::tr("Release", "MagicEffect sound type for form load warnings");
                        break;
                     case concentration_cast_loop:
                        type = QObject::tr("Cast Loop (Conc.)", "MagicEffect sound type for form load warnings");
                        break;
                     case on_hit:
                        type = QObject::tr("On Hit", "MagicEffect sound type for form load warnings");
                        break;
                  }

                  return QObject::tr(
                     "Magic Effect %1 specifies more than one \"%2\" sound. The game will only use the first loaded sound for each type, so sound %3 will not be used.",
                     disambig
                  ).arg(subject).arg(type).arg(descriptor);
               }
            #pragma endregion
            #pragma region material_type
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::material_type::name_too_long*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "The name for Material Type %1 is too long: it's %2 bytes long, but the Creation Kit "
                     "and the game will only load the first %3 bytes.",
                     disambig
                  ).arg(subject).arg(casted->size).arg(casted->max_serializable_size);
               }
            #pragma endregion
            #pragma region message
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::message::orphaned_conditions*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString format;
                  if (casted->retained) {
                     format = QObject::tr(
                        "Message %1 contains %2 condition(s) not associated with any menu button. A blank "
                        "button has been added for them."
                     );
                  } else {
                     format = QObject::tr(
                        "Message %1 contains %2 condition(s) not associated with any menu button. These "
                        "conditions have been discarded."
                     );
                  }
                  return format.arg(subject).arg(casted->count);
               }
            #pragma endregion
            #pragma region music track
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::music_track::invalid_track_type*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Music track %1 specified an invalid type (%2)."
                  ).arg(subject).arg(casted->seen_type);
               }
            #pragma endregion
            #pragma region note
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::note::non_texture_note_includes_texture_path*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Note %1 is not a texture note, but still supplies a texture path.",
                     disambig
                  ).arg(subject);
               }
            #pragma endregion
            #pragma region package
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::invalid_interrupt_override_target*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "One of the targets in Package %1 refers to an invalid interrupt override target (raw value %2).",
                     disambig
                  ).arg(subject).arg(casted->seen);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::package_data_metadata_belongs_to_none*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Package %1 contains invalid metadata for one of its Package Data. Specifically, it attempts to "
                     "associate metadata (a UNAM+BNAM+PNAM subrecord triplet) with the \"None\" (255) Package Data ID.",
                     disambig
                  ).arg(subject).arg(casted->which);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::package_data_unexpected_value_subrecord*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString data_type = editor::localize::package_data_type(casted->data_type);
                  QString signature = cobb::qt::four_cc_to_string(casted->signature);
                  if (data_type.isEmpty()) {
                     data_type = QObject::tr("?", disambig);
                  }
                  return QObject::tr(
                     "Failed to load the value for a Package Data in Package %1. Package Data #%2 (%3) ran into unexpected subrecord %4.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(data_type).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::paackage_data_unrecognized_typename*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Failed to load a Package Data in Package %1. Package Data #%2 has unrecognized typename %3. If "
                     "the current version of the game and Creation Kit don't recognize this typename either, then they "
                     "will experience cascading errors when trying to load this Package, and probably fail to properly "
                     "load anything within the Package that comes after this Package Data.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(QString::fromStdString(casted->type));
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::package_data_wants_none_as_unique_id*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString data_type = editor::localize::package_data_type(casted->data_type);
                  if (data_type.isEmpty()) {
                     data_type = QObject::tr("?", disambig);
                  }
                  return QObject::tr(
                     "Failed to load the internal ID for a Package Data in Package %1. Package Data #%2 (%3) wants to use \"None\" (255) "
                     "as its unique ID.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(data_type);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::procedure_has_extra_parameters*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Package %1 contains a Procedure with %2 too many parameters.",
                     disambig
                  ).arg(subject).arg(casted->count);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::procedure_missing_required_parameter*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Package %1 contains a Procedure that is missing a required parameter (#%2: %3).",
                     disambig
                  ).arg(subject).arg(casted->param_index).arg(QString::fromStdString(casted->param_name));
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::procedure_typename_unrecognized*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Failed to load a Procedure in Package %1. The Procedure had unrecognized typename %2..",
                     disambig
                  ).arg(subject).arg(QString::fromStdString(casted->type));
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::target_has_an_invalid_object_type*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString format;
                  if (casted->subject.form_type == dovah::form_type::faction) {
                     format = QObject::tr(
                        "Faction %1 specifies an invalid vendor location: it refers to an invalid object type (raw value %2).",
                        disambig
                     );
                  } else {
                     format = QObject::tr(
                        "One of the targets in Package %1 refers to an invalid interrupt override target (raw value %2).",
                        disambig
                     );
                  }
                  return format.arg(subject).arg(casted->seen);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::target_is_exterior_cell*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString cell    = form_identifiers_to_string(&casted->cell);
                  QString format;
                  if (casted->subject.form_type == dovah::form_type::faction) {
                     format = QObject::tr(
                        "Faction %1 specifies an invalid vendor location: it refers to exterior cell %2. Vendor locations "
                        "must be interior cells.",
                        disambig
                     );
                  } else {
                     format = QObject::tr(
                        "One of the targets in Package %1 refers to exterior cell %2. Package targets must be interior cells.",
                        disambig
                     );
                  }
                  return format.arg(subject).arg(cell);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::too_many_package_data*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Package %1 contains %2 Package Data. However, Package Data require a (hidden) unique ID to be used, "
                     "and only 255 unique IDs are possible.",
                     disambig
                  ).arg(subject).arg(casted->count);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::unexpected_subrecord_in_unique_id_list*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString signature = cobb::qt::four_cc_to_string(casted->signature);
                  return QObject::tr(
                     "Package %1 is supposed to contain a list of internal \"unique IDs\" for all of its Package Data. "
                     "These should be a set of UNAM subrecords, one per package data. However, subrecord #%2 was instead "
                     "a %3 subrecord. The game, Creation Kit, and DovahKit will treat this a sif it were a UNAM subrecord "
                     "whose content was the unique ID %2.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::unique_id_has_multiple_metadata*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Package %1 contains multiple sets of metadata (i.e. name and \"public\" flag) associated with "
                     "Package Data Unique ID %2. The last-loaded set will be retained.",
                     disambig
                  ).arg(subject).arg(casted->unique_id);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::unique_id_used_by_multiple_package_data*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Package %1 contains multiple Package Data that are all trying to use the same unique ID (%2).",
                     disambig
                  ).arg(subject).arg(casted->unique_id);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::package::wrong_target_for_interrupt_override*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);

                  QString target;
                  QString required_type;
                  QString actual_type;
                  switch (casted->target) {
                     using enum dovah::packages::interrupt_override_target;
                     case threat_to_spectate:
                        target = QObject::tr("Threat Ref", disambig);
                        break;
                     case corpse_to_observe:
                        target = QObject::tr("Dead Ref", disambig);
                        break;
                     case ref_to_guard:
                        target = QObject::tr("Guarded Ref", disambig);
                        break;
                     case trespasser:
                        target = QObject::tr("Trespasser Ref", disambig);
                        break;
                     case combat_target:
                        target = QObject::tr("Target Ref", disambig);
                        break;
                  }

                  auto _stringify_interrupt_override_type = [](dovah::packages::interrupt_override_type type) -> QString {
                     switch (type) {
                        using enum dovah::packages::interrupt_override_type;
                        case none:
                           return QObject::tr("None", disambig);
                        case spectator:
                           return QObject::tr("Spectator", disambig);
                        case observe_dead:
                           return QObject::tr("Observe Corpse", disambig);
                        case guard_warn:
                           return QObject::tr("Guard Warn", disambig);
                        case combat:
                           return QObject::tr("Combat", disambig);
                     }
                     return QObject::tr("?", disambig);
                  };
                  required_type = _stringify_interrupt_override_type(casted->required_type);
                  actual_type   = _stringify_interrupt_override_type(casted->actual_type);
                  //
                  if (casted->actual_type == dovah::packages::interrupt_override_type::none) {
                     return QObject::tr(
                        "One of the targets in Package %1 is \"%2\", specific to the \"%3\" interrupt override type. "
                        "However, this package is not an interrupt override.",
                        disambig
                     ).arg(subject).arg(target).arg(required_type).arg(actual_type);
                  } else {
                     return QObject::tr(
                        "One of the targets in Package %1 is \"%2\", specific to the \"%3\" interrupt override type. "
                        "However, this package is for the \"%4\" interrupt override.",
                        disambig
                     ).arg(subject).arg(target).arg(required_type).arg(actual_type);
                  }
               }
            #pragma endregion
            #pragma region perk
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::effect_entry_point_has_mismatched_data_for_type*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString signature = cobb::qt::four_cc_to_string(casted->subrecord);
                  return  QObject::tr(
                     "Perk %1 effect %2 contains subrecord %3, which should not be present given the "
                     "chosen entry point and options."
                  ).arg(subject).arg(casted->which_effect).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::effect_entry_point_has_mismatched_type_for_function*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString function;
                  QString type_seen;
                  QString type_expected;

                  switch (casted->function) {
                     case dovah::entry_point_function::absolute_value:
                        function = QObject::tr("Absolute Value", "entry point function");
                        break;
                     case dovah::entry_point_function::add_activate_choice:
                        function = QObject::tr("Add Activate Choice", "entry point function");
                        break;
                     case dovah::entry_point_function::add_actor_value_mult:
                        function = QObject::tr("Add Actor Value Mult", "entry point function");
                        break;
                     case dovah::entry_point_function::add_leveled_list:
                        function = QObject::tr("Add Leveled List", "entry point function");
                        break;
                     case dovah::entry_point_function::add_range_to_value:
                        function = QObject::tr("Add Range to Value", "entry point function");
                        break;
                     case dovah::entry_point_function::add_value:
                        function = QObject::tr("Add Value", "entry point function");
                        break;
                     case dovah::entry_point_function::multiply_actor_value_mult:
                        function = QObject::tr("Absolute Value", "entry point function");
                        break;
                     case dovah::entry_point_function::multiply_one_plus_av_mult:
                        function = QObject::tr("Multiply 1 + Actor Value Mult", "entry point function");
                        break;
                     case dovah::entry_point_function::multiply_value:
                        function = QObject::tr("Multiply Value", "entry point function");
                        break;
                     case dovah::entry_point_function::negative_absolute_value:
                        function = QObject::tr("Negative Absolute Value", "entry point function");
                        break;
                     case dovah::entry_point_function::none:
                        function = QObject::tr("None", "entry point function");
                        break;
                     case dovah::entry_point_function::select_spell:
                        function = QObject::tr("Select Spell", "entry point function");
                        break;
                     case dovah::entry_point_function::select_text:
                        function = QObject::tr("Select Text", "entry point function");
                        break;
                     case dovah::entry_point_function::set_text:
                        function = QObject::tr("Set Text", "entry point function");
                        break;
                     case dovah::entry_point_function::set_to_actor_value_mult:
                        function = QObject::tr("Set to Actor Value Mult", "entry point function");
                        break;
                     case dovah::entry_point_function::set_value:
                        function = QObject::tr("Set Value", "entry point function");
                        break;
                  }

                  auto _type_to_string = [](dovah::entry_point_function_type t) -> QString {
                     switch (t) {
                        case dovah::entry_point_function_type::activate_choice:
                           return QObject::tr("Activate Choice", "entry point function data type");
                        case dovah::entry_point_function_type::animation_graph_var:
                           return QObject::tr("Animation Graph Variable Name", "entry point function data type");
                        case dovah::entry_point_function_type::leveled_item:
                           return QObject::tr("Leveled Item", "entry point function data type");
                        case dovah::entry_point_function_type::localized_string:
                           return QObject::tr("Localized String", "entry point function data type");
                        case dovah::entry_point_function_type::none:
                           return QObject::tr("None", "entry point function data type");
                        case dovah::entry_point_function_type::one_float:
                           return QObject::tr("One Float", "entry point function data type");
                        case dovah::entry_point_function_type::spell:
                           return QObject::tr("Spell", "entry point function data type");
                        case dovah::entry_point_function_type::two_floats:
                           return QObject::tr("Two Floats", "entry point function data type");
                     }
                     return QString::number((uint32_t)t);
                  };
                  type_expected = _type_to_string(casted->type_expected);
                  type_seen     = _type_to_string(casted->type_seen);

                  return  QObject::tr(
                     "Perk %1 effect %2 uses entry point function %3. The function parameter type should "
                     "be %4, but is instead %5."
                  ).arg(subject).arg(casted->which_effect).arg(function).arg(type_expected).arg(type_seen);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::effect_entry_point_params_specify_an_invalid_av*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return  QObject::tr(
                     "Perk %1 effect %2 specifies an invalid Actor Value index (%3)."
                  ).arg(subject).arg(casted->which_effect).arg(casted->av_seen);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::effect_header_has_invalid_size*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return  QObject::tr(
                     "Perk %1 effect %2 has a header (PRKE subrecord) with an incorrect size in bytes. "
                     "The header is size %3, but the game will only accept size %4."
                  ).arg(subject).arg(casted->which_effect).arg(casted->size_seen).arg(casted->size_expected);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::entry_point_data_for_effect_of_other_type*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString signature = cobb::qt::four_cc_to_string(casted->subrecord);
                  return  QObject::tr(
                     "Perk %1 effect %2 contains subrecord %3, which should not be present given the "
                     "chosen effect type."
                  ).arg(subject).arg(casted->which_effect).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::invalid_effect_type*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return  QObject::tr(
                     "Perk %1 effect %2 uses invalid effect type %3."
                  ).arg(subject).arg(casted->which_effect).arg(casted->seen_type);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::orphaned_effect_subrecord*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString signature = cobb::qt::four_cc_to_string(casted->subrecord);
                  return  QObject::tr(
                     "Perk %1 contains an instance of subrecord %2 not associated with any perk effect."
                  ).arg(subject).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::orphaned_entry_point_conditions*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return  QObject::tr(
                     "Perk %1 effect %2 contains %3 condition(s) not associated with any subject. These "
                     "conditions have been discarded."
                  ).arg(subject).arg(casted->which_effect).arg(casted->count);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::perk::unterminated_perk_effect*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Perk %1 effect %2 is missing its closing PRKF subrecord.",
                     disambig
                  ).arg(subject).arg(casted->which_effect);
               }
            #pragma endregion
            #pragma region quest
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::quest::alias_papyrus_data_belongs_to_missing_alias*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Quest %1 attempts to attach Papyrus script data to alias ID %2. No such alias exists on the quest.",
                     disambig
                  ).arg(subject).arg(casted->alias_id);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::quest::alias_papyrus_data_specifies_wrong_quest*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString target  = form_identifiers_to_string(&casted->target);
                  //
                  return QObject::tr(
                     "Quest %1 attempts to attach Papyrus script data to alias ID %3 on a completely different form, %2. "
                     "Although the game allows one quest to transplant script data onto aliases in other quests, DovahKit "
                     "is incapable of loading this data, so it will be discarded.",
                     disambig
                  ).arg(subject).arg(target).arg(casted->target_alias_id);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::quest::papyrus_fragment_belongs_to_missing_log_entry*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Quest %1 attempts to attach a Papyrus script fragment to quest stage %2, log entry %3. "
                     "No such log entry exists on the quest.",
                     disambig
                  ).arg(subject).arg(casted->stage_id).arg(casted->entry_index);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::quest::unexpected_subrecord_in_objective*>(&warning)) {
                  auto subject   = form_identifiers_to_string(&casted->subject);
                  auto signature = cobb::qt::four_cc_to_string(casted->signature);
                  
                  return QObject::tr(
                     "While loading a quest objective for quest %1, encountered an unexpected subrecord "
                     "with signature %2. A quest objective will blindly consume subrecords until it finds "
                     "one it expects; if the objective is missing its \"end\" subrecord, then the quest "
                     "will not load properly.",
                     disambig
                  ).arg(subject).arg(signature);
               }
            #pragma endregion
            #pragma region race
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::biped_object_name_too_long*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Biped Object Name #%2 in Race %1 is too long: it's %3 bytes long, but the Creation Kit "
                     "and the game will only load the first %4 bytes.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::invalid_boosted_skill*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Race %1 has an invalid skill boost: slot %2 uses invalid skill ID %3.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(casted->skill);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::invalid_face_texture_sex*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString target  = form_identifiers_to_string(casted->texture_set);
                  //
                  return QObject::tr(
                     "Race %1 attempts to offer TextureSet %2 as a complexion option, but the option is "
                     "associated with an invalid sex (%3).",
                     disambig
                  ).arg(subject).arg(target).arg(casted->invalid_sex);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::invalid_morph_bitmask_index*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Race %1 attempts to define indexed face morphs for index %2. The maximum valid "
                     "index is %3.",
                     disambig
                  ).arg(subject).arg(casted->index).arg(casted->max_index);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::movement_type_override_without_speeds*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Race %1 contained an improperly formatted speed override: an MTYP subrecord "
                     "not immediately followed by a SPED subrecord. The game will blindly swallow "
                     "whatever subrecord comes after MTYP and will load the form incorrectly. "
                     "(DovahKit will attempt to fix this up on save.)",
                     disambig
                  ).arg(subject);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::tint_layer_data_before_tint_layer*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "Race %1 contains a %2 subrecord before any TINI subrecord. That is: there exists "
                     "tint layer data before any actual tint layer.",
                     disambig
                  ).arg(subject).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::too_many_biped_object_names*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Race %1 attempts to define %2 Biped Object Name(s), but the maximum supported "
                     "number of Biped Object Name(s) is %3.",
                     disambig
                  ).arg(subject).arg(casted->count).arg(casted->max_count);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::too_many_phonemes*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Race %1 attempts to define weight values for %2 FaceFX phonemes, but FaceFx only "
                     "had %3 phonemes as of its use in Skyrim.",
                     disambig
                  ).arg(subject).arg(casted->count).arg(casted->max_count);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::race::wrong_weight_count_per_phoneme*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString phoneme = face_fx_phoneme_name(casted->phoneme);
                  //
                  return QObject::tr(
                     "Race %1 attempts to define %3 weight values for FaceFX phoneme %2, but the "
                     "race only defines %4 weights to use for lip synching.",
                     disambig
                  ).arg(subject).arg(phoneme).arg(casted->count).arg(casted->desired_count);
               }
            #pragma endregion
            #pragma region scene
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::invalid_scene_action_type*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Scene %1 contains an action with an invalid type (%2).",
                     disambig
                  ).arg(subject).arg(casted->action_type);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::scene_action_base_layout_incorrect*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  if (casted->problem == decltype(casted->problem)::malformed_valid_subrecord) {
                     return QObject::tr(
                        "Scene %1 action ID %2 contained a malformed %3 subrecord. The scene action will "
                        "not be loaded, and subrecords intended for the action may be consumed by the Scene "
                        "itself instead of a scene action.",
                        disambig
                     ).arg(subject).arg(casted->action_id).arg(signature);
                  } else if (casted->problem == decltype(casted->problem)::unrecognized_subrecord) {
                     return QObject::tr(
                        "Scene %1 action ID %2 contained an unrecognized subrecord (%3). The scene action "
                        "will not be loaded, and subrecords intended for the action may be consumed by the "
                        "Scene itself instead of a scene action.",
                        disambig
                     ).arg(subject).arg(casted->action_id).arg(signature);
                  } else {
                     return QObject::tr(
                        "Scene %1 action ID %2 has its first few subrecords laid out incorrectly in some "
                        "way. (Please ask DovahKit's developer to update this error message so that it "
                        "properly offers specifics.) The scene action will not be loaded, and subrecords "
                        "intended for the action may be consumed by the Scene itself instead of a scene "
                        "action.",
                        disambig
                     ).arg(subject).arg(casted->action_id).arg(signature);
                  }
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::scene_action_necessary_subrecord_missing*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "Scene %1 action ID %2 is missing a necessary subrecord (%3). The scene action will not "
                     "be loaded, and subrecords intended for the action may be consumed by the Scene itself "
                     "instead of a scene action.",
                     disambig
                  ).arg(subject).arg(casted->action_id).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::scene_action_necessary_subrecord_unreadable*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "Scene %1 action ID %2 contained a necessary subrecord (%3) with malformed or truncated "
                     "data. The scene action will not be loaded, and subrecords intended for the action may "
                     "be consumed by the Scene itself instead of a scene action.",
                     disambig
                  ).arg(subject).arg(casted->action_id).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::scene_actor_subrecords_out_of_order*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "Scene %1 contained an actor definition for which the subrecords were out of order. Data "
                     "may be loaded into the wrong actor definition. The problem was detected upon reading a "
                     "subrecord with signature %2.",
                     disambig
                  ).arg(subject).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::unexpected_subrecord_in_scene_action*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->signature);
                  //
                  return QObject::tr(
                     "Scene %1 action ID %2 contained an unexpected subrecord (%3). The subrecord will be "
                     "skipped.",
                     disambig
                  ).arg(subject).arg(casted->action_id).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::unexpected_subrecord_in_scene_phase*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  auto    signature = cobb::qt::four_cc_to_string(casted->signature);
                  //
                  return QObject::tr(
                     "Scene %1 Phase %2 contained an unexpected subrecord (%3). The subrecord will be "
                     "skipped.",
                     disambig
                  ).arg(subject).arg(casted->which_phase + 1).arg(signature);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::scene::unterminated_scene_phase*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Scene %1 phase is missing its closing HNAM subrecord. The phase will not be loaded, "
                     "and the scene itself may fail to load data as well.",
                     disambig
                  ).arg(subject).arg(casted->which_phase + 1);
               }
            #pragma endregion
            #pragma region shout
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::shout::wrong_word_count*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Shout %1 defined %2 words. Shouts must have exactly three words.",
                     disambig
                  ).arg(subject).arg(casted->word_count);
               }
            #pragma endregion
            #pragma region sound category
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::sound_category::is_own_parent*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Sound category %1 is its own parent category.",
                     disambig
                  ).arg(subject);
               }
            #pragma endregion
            #pragma region sound descriptor
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::sound_descriptor::sound_file_path_too_long*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Sound file path #%2 in Sound Descriptor %1 is too long: it's %3 bytes long, but the Creation Kit "
                     "and the game will only load the first %4 bytes.",
                     disambig
                  ).arg(subject).arg(casted->which).arg(casted->size).arg(casted->max_serializable_size);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::sound_descriptor::unexpected_subrecord_after_cnam*>(&warning)) {
                  auto subject   = form_identifiers_to_string(&casted->subject);
                  auto signature = cobb::qt::four_cc_to_string(casted->unexpected_signature);
                  
                  return QObject::tr(
                     "While loading sound descriptor %1, encountered an unexpected subrecord with "
                     "with signature %2. This subrecord was placed after CNAM, where the game would "
                     "expect GNAM; depending on when the game decides to load the sound data, it may "
                     "mistake this unexpected subrecord for GNAM and misread its contents.",
                     disambig
                  ).arg(subject).arg(signature);
               }
            #pragma endregion
            #pragma region static collection
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::static_collection::orphaned_instances*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  return QObject::tr(
                     "Static Collection %1 contains %2 object instances not associated with any object "
                     "type (i.e. Static). These will be discarded."
                  ).arg(subject).arg(casted->count);
               }
            #pragma endregion
            #pragma region topic info
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::topic_info::response_addendum_subrecord_too_early*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
                  //
                  return QObject::tr(
                     "TopicInfo %1 contained subrecord %2, which defines information for the last loaded response, "
                     "before any response was loaded. This data will be discarded.",
                     disambig
                  ).arg(subject).arg(subrecord);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::topic_info::response_has_id_zero*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  QString count   = QString::number(casted->num_responses_using);
                  //
                  return QObject::tr(
                     "TopicInfo %1 contains %2 response(s) which use(s) ID zero. Responses must have a unique ID, "
                     "but ID zero will cause the game and Creation Kit to use an incorrect filename for a response's "
                     "voice files.",
                     disambig
                  ).arg(subject).arg(count);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::topic_info::response_ids_are_not_unique*>(&warning)) {
                  QString subject   = form_identifiers_to_string(&casted->subject);
                  QString responses = QString::number(casted->response_count);
                  QString ids;
                  {
                     auto& src = casted->reused_ids;
                     for (size_t i = 0; i < src.size(); ++i) {
                        ids += QString::number(src[i]);
                        if (i + 1 < src.size())
                           ids += ", ";
                     }
                  }
                  //
                  return QObject::tr(
                     "TopicInfo %1 contained %2 responses, each of which must have a unique ID. However, there are "
                     "only 255 IDs available per response, and the following IDs are reused: %3.",
                     disambig
                  ).arg(subject).arg(responses).arg(ids);
               }
            #pragma endregion
            #pragma region weapon
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::weapon::invalid_resistance*>(&warning)) {
                  QString subject    = form_identifiers_to_string(&casted->subject);
                  QString resistance = QString::number(casted->actor_value);
                  if (casted->actor_value < dovah::all_actor_value_info.size()) {
                     auto& name = dovah::all_actor_value_info[casted->actor_value].name;
                     //
                     // TODO: Once we can load ActorValueInfo form data, prefer the AV's display/localized name 
                     // if it isn't blank.
                     //
                     resistance = QObject::tr("%1: %2").arg(resistance).arg(QString::fromLatin1(name.data(), name.size()));
                  }

                  return QObject::tr(
                     "Weapon %1 uses an invalid resistance (%2); weapons must specify an Actor Value that can "
                     "be used as a damage resistance. The game will correct this to \"None\" on load, so "
                     "DovahKit does as well.",
                     disambig
                  ).arg(subject).arg(resistance);
               }
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::weapon::invalid_skill*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Weapon %1 uses an invalid skill (%2). The game will correct this to \"None\" on load, "
                     "so DovahKit does as well.",
                     disambig
                  ).arg(subject).arg(casted->skill);
               }
            #pragma endregion
            #pragma region worldspace
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::worldspace::is_own_parent*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Worldspace %1 is its own parent. The game will freeze (infinite loading screen) when trying to load it.",
                     disambig
                  ).arg(subject);
               }
            #pragma endregion
         #pragma endregion
      #pragma endregion

      #pragma region form save warnings
         if (auto* casted = cobb::dynamic_fast_cast<const form_save_warnings::not_yet_implemented*>(&warning)) {
            QString subject   = form_identifiers_to_string(&casted->subject);
            QString subrecord = cobb::qt::four_cc_to_string(casted->subrecord_signature);
            QString explanation;
            
            switch (casted->subject.form_type) {
               case dovah::form_type::land:
                  switch (casted->subrecord_signature) {
                     case 'MPCD':
                        explanation = QObject::tr(" (pre-calculated collision data)", disambig);
                        break;
                  }
                  break;
            }

            return QObject::tr(
               "%1 contains subrecord %2%3, which will not be saved, as DovahKit cannot yet generate/update/save this data.",
               disambig
            ).arg(subject).arg(subrecord).arg(explanation);
         }
      #pragma endregion

      return QObject::tr("Unknown warning.", disambig);
   }
}