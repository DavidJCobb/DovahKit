#pragma once

namespace dovah::exceptions {
   enum class form_creation_error_code {
      // The request was passed to a `file_load_order` instance other than the one used to 
      // create and configure it.
      wrong_load_order,

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

      // The form ID that DovahKit tried to use is occupied by a none-stub; that is, it's the 
      // target of dangling references from one or more forms. The forms in question are all 
      // defined in the active file; however, a call to `form_stub::load` failed for at least one 
      // of them, so we were unable to sever the dangling reference(s).
      cannot_sever_references_to_none_stub,
      
      // This form type doesn't exist in the currently loaded game (e.g. Skyrim Special Edition-
      // exclusive form types in Skyrim Classic).
      form_type_unavailable_in_current_game,
   };
}