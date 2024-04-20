#pragma once
#include <stdexcept>
#include "helpers/vector3.h"
#include "../core.h"

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class form_renumber_failed : public std::runtime_error {
      public:
         enum class error_code {
            // The request was passed to a `file_load_order` instance other than the one used to 
            // create and configure it.
            wrong_load_order,

            // You cannot renumber hardcoded forms.
            form_is_hardcoded,

            // You can only renumber forms that originate from the active file.
            form_is_not_from_active_file,

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

            // The requested form ID is occupied by a none-stub, and belongs to a dependency; that is, 
            // you're attempting to do form injection, but the requested form ID is the target of a 
            // dangling reference from one or more forms. Unfortunately, in this case, the referring 
            // form does not originate from the active file, so we can't sever the reference.
            cannot_inject_form_overtop_none_stub,

            // The requested form ID is occupied by a none-stub; that is, it's the target of dangling 
            // references from one or more forms. The forms in question are all defined in the active 
            // file; however, a call to `form_stub::load` failed for at least one of them, so we were 
            // unable to sever the dangling reference(s).
            cannot_sever_references_to_none_stub,
         };

      public:
         form_renumber_failed(error_code ec, form_stub& fs) : std::runtime_error("Failed to renumber the given form."), code(ec), subject(fs) {}

         const error_code code;
         form_stub&       subject;

         struct {
            form_stub* none_stub = nullptr;
         } details;
   };
}