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
         //
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
         //
         //
         //
         //
         //
         //
         record_found_in_wrong_top_level_group       = 0x00000042,
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
