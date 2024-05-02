#pragma once
#include "./form_reference_type_mismatch.h"
#include "./unrecognized_subrecord.h"
#pragma region by form component
   #include "./by_form_component/attack_data/expected_event_subrecord.h"
   #include "./by_form_component/container/item_has_bad_owner_form_type.h"
   #include "./by_form_component/destruction/stage_serialized_index_out_of_bounds.h"
   #include "./by_form_component/extra_data/room_ref_data_insufficient_rooms.h"
   #include "./by_form_component/extra_data/room_ref_data_swallowed_subrecord.h"
   #include "./by_form_component/package_event_dialogue/unrecognized_subrecord.h"
#pragma endregion
#pragma region by form type
   #include "./by_form_type/cell/cell_type_not_yet_known.h"
   #include "./by_form_type/cell/data_for_wrong_cell_type.h"
   #include "./by_form_type/dialogue_branch/mishandled_owning_quest_id.h"
   #include "./by_form_type/landscape/excess_layers_per_quad.h"
   #include "./by_form_type/landscape/invalid_quad_for_land_texture.h"
   #include "./by_form_type/note/non_texture_note_includes_texture_path.h"
   #include "./by_form_type/quest/alias_papyrus_data_belongs_to_missing_alias.h"
   #include "./by_form_type/quest/alias_papyrus_data_specifies_wrong_quest.h"
   #include "./by_form_type/quest/papyrus_fragment_belongs_to_missing_log_entry.h"
   #include "./by_form_type/quest/unexpected_subrecord_in_objective.h"
   #include "./by_form_type/shout/wrong_word_count.h"
   #include "./by_form_type/topic_info/response_addendum_subrecord_too_early.h"
   #include "./by_form_type/worldspace/is_own_parent.h"
#pragma endregion