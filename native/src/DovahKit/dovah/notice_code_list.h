#pragma once
#include "notice_code_t.h"

namespace dovah {
   struct notice_code {
      notice_code()       = delete;
      enum type : notice_code_t {
         none = 0x00000000,
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
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
         //
         cannot_load_right_now                       = 0x00000024, // It is not safe to load right now, as a save or load operation is already in progress.
         game_setting_record_has_no_data             = 0x00000025, // A GMST record had no DATA subrecord.
         game_setting_record_unreadable_data         = 0x00000026, // A GMST record's DATA subrecord could not be read. This can happen if the subrecord is too small.
         //
         //
         default_object_rejected_for_bad_type        = 0x00000029, // The loaded DefaultObjectManager won't let you use the specified form for the specified entry, as the form is of the wrong type.
         default_object_accepted_but_unknown         = 0x0000002A, // The loaded DefaultObjectManager has created the specified entry, but wants you to know that the entry isn't one that DovahKit's backend recognizes.
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
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
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         quest_objective_unexpected_subrecord        = 0x00000052, // Unrecognized subrecord inside of the object. The loader for this object blindly consumes subrecords until it finds an expected end; if the end is missing, then this will end badly!
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         //
         havok_data_is_not_supported_here            = 0x00000066, // This feature can't load/save/etc. Havok data.
         //
         //
         //
         //
         //
         //
      };
   };
}
