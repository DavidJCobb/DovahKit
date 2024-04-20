#pragma once
#include <stdexcept>
#include "helpers/vector3.h"
#include "../core.h"

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class game_setting_renumber_failed : public std::runtime_error {
      public:
         enum class error_code {
            // The request was passed to a `file_load_order` instance other than the one used to 
            // create and configure it.
            wrong_load_order,

            // This game setting isn't defined in the active file; there's nothing to renumber, or 
            // you're trying to renumber a GMST from outside the active file.
            setting_is_not_in_active_file,

            // The requested form ID was zero, and this is not allowed.
            form_id_is_zero,

            // NOTE: This is only a hard error in Skyrim Classic.
            form_id_is_in_hardcoded_range,

            // The requested form ID doesn't map to any valid load order slot.
            form_id_is_out_of_bounds,

            // The requested form ID is already being used by another form.
            form_id_is_occupied,

            // The requested form ID has been reserved for use by an in-progress editing operation.
            form_id_is_reserved,

            // There is no active file. (If you didn't specify an existing active file when setting up 
            // the load order, then this means that there also wasn't room in the load order for the 
            // implicitly-created active file.)
            no_active_file,

            // One or more forms refer to the setting by form ID, even though that should not be 
            // possible. A call to `form_stub::load` failed for at least one of them, so we were 
            // unable to sever the reference.
            cannot_sever_references_to_setting,
         };

      public:
         game_setting_renumber_failed(error_code ec, std::string& sn) : std::runtime_error("Failed to renumber the given game setting."), code(ec), setting_name(sn) {}

         const error_code code;
         std::string      setting_name;

         struct {
            form_stub* occupying_stub = nullptr;
         } details;
   };
}