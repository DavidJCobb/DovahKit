#include "./form_creation_error_code.h"
#include <QCoreApplication>

#define ENUMERATION_TYPE dovah::exceptions::form_creation_error_code
#include "./_macros.h"

namespace editor::localize {
   extern QString form_creation_error_code(ENUMERATION_TYPE v) {
      using enum ENUMERATION_TYPE;
      switch (v) {
         case wrong_load_order:
            return STRING("An internal program error occurred: DovahKit tried to create a form using the wrong load-order state.");
         case no_active_file:
            return STRING("There is no active file, nor any room in the load order for a new file.");
         case invalid_form_type:
            return STRING("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
         case unimplemented_form_type:
            return STRING("DovahKit does not support editing this form type.");
         case invalid_parent_child_relationship:
            return STRING("The specified parent form cannot have a child form of this type.");
         case cannot_create_reference_with_no_parent_cell:
            return STRING("References and other \"cell child\" forms cannot be created outside of a cell.");
         case interior_cell_clone_cannot_have_parent:
            return STRING("Interior cells cannot have a parent worldspace.");
         case exterior_cell_clone_must_have_parent:
            return STRING("Exterior cells must have a parent worldspace.");
         case exterior_cell_must_have_grid_coordinates:
            return STRING("Exterior cells must have grid coordinates.");
         case do_not_use_worldspace_persistent_cell_as_parent:
            return STRING("DovahKit cannot create references and other \"cell child\" forms inside of a worldspace's persistent cell. A normal parent cell must be chosen, based on the desired coordinates of the to-be-created form.");
         case no_form_id_available:
            return STRING("You've used up all of the form IDs available to this file!");
         case exterior_grid_coordinates_already_taken:
            return STRING("The specified worldspace already has an exterior cell at the desired grid coordinates.");
         case cannot_sever_references_to_none_stub:
            return STRING("DovahKit needed to select a form ID to use for the new form. The chosen form ID is the target of one or more dangling uses, and DovahKit does not know how to sever those uses, so the form creation process could not continue.");
         case form_type_unavailable_in_current_game:
            return STRING("The desired form type doesn't exist in the version (e.g. Classic/Special) of Skyrim this file was created for. Try converting the file to the target Skyrim version first.");
      }
      return "";
   }

   namespace terse {
      extern QString form_creation_error_code(ENUMERATION_TYPE v) {
         using enum ENUMERATION_TYPE;
         switch (v) {
            case wrong_load_order:
               return STRING("internal program error: `wrong_load_order`");
            case no_active_file:
               return STRING("there is neither an active file nor room in the load order to create one");
            case invalid_form_type:
               return STRING("the requested form type is not valid");
            case unimplemented_form_type:
               return STRING("DovahKit doesn't yet support editing forms of this type");
            case invalid_parent_child_relationship:
               return STRING("the specified parent form cannot have a child form of this type");
            case cannot_create_reference_with_no_parent_cell:
               return STRING("a ref or other cell-child form must have a parent cell");
            case interior_cell_clone_cannot_have_parent:
               return STRING("interior cells are not allowed to have a parent form");
            case exterior_cell_clone_must_have_parent:
               return STRING("exterior cells must have a parent worldspace");
            case exterior_cell_must_have_grid_coordinates:
               return STRING("exterior cells must have grid coordinates specified");
            case do_not_use_worldspace_persistent_cell_as_parent:
               return STRING("do not use a worldspace's persistent cell as the parent cell of a cell-child form; pick the appropriate parent cell based on the intended coordinates of the cell-child form");
            case no_form_id_available:
               return STRING("not enough form IDs available in the active file");
            case exterior_grid_coordinates_already_taken:
               return STRING("the specified parent worldspace already has an exterior cell at the desired grid coordinates");
            case cannot_sever_references_to_none_stub:
               return STRING("form ID that DovahKit wants to use is the target of one or more dangling references, and at least one is outbound from a form that DovahKit doesn't yet know how to load");
            case form_type_unavailable_in_current_game:
               return STRING("the requested form type is unavailable in the current game");
         }
         return "";
      }
   }
}