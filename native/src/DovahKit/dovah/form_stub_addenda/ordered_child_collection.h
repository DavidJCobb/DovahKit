#pragma once
#include <vector>
namespace dovah {
   namespace form_stub_passkeys {
      class build_use_info_during_load;
   }
   namespace form_stub_addendum_types::passkeys {
      class reorder_children_during_load;
      class reorder_children_after_load;
   }
   class form_stub;
}

namespace dovah::form_stub_addendum_types {
   class ordered_child_collection {
      protected:
         //
         // Store the list of ordered children as of the last loaded master file, 
         // and the list as of the active file. If child forms are reordered within 
         // the active file, then the parent form will need to be saved to the active 
         // file even if it, itself, has not been edited; and we can detect this case 
         // by simply comparing the two lists.
         //
         std::vector<form_stub*> dependencies;
         std::vector<form_stub*> active_file;

      public:
         constexpr const std::vector<form_stub*>& get_master_list() const noexcept { return this->dependencies; }
         constexpr const std::vector<form_stub*>& get_active_list() const noexcept { return this->active_file; }

         // Indices should be bounds-checked by the caller; we assert that they are 
         // in-bounds.
         void move_child_after_index(size_t from, size_t to);

         // Indices should be bounds-checked by the caller; we assert that they are 
         // in-bounds.
         void move_child_before_index(size_t from, size_t to);

         // Change the order of the active list. The input vector must contain the 
         // exact same stubs as the stored vector; just in a different order. This 
         // function will assert as much.
         void replace_order(std::vector<form_stub*>&&);

      public: // passkeyed
         void _insert_child_on_load(form_stub_passkeys::build_use_info_during_load, bool is_active_file, form_stub& child); // appends
         void _insert_child_on_load(form_stub_passkeys::build_use_info_during_load, bool is_active_file, form_stub& child, form_stub* after);
         void _remove_child_on_load(form_stub_passkeys::build_use_info_during_load, bool is_active_file, form_stub& child);
         void _insert_child_after_load(passkeys::reorder_children_after_load, form_stub& child, size_t at); // `at` is clamped if out of bounds
         void _remove_child_after_load(passkeys::reorder_children_after_load, form_stub& child);
         void _sever_references_to_deleted_form(passkeys::reorder_children_after_load, form_stub& child, bool just_being_flagged);
   };
}