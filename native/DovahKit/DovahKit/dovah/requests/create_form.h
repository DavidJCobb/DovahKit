#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include "dovah/types/bare_form_id.h"
#include "dovah/types/cells/grid_position.h"
namespace dovah {
   class active_load_order;
   class form_stub;
   enum class form_type : uint8_t;
}

namespace dovah::requests {
   class create_form {
      public:
         enum class error_code {
            // There is no active file. (If you didn't specify an existing active file when setting up 
            // the load order, then this means that there also wasn't room in the load order for the 
            // implicitly-created active file.)
            no_active_file,

            // The requested form type is invalid.
            invalid_form_type,

            // Full support for the requested form type is not yet implemented.
            unimplemented_form_type,

            // The specified parent form cannot be a parent to forms of the requested form type.
            invalid_parent_child_relationship,

            // You cannot create a ref with no parent cell.
            cannot_create_reference_with_no_parent_cell,

            // You cannot create an interior cell with a parent form.
            interior_cell_clone_cannot_have_parent,

            // You cannot create an exterior cell with no parent worldspace.
            exterior_cell_clone_must_have_parent,

            // You tried to create an exterior cell, but you didn't specify world grid coordinates for 
            // it to be placed into.
            exterior_cell_must_have_grid_coordinates,

            // Don't use a worldspace's persistent cell as the parent for a new cell-child form. Pick 
            // the appropriate parent cell based on the desired coordinates of the to-be-created form.
            do_not_use_worldspace_persistent_cell_as_parent,

            // No form IDs are available for use right now.
            no_form_id_available,

            // You wanted to create an exterior cell in a given worldspace, but the requested exterior 
            // grid coordinates are already taken by an existing cell.
            exterior_grid_coordinates_already_taken,

            // This form type doesn't exist in the currently loaded game (e.g. Skyrim Special Edition-
            // exclusive form types in Skyrim Classic).
            form_type_unavailable_in_current_game,
         };

      public:
         create_form(active_load_order&);
         create_form(const create_form&) = delete;
         create_form& operator=(const create_form&) = delete;
         ~create_form();

      public:
         active_load_order& active_load_order;
         form_stub*         child_of  = nullptr;
         form_stub*         clone_of  = nullptr;
         std::string        editor_id;
         const bare_form_id form_id   = 0;
         enum form_type     form_type = {};
         std::optional<types::cells::grid_position> grid_position; // for exterior cells only
      protected:
         bool committed = false;

      public:
         std::optional<error_code> would_fail() const;
         form_stub& commit();
   };
}