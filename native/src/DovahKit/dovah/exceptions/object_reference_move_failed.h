#pragma once
#include <stdexcept>
#include "helpers/vector3.h"

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class object_reference_move_failed : public std::runtime_error {
      public:
         enum class error_code {
            // You asked to move this ref to an exterior cell, but the position you wanted to use is outside of 
            // that cell's bounds. If you want DovahKit to just put it in whatever cell is appropriate, then 
            // specify the parent world instead.
            desired_position_is_outside_of_desired_cell,

            // The coordinates you wanted to move this ref to lie outside of any existing cells, and attempting 
            // to create a new cell failed (e.g. no form ID available, or some other internal problem).
            failed_to_create_destination_cell,

            // This operation isn't allowed on working copies, as it requires setting relationships between the 
            // form stubs involved (e.g. parent/child cell/ref relationships) and so is not bounded to the 
            // working copy. In general, working copies exist for things like dialog boxes with OK/Cancel buttons, 
            // where it's simpler to have a form to write to than to try and maintain all state within the UI. For 
            // those cases, it should be trivial to hang on to whatever coordinates you want to set manually, and 
            // then invoke this operation on the "real" form after the working copy is committed to it.
            operation_not_allowed_on_form_working_copy,

            // Hardcoded refs typically don't exist in a parent cell, and cannot be reparented.
            reference_is_hardcoded,

            // This ref has no parent form (e.g. PlayerRef), and so cannot be moved.
            reference_is_orphaned,
         };

      public:
         object_reference_move_failed(error_code ec, form_stub& fs) : std::runtime_error("Failed to alter a ref's position/cell/world."), code(ec), subject(fs) {}

         const error_code code;
         form_stub&       subject;

         struct {
            form_stub*           cell = nullptr;
            cobb::vector3<float> position;
         } details;
   };
}