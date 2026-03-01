#include "./ordered_child_collection.h"
#include <cassert>
#include "helpers/vectors/move_item_to_index.h"
#include "helpers/vectors/move_item_within.h"
#include "dovah/form_stubs/passkeys/build_use_info_during_load.h"
#include "./passkeys/ordered_child_collection.h"

namespace dovah::form_stub_addendum_types {
   void ordered_child_collection::move_child_after_index(size_t from, size_t to) {
      assert(from < this->active_file.size());
      assert(to   < this->active_file.size());
      if (from == to)
         return;
      cobb::vectors::move_item_after_index(this->active_file, from, to);
   }

   void ordered_child_collection::move_child_before_index(size_t from, size_t to) {
      assert(from < this->active_file.size());
      assert(to   < this->active_file.size());
      if (from == to)
         return;
      cobb::vectors::move_item_before_index(this->active_file, from, to);
   }

   void ordered_child_collection::replace_order(std::vector<form_stub*>&& src) {
      auto& dst = this->active_file;

      #if _DEBUG
      //
      // Verify that `src` and `dst` contain the same elements (even if in a 
      // different order).
      //
      {
         assert(dst.size() == src.size());
         const auto size = dst.size();
         for (size_t i = 0; i < size; ++i) {
            bool found = false;
            for (size_t j = 0; j < size; ++j) {
               if (dst[i] == src[j]) {
                  found = true;
                  break;
               }
            }
            assert(found);
         }
      }
      #endif

      std::swap(this->active_file, src);
   }

   void ordered_child_collection::_insert_child_on_load(form_stub_passkeys::build_use_info_during_load, bool is_active_file, form_stub& child, form_stub* after) {
      auto _insert = [&child, after](auto& list) {
         {
            auto it = std::find(list.begin(), list.end(), &child);
            if (it != list.end())
               list.erase(it);
         }
         size_t at = 0;
         if (after) {
            auto it = std::find(list.begin(), list.end(), after);
            if (it != list.end())
               at = std::distance(list.begin(), it) + 1;
         }
         list.insert(list.begin() + at, &child);
      };

      //
      // The `is_active_file` parameter indicates whether we're inserting a child 
      // after having loaded that child's record from the active file.
      // 
      // For non-active-file loads, we need to insert into both the master list 
      // and the active list. Why? Because if the child isn't overridden in the 
      // active file, it'll still *be* an ordered-child, so it should be present 
      // in both lists.
      //
      if (!is_active_file)
         _insert(this->dependencies);
      _insert(this->active_file);
   }
   void ordered_child_collection::_insert_child_on_load(form_stub_passkeys::build_use_info_during_load, bool is_active_file, form_stub& child) {
      auto _insert = [&child](auto& list) {
         auto it = std::find(list.begin(), list.end(), &child);
         if (it != list.end())
            list.erase(it);
         list.push_back(&child);
      };

      // See comment in the other `_insert_child_on_load` overload.
      if (!is_active_file)
         _insert(this->dependencies);
      _insert(this->active_file);
   }
   void ordered_child_collection::_remove_child_on_load(form_stub_passkeys::build_use_info_during_load, bool is_active_file, form_stub& child) {
      // See comment in `_insert_child_on_load`.
      if (!is_active_file)
         std::erase(this->dependencies, &child);
      std::erase(this->active_file, &child);
   }
   void ordered_child_collection::_insert_child_after_load(passkeys::reorder_children_after_load, form_stub& child, size_t at) {
      auto& list = this->active_file;
      auto  size = list.size();
      auto  it   = std::find(list.begin(), list.end(), &child);
      if (it == list.end()) {
         if (at >= size)
            at = size;
         list.insert(list.begin() + at, &child);
      } else {
         if (at >= size)
            at = size - 1;
         auto i = std::distance(list.begin(), it);
         cobb::vectors::move_item_within(list, i, (int)at - i);
      }
   }
   void ordered_child_collection::_remove_child_after_load(passkeys::reorder_children_after_load, form_stub& child) {
      std::erase(this->active_file, &child);
   }
   void ordered_child_collection::_sever_references_to_deleted_form(passkeys::reorder_children_after_load, form_stub& child, bool just_being_flagged) {
      std::erase(this->active_file, &child);
      if (!just_being_flagged) {
         //
         // If the form is being deleted from memory, then we need to remove it from the 
         // master list of ordered children in order to avoid a dangling pointer. However, 
         // if the form is just being *flagged* as deleted, then we need to keep it in 
         // that list, so that when we go to save the active file, we know which other 
         // INFOs to serialize PNAM subrecords for.
         //
         std::erase(this->dependencies, &child);
      }
   }
}