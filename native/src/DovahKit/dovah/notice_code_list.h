#pragma once
#include "notice_code_t.h"

namespace dovah {
   struct notice_code {
      notice_code()       = delete;
      enum type : notice_code_t {
         none = 0x00000000,
         //
         load_order_would_overflow_into_lights       = 0x00000001,
         load_order_contains_light_files             = 0x00000002,
         load_order_is_invalid_somehow               = 0x00000003, // The operation you wish to perform is impossible, because the load order is already in an invalid state.
         unknown_form_type                           = 0x00000004,
         cannot_save_right_now                       = 0x00000005, // It is not safe to save right now, as a save or load operation is already in progress.
         no_filename_specified                       = 0x00000006,
         save_complete_but_reopen_failed             = 0x00000007, // A file was successfully saved, but could not be reopened afterwards. Further editing is not possible.
         out_of_memory                               = 0x00000008,
         zlib_memory_error                           = 0x00000009,
         zlib_buffer_error                           = 0x0000000A,
         forms_out_of_esl_form_id_range              = 0x0000000B, // One or more relevant forms is outside of the range of form IDs available to an ESL.
         file_has_too_many_dependencies              = 0x0000000C,
         no_active_file                              = 0x0000000D,
         save_complete_but_to_temporary_file         = 0x0000000E, // The file was saved successfully, but only to a temporary file. It was not possible to rename that temporary file to the desired filename.
         unsaved_form_cleanup_failed                 = 0x0000000F, // Some forms were not saved to the file, but could not be deleted from memory. It is not safe to continue this editing session.
         form_override_has_type_mismatch             = 0x00000010,
         //
         //
         //
         //
         //
         //
         //
         //
         //
         game_setting_record_is_misordered           = 0x0000001A, // A GMST record has its EDID record in the wrong place.
         subrecord_has_extra_content                 = 0x0000001B, // A subrecord has unexpected data at its end. (This is not emitted for most subrecords, but is explicitly checked for in special cases like the GMST loader.)
         //
         //
         game_setting_record_has_bad_type            = 0x0000001E, // A GMST record has an unrecognized name, and the name's type prefix is also unrecognized.
         //
         //
         //
         //
         form_id_is_reserved_for_other_process       = 0x00000023, // Cannot use the specified form ID. It is reserved for use by another process, such as form creation or form renumbering.
         cannot_load_right_now                       = 0x00000024, // It is not safe to load right now, as a save or load operation is already in progress.
         game_setting_record_has_no_data             = 0x00000025, // A GMST record had no DATA subrecord.
         game_setting_record_unreadable_data         = 0x00000026, // A GMST record's DATA subrecord could not be read. This can happen if the subrecord is too small.
         //
         //
         default_object_rejected_for_bad_type        = 0x00000029, // The loaded DefaultObjectManager won't let you use the specified form for the specified entry, as the form is of the wrong type.
         default_object_accepted_but_unknown         = 0x0000002A, // The loaded DefaultObjectManager has created the specified entry, but wants you to know that the entry isn't one that DovahKit's backend recognizes.
         singleton_form_is_redundantly_defined       = 0x0000002B, // A file contained multiple records for the same singleton form (e.g. multiple DOBJ records or multiple NAVI records).
         //
         //
         zero_is_not_an_allowed_form_id              = 0x0000002E, // The desired operation does not allow you to use zero as a form ID.
         cannot_load_all_users_of_this_form          = 0x0000002F, // The desired operation requires that DovahKit load all of the forms that use the target form, and that isn't yet implemented.
         //
         //
         unimplemented_form_type                     = 0x00000032, // DovahKit recognizes this form type and it is valid, but editing has not yet been implemented for it.
         //
         //
         //
         //
         //
         //
         //
         form_id_is_out_of_bounds                    = 0x0000003A, // The requested form ID is out-of-bounds, e.g. form ID 0x05000000 in a load order with fewer than six files.
         post_save_none_stub_cleanup_failed          = 0x0000003B, // Failed to clean up none-stubs after an otherwise successful save operation. It is not safe to continue this editing session.
         malformed_file                              = 0x0000003C, // Generic error code for malformed files.
         missing_master                              = 0x0000003D, // Failed to load a file, because one of the file's masters is missing.
         missing_file                                = 0x0000003E, // Failed to load a file, because the file is missing.
         locked_file                                 = 0x0000003F, // DovahKit was unable to read a file because it is locked by the OS.
         cyclical_dependency_between_files           = 0x00000040, // The load order contains files whose master lists form a circular dependency.
         active_file_is_master_and_there_are_plugins = 0x00000041, // DovahKit can't place the active file at the end of the load order, because it's ESM-flagged and other files aren't.
         record_found_in_wrong_top_level_group       = 0x00000042,
         //
         //
         unknown_error                               = 0x00000045,
         active_file_is_dependency                   = 0x00000046, // The active file is listed as another file's master. This load order is invalid, because we need the active file at the bottom of the load order.
         load_order_would_have_too_many_files        = 0x00000047, // We can't load this load order. It would have too many files (greater than 4096 lights, 255 heavies, or if an active file is selected, 255 or 254 total).
         filesystem_error                            = 0x00000048,
         interior_cell_block_has_no_parent_group     = 0x00000049, // Malformed file: an interior cell block GRUP has no parent GRUP.
         interior_cell_block_group_badly_nested      = 0x0000004A, // Malformed file: an interior cell block GRUP is nested under a parent GRUP of the wrong type or hierarchy.
         invalid_record_signature                    = 0x0000004B, // Malformed file: a record had a suspicious (or just unknown) signature.
         form_id_is_inside_of_a_missing_master       = 0x0000004C,
         unexpected_nested_group_in_simple_top_group = 0x0000004D, // Malformed file: a "simple" top-level GRUP contained a nested GRUP.
         extended_subrecord_with_no_length           = 0x0000004E, // Malformed file: an 'XXXX' subrecord contained no usable length value.
         form_initial_record_is_partial              = 0x0000004F, // A form's initial record is flagged as "partial." The flag will not be honored when loading form data on-demand.
         form_initial_record_is_partial_and_injected = 0x00000050, // A form's initial record is flagged as "partial," and is injected. The game would skip this record entirely, so we're skipping it as well; the form has not been loaded.
         cannot_delete_hardcoded_form                = 0x00000051,
         quest_objective_unexpected_subrecord        = 0x00000052, // Unrecognized subrecord inside of the object. The loader for this object blindly consumes subrecords until it finds an expected end; if the end is missing, then this will end badly!
         parent_form_is_missing                      = 0x00000053, // Failed to load a file, because a form's parent form ID doesn't correspond to a valid form.
         partial_info_override_has_different_parent  = 0x00000054, // A partial-flagged INFO override has a different parent from the original. This can lead to all sorts of data mishandling in-game.
         //
         //
         //
         too_many_script_fragments_to_save           = 0x00000058, // A form has too many script fragments, and cannot be saved.
         too_many_aliases_with_scripts_to_save       = 0x00000059, // A quest has too many aliases with script data, and cannot be saved.
         //
         //
         //
         //
         //
         //
         length_prefixed_string_was_too_long_to_save = 0x00000060,
         too_many_destruction_stages_to_save         = 0x00000061,
         //
         //
         //
         //
         havok_data_is_not_supported_here            = 0x00000066, // This feature can't load/save/etc. Havok data.
         landscape_heights_are_too_steep             = 0x00000067, // This landscape cannot be saved, because there is too steep a slope somewhere in its heightmap.
         //
         //
         //
         //
         //
      };
   };
}
