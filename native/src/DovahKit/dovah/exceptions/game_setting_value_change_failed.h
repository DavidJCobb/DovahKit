#pragma once
#include <stdexcept>
#include "helpers/vector3.h"
#include "../core.h"

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class game_setting_value_change_failed : public std::runtime_error {
      public:
         enum class error_code {
            // The request was passed to a `file_load_order` instance other than the one used to 
            // create and configure it.
            wrong_load_order,

            // The request specified a specific form ID to use if the game setting wasn't already 
            // present in the active file. However, the requested form ID was zero, and this is not 
            // allowed.
            form_id_is_zero,

            /*//
            // NOTE: This is only a hard error in Skyrim Classic.
            form_id_is_in_hardcoded_range,

            // The request specified a specific form ID to use if the game setting wasn't already 
            // present in the active file. However, the requested form ID doesn't map to any valid 
            // load order slot.
            form_id_is_out_of_bounds,
            //*/

            // The request specified a specific form ID to use if the game setting wasn't already 
            // present in the active file. However, the requested form ID is already being used by 
            // another form.
            form_id_is_occupied,

            // The request specified a specific form ID to use if the game setting wasn't already 
            // present in the active file. However, the requested form ID has been reserved for use 
            // by an in-progress editing operation.
            form_id_is_reserved,

            // There is no active file. (If you didn't specify an existing active file when setting up 
            // the load order, then this means that there also wasn't room in the load order for the 
            // implicitly-created active file.)
            no_active_file,

            // The request specified that if the game setting wasn't already present in the active 
            // file, a form ID should be automatically selected and used to store its value. However, 
            // no form IDs are available for use right now.
            no_form_id_available,
         };

      public:
         game_setting_value_change_failed(error_code ec, std::string& sn) : std::runtime_error("Failed to change the value of the given game setting."), code(ec), setting_name(sn) {}

         const error_code code;
         std::string      setting_name;

         struct {
            form_stub* occupying_stub = nullptr;
         } details;
   };
}