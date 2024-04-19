#include "./backend_warning_to_string.h"
#include <QObject>
#include "helpers/dynamic_fast_cast.h"
#include "dovah/notices/_all_warnings.h"

#include "./form_identifiers_to_string.h"
#include "./form_type_name_to_string.h"
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
         if (auto* casted = cobb::dynamic_fast_cast<const file_load_warnings::form_override_has_armo_arma_mismatch*>(&warning)) {
            auto overridden_form = form_identifiers_to_string(&casted->overridden_form.stub);
            auto overriding_form = QString("[%1:%2]%3")
               .arg(editor_helpers::form_signature_to_string(&casted->overriding_form.stub))
               .arg(editor_helpers::form_id_to_string(casted->overriding_form.form_ids.global))
               .arg(casted->overriding_form.stub.get_editor_id());

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
            #pragma endregion
            #pragma region shout
               if (auto* casted = cobb::dynamic_fast_cast<const form_load_warnings::by_type::shout::wrong_word_count*>(&warning)) {
                  QString subject = form_identifiers_to_string(&casted->subject);
                  //
                  return QObject::tr(
                     "Shout %1 defined %2 words. Shouts must have exactly three words",
                     disambig
                  ).arg(subject).arg(casted->word_count);
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