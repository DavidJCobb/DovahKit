#pragma once
#include <cstdint>
#include <string>
#include "../core.h"
#include "../form_types.h"
#include "../notice_code_t.h"

namespace dovah {
   class file_load_order;
   class form_stub;
}

namespace dovah {
   class form_creation_request {
      friend class file_load_order;
      friend class form_duplication_request;
      //
      // Instances of this class can be created through the (file_load_order), and allow outside 
      // code to take actions in between reserving a form ID for use with a new form, and actually 
      // creating the new form. The use case that drove its creation: being able to have this UI 
      // flow:
      //
      //  - User asks to create a new form. We immediately try to reserve a form ID.
      //
      //  - If the reservation fails, we report an error and abort immediately.
      //
      //  - We ask the user for the desired editor ID.
      //
      //  - We create the form, with that editor ID, all in one go.
      //
      // This class is capable of creating a new, blank form, or of duplicating a single form. If 
      // you wish to duplicate a form and its children, then use (form_duplication_request).
      //
      protected:
         file_load_order& owner;
         enum form_type   form_type = form_type::none;
         bare_form_id_t   formID    = 0;       // the form ID reserved for the newly-created form. set by the owning load order
         form_stub*       child_of  = nullptr; // what form should serve as the new form's parent?
         form_stub*       clone_of  = nullptr; // do we want to create a new form from scratch, or duplicate an existing one?
         notice_code_t    error     = default_notice_code;

         form_creation_request(file_load_order& o);
         form_creation_request(form_creation_request&&);
         form_creation_request(const form_creation_request&) = delete;
         form_creation_request& operator=(const form_creation_request&) = delete;
         
      public:
         ~form_creation_request();
         
         std::string editorID; // the editor ID to be used for the new form
         struct {
            int32_t x = 0;
            int32_t y = 0;
            bool    present = false;
         } cell_grid_coordinates; // grid coordinates to use when creating an exterior cell
         
         constexpr bool is_valid() const noexcept { return this->formID != 0 && this->error == default_notice_code; } // returns (true) if the request has a reserved ID and has not yet completed/failed
         constexpr notice_code_t get_error_code() const noexcept { return this->error; }
         
         void set_parent_form(form_stub* parent);
         void set_parent_form(bare_form_id_t parentID);
         
         void queue_clone(form_stub* original);
         form_stub* commit();
   };
}