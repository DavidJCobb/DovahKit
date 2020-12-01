#pragma once
#include "notice_code_t.h"

namespace dovah {
   struct notice_code {
      notice_code()       = delete;
      enum type : notice_code_t {
         none       = 0x00000000,
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
         too_many_dependencies                       = 0x0000000C,
         no_active_file                              = 0x0000000D,
         save_complete_but_to_temporary_file         = 0x0000000E, // The file was saved successfully, but only to a temporary file. It was not possible to rename that temporary file to the desired filename.
         game_conversion_form_cleanup_failed         = 0x0000000F, // Some forms were not saved due to the file, but could not be deleted from memory. It is not safe to continue this editing session.
         form_override_has_type_mismatch             = 0x00000010,
         form_override_has_armo_arma_mismatch        = 0x00000011,
         cell_flags_not_yet_found                    = 0x00000012, // CELL full load: a subrecord specific to interior or exterior cells was found before we discovered (by virtue of CELL/DATA) what type of cell this is.
         interior_cell_data_in_exterior_cell         = 0x00000013, // CELL full load: a subrecord specific to interior cells was found in a cell that is flagged as an exterior.
         exterior_cell_data_in_interior_cell         = 0x00000014, // CELL full load: a subrecord specific to exterior cells was found in a cell that is flagged as an interior.
         unrecognized_subrecord                      = 0x00000015, // FORM full load: a subrecord was unrecognized.
         form_reference_is_of_incorrect_type         = 0x00000016, // FORM full load: a (form_reference_t) ended up referring to a form of the wrong type.
         shout_has_wrong_word_count                  = 0x00000017, // SHOU full load: the shout has too many, or too few, words.
         package_event_dialogue_unrecognized_subrecord = 0x00000018,
         game_setting_record_is_nameless             = 0x00000019, // A GMST record had no EDID or an empty EDID.
         game_setting_record_is_misordered           = 0x0000001A, // A GMST record has its EDID record in the wrong place.
         subrecord_has_extra_content                 = 0x0000001B, // A subrecord has unexpected data at its end. (This is not emitted for most subrecords, but is explicitly checked for in special cases like the GMST loader.)
         game_setting_record_is_redundant            = 0x0000001C, // Multiple GMST records in the same file define the same setting but with different form IDs.
         game_setting_record_has_bad_form_id         = 0x0000001D, // A GMST record has an out-of-bounds or otherwise invalid form ID.
         game_setting_record_has_bad_type            = 0x0000001E, // A GMST record has an unrecognized name, and the name's type prefix is also unrecognized.
         game_setting_name_is_unrecognized           = 0x0000001F,
         form_id_unavailable_for_game_setting        = 0x00000020, // A game setting edit request failed because a form ID wasn't available for use.
         game_setting_edit_request_lacked_id         = 0x00000021, // A game setting edit request failed because it had no form ID.
         form_id_is_already_in_use                   = 0x00000022,
         form_id_is_reserved_for_other_process       = 0x00000023, // Cannot use the specified form ID. It is reserved for use by another process, such as form creation or form renumbering.
         cannot_load_right_now                       = 0x00000024, // It is not safe to load right now, as a save or load operation is already in progress.
         game_setting_record_has_no_data             = 0x00000025, // A GMST record had no DATA subrecord.
         game_setting_record_unreadable_data         = 0x00000026, // A GMST record's DATA subrecord could not be read. This can happen if the subrecord is too small.
         cannot_sever_references_to_target           = 0x00000027,
         game_setting_is_not_in_active_file          = 0x00000028, // Cannot renumber a GMST that doesn't originate from the active file.
         default_object_rejected_for_bad_type        = 0x00000029, // The loaded DefaultObjectManager won't let you use the specified form for the specified entry, as the form is of the wrong type.
         default_object_accepted_but_unknown         = 0x0000002A, // The loaded DefaultObjectManager has created the specified entry, but wants you to know that the entry isn't one that DovahKit's backend recognizes.
         singleton_form_is_redundantly_defined       = 0x0000002B, // A file contained multiple records for the same singleton form (e.g. multiple DOBJ records or multiple NAVI records).
         cannot_renumber_hardcoded_form              = 0x0000002C,
         form_is_not_defined_in_active_file          = 0x0000002D, // The desired operation can only be performed on forms that were originally defined in the active file.
         zero_is_not_an_allowed_form_id              = 0x0000002E, // The desired operation does not allow you to use zero as a form ID.
         cannot_load_all_users_of_this_form          = 0x0000002F, // The desired operation requires that DovahKit load all of the forms that use the target form, and that isn't yet implemented.
         form_id_is_in_the_hardcoded_range           = 0x00000030, // You cannot use this form ID, because it's in the range reserved for hardcoded forms.
         cannot_sever_references_to_none_stub        = 0x00000031,
         unimplemented_form_type                     = 0x00000032, // DovahKit recognizes this form type and it is valid, but editing has not yet been implemented for it.
         invalid_parent_child_relationship           = 0x00000033, // Forms of type A cannot have parents of type B.
         exterior_grid_coordinates_already_taken     = 0x00000034, // Cannot create a cell with the specified coordinates, as a cell in this worldspace already has those coordinates.
         cannot_create_reference_with_no_parent_cell = 0x00000035,
         interior_cell_clone_cannot_have_parent      = 0x00000036, // When duplicating an interior cell, you must not specify a parent worldspace.
         exterior_cell_clone_must_have_parent        = 0x00000037, // When duplicating an exterior cell, you must specify a parent worldspace.
         form_created_but_clone_failed               = 0x00000038, // We were able to make a new form, but Form::clone() returned false; we made a blank new form instead of duplicating a form as requested.
         form_id_unavailable_for_new_form            = 0x00000039, // A form creation request failed because a form ID wasn't available for use.
         form_id_is_out_of_bounds                    = 0x0000003A, // The requested form ID is out-of-bounds, e.g. form ID 0x05000000 in a load order with fewer than six files.
         post_save_none_stub_cleanup_failed          = 0x0000003B, // Failed to clean up none-stubs after an otherwise successful save operation. It is not safe to continue this editing session.
         malformed_file                              = 0x0000003C,
         missing_master                              = 0x0000003D, // Failed to load a file, because one of the file's masters is missing.
         missing_file                                = 0x0000003E, // Failed to load a file, because the file is missing.
         file_is_locked                              = 0x0000003F, // DovahKit was unable to read a file because it is locked by the OS.
         cyclical_dependency_between_files           = 0x00000040, // The load order contains files whose master lists form a circular dependency.
         active_file_is_master_and_there_are_plugins = 0x00000041, // DovahKit can't place the active file at the end of the load order, because it's ESM-flagged and other files aren't.
         record_found_in_wrong_top_level_group       = 0x00000042,
      };
   };
}
