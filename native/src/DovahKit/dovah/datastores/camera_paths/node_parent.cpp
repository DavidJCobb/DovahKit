#pragma once
#include "./node_parent.h"
#include <cassert>
#include "helpers/vectors/move_item_to_index.h"
#include "./node.h"
#include "./passkeys/initial_build.h"

namespace dovah::datastores::impl::camera_paths {
   size_t node_parent::index_of_child(const form_stub& stub) const noexcept {
      const auto& list = this->children;
      const auto  size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (&list[i]->stub == &stub)
            return i;
      return index_of_none;
   }
   bool node_parent::is_ancestor_of(const node& n) const noexcept {
      const node_parent* parent = n.parent;
      if (!parent)
         return false;
      const node* p_node = dynamic_cast<const node*>(parent);
      while (p_node) {
         if (p_node == this)
            return true;
         p_node = dynamic_cast<const node*>((const node_parent*)p_node->parent);
      }
      return false;
   }
   
   void node_parent::_insert_sorted_child(passkeys::initial_build passkey, node& subject) {
      //
      // Insert the idle after its desired previous sibling, if said sibling 
      // is non-null and is already in our child list.
      //
      {
         node* desired_prev = subject._get_sort_state(passkey).previous;
         if (desired_prev) {
            auto i = this->index_of_child(*desired_prev);
            if (i != index_of_none)
               this->children.insert(this->children.begin() + i + 1, &subject);
            else
               this->children.push_back(&subject);
         } else {
            this->children.push_back(&subject);
         }
         subject.parent = this;
      }
      //
      // This idle may potentially be the desired previous sibling of an idle 
      // that was inserted earlier, so crawl the list and reorder the desired 
      // next sibling(s) as appropriate.
      //
      node* current_node = &subject;
      node* next_node    = nullptr;
      do {
         size_t current_index = index_of_none;
         size_t next_index    = index_of_none;
         for (size_t i = 0; i < this->children.size(); ++i) {
            node* candidate = this->children[i];
            if (candidate == current_node) {
               current_index = i;
               continue;
            }
            if (candidate->_get_sort_state(passkey).previous == current_node) {
               if (next_node) {
                  current_node->_get_sort_state(passkey).multiple_next_siblings = true;
               } else {
                  next_node  = candidate;
                  next_index = i;
               }
            }
         }
         if (next_index == index_of_none) // no next idle?
            break;
         if (next_index == current_index + 1) // next idle is already where it should be?
            break;
         assert(next_node != nullptr);
         cobb::vectors::move_item_after_index(
            this->children,
            next_index,
            current_index
         );
         //
         // Move on to next.
         //
         current_node = next_node;
         next_node    = nullptr;
      } while (current_node);
   }
}