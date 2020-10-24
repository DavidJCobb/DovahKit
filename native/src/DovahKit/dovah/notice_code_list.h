#pragma once
#include "notice_code_t.h"

namespace dovah {
   struct notice_code {
      notice_code() = delete;
      enum type : notice_code_t {
         none = 0x00000000,
         //
         load_order_would_overflow_into_lights = 0x00000001,
         load_order_contains_light_files       = 0x00000002,
         load_order_is_invalid_somehow         = 0x00000003, // The operation you wish to perform is impossible, because the load order is already in an invalid state.
         unknown_form_type                     = 0x00000004,
         cannot_save_right_now                 = 0x00000005, // It is not safe to save right now, as a save or load operation is already in progress.
         no_filename_specified                 = 0x00000006,
         save_complete_but_reopen_failed       = 0x00000007, // A file was successfully saved, but could not be reopened afterwards. Further editing is not possible.
         out_of_memory                         = 0x00000008,
         zlib_memory_error                     = 0x00000009,
         zlib_buffer_error                     = 0x0000000A,
         forms_out_of_esl_form_id_range        = 0x0000000B, // One or more relevant forms is outside of the range of form IDs available to an ESL.
         too_many_dependencies                 = 0x0000000C,
         no_active_file                        = 0x0000000D,
         save_complete_but_to_temporary_file   = 0x0000000E,
         game_conversion_form_cleanup_failed   = 0x0000000F, // Some forms were not saved due to the file, but could not be deleted from memory. It is not safe to continue this editing session.
         form_override_has_type_mismatch       = 0x00000010,
         form_override_has_armo_arma_mismatch  = 0x00000011,
      };
   };
}
