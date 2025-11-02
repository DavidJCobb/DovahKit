#include "./idle_parent_node.h"
#include "helpers/vectors/move_item_within.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./idle_node.h"
#include "./passkeys/idle_sorting.h"
#include "./warnings/idle_has_multiple_next_siblings.h"

namespace dovah::datastores::impl::idles {
   idle_parent_node::~idle_parent_node() {
      for (auto& ptr : this->children) {
         if (!ptr)
            continue;
         delete ptr;
         ptr = nullptr;
      }
   }

   void idle_parent_node::append_child(std::unique_ptr<idle_node>&& node_ptr) {
      assert(node_ptr != nullptr);
      assert(node_ptr->parent == nullptr && "When using unique pointers, a node must be taken (`take_child`) from its parent before it can be appended!");
      auto& node = *node_ptr.get();
      this->children.emplace_back() = node_ptr.get();
      node_ptr.release();
      node.parent = this;
   }
   void idle_parent_node::append_child(idle_node& node) {
      if (node.parent) {
         if (node.parent == this)
            return;
         auto i = node.parent->index_of_child(node);
         assert(i != no_index);
         auto& dst_ptr = this->children.emplace_back();
         auto  src_ptr = node.parent->take_child(i);
         dst_ptr = src_ptr.release();
         return;
      }
      this->children.emplace_back() = &node;
      node.parent = this;
   }
   void idle_parent_node::destroy_child(size_t i) {
      if (i >= this->children.size())
         throw std::out_of_range("Index out of range.");
      idle_node* node = this->children[i];
      if (node)
         delete node;
      this->children.erase(this->children.begin() + i);
   }
   size_t idle_parent_node::index_of_child(const form_stub& stub) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i) {
         const idle_node* node = this->children[i];
         if (&node->stub == &stub)
            return i;
      }
      return no_index;
   }
   std::unique_ptr<idle_node> idle_parent_node::take_child(size_t i) {
      if (i >= this->children.size())
         throw std::out_of_range("Index out of range.");
      std::unique_ptr<idle_node> node_ptr;
      idle_node* node = this->children[i];
      this->children.erase(this->children.begin() + i);
      node_ptr.reset(node);
      return node_ptr;
   }

   void idle_parent_node::sort_children() {
      auto&      list = this->children;
      const auto size = list.size();
      
      size_t i = 0;
      do {
         idle_node* subject  = list[i];
         idle_node* previous = subject->_get_sort_state({}).previous_idle;
         if (!previous) {
            ++i;
            continue;
         }
         auto prev_it = std::find(list.begin(), list.end(), previous);
         if (prev_it == list.end()) {
            ++i;
            continue;
         }
         if (i > 0 && prev_it == (list.begin() + i - 1)) {
            ++i;
            continue;
         }
         //
         // Move the current element to after its intended previous sibling.
         //
         size_t offset_to_destination;
         {
            auto from = list.begin() + i;
            auto to   = prev_it + 1;
            offset_to_destination = std::distance(from, to);
         }
         cobb::vectors::move_item_within(
            list,
            i,
            offset_to_destination
         );
      } while (i < size);
   }
   void idle_parent_node::sort_descendants() {
      this->sort_children();
      for (idle_node* child : this->children)
         child->sort_descendants();
   }

   void idle_parent_node::insert_sorted_child(passkeys::idle_sorting, idle_node& subject, std::vector<warning*>& warnings) {
      this->append_child(subject);

      idle_node* current_node = &subject;
      idle_node* next_node    = nullptr;
      do {
         size_t current_index = no_index;
         size_t next_index    = no_index;
         for (size_t i = 0; i < this->children.size(); ++i) {
            idle_node* subject = this->children[i];
            if (subject == current_node) {
               current_index = i;
               continue;
            }
            if (subject->_get_sort_state({}).previous_idle == current_node) {
               if (next_node) {
                  auto& dst = warnings.emplace_back();
                  dst = new warnings::idle_has_multiple_next_siblings(*current_node);
               } else {
                  next_node  = subject;
                  next_index = i;
               }
            }
         }
         if (next_index == no_index) // no next idle?
            break;
         if (next_index == current_index + 1) // next idle is already where it should be?
            break;
         cobb::vectors::move_item_within(
            this->children,
            next_index,
            (int)(current_index) - next_index
         );
         //
         // Move on to next.
         //
         current_node = next_node;
         next_node    = nullptr;
      } while (current_node);
   }
}