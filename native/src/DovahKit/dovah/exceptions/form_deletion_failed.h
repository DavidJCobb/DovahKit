#pragma once
#include <stdexcept>
#include "helpers/vector3.h"
#include "../core.h"

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class form_deletion_failed: public std::runtime_error{
      public:
         enum class error_code {
            // The request was passed to a `file_load_order` instance other than the one used to 
            // create and configure it.
            wrong_load_order,

            // You cannot delete hardcoded forms.
            form_is_hardcoded,

            // There is no active file. (If you didn't specify an existing active file when setting up 
            // the load order, then this means that there also wasn't room in the load order for the 
            // implicitly-created active file.)
            no_active_file,

            // One or more of the to-be-deleted forms cannot be loaded, because loading of that form 
            // type is not yet implemented.
            unimplemented_form_type,

            // One or more of the to-be-deleted forms cannot be deleted, because it is referred to by 
            // a form whose full data we failed to load.
            cannot_load_all_users_of_this_form,
         };

      public:
         form_deletion_failed(error_code ec, form_stub& fs) : std::runtime_error("Failed to delete the given form or one of its children."), code(ec), subject(fs) {
            this->details.referent = &fs;
         }

         const error_code code;
         form_stub&       subject;

         struct {
            form_stub* referent = nullptr; // form we couldn't delete
            form_stub* referrer = nullptr; // form whose reference we couldn't sever
         } details;
   };
}