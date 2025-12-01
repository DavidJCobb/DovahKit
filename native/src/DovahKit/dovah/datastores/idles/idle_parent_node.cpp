#include "./idle_parent_node.h"
#include "helpers/vectors/move_item_to_index.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../idles.h"
#include "./idle_node.h"
#include "./passkeys/check_is_building.h"
#include "./passkeys/idle_sorting.h"
#include "./passkeys/push_idle_hierarchy_position_to_form.h"
#include "./warnings/idle_has_multiple_next_siblings.h"

namespace dovah::datastores::impl::idles {
   idle_parent_node::~idle_parent_node() {
   }

   void idle_parent_node::append_child(std::unique_ptr<idle_node>&& node_ptr) {
      auto& callbacks   = this->datastore.callbacks.idles.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});
      assert(node_ptr != nullptr);
      assert(&node_ptr->datastore == &this->datastore);
      assert(node_ptr->parent == nullptr && "When using unique pointers, a node must be taken (`take_child`) from its parent before it can be appended!");

      auto& node = *node_ptr;

      size_t dst_index;
      if (should_fire) {
         dst_index = this->children.size();
         if (callbacks.before)
            (callbacks.before)(node, *this, dst_index);
      }
      this->children.emplace_back() = node_ptr.get();
      node_ptr.release();
      node.parent = this;
      if (should_fire) {
         this->_on_parent_changed(node, dst_index);
         if (callbacks.after)
            (callbacks.after)(node);
      }
   }
   void idle_parent_node::append_child(idle_node& node) {
      assert(&node.datastore == &this->datastore);

      size_t src_index = 0;
      size_t dst_index = this->children.size();
      if (node.parent == this) {
         src_index = this->index_of_child(node);
         --dst_index;
         if (src_index == dst_index)
            return;
      }

      auto& callbacks   = this->datastore.callbacks.idles.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});

      if (should_fire && callbacks.before)
         (callbacks.before)(node, *this, dst_index);
      auto& dst_ptr = this->children.emplace_back();
      if (node.parent) {
         if (node.parent == this) {
            cobb::vectors::move_item_to_index(this->children, src_index, dst_index);
         } else {
            auto i = node.parent->index_of_child(node);
            assert(i != no_index);
            auto src_ptr = node.parent->take_child(i);
            dst_ptr = src_ptr.release();
         }
      } else {
         dst_ptr = &node;
      }
      node.parent = this;
      if (should_fire) {
         this->_on_parent_changed(node, dst_index);
         if (callbacks.after)
            (callbacks.after)(node);
      }
   }
   bool idle_parent_node::contains(const idle_node& node) const noexcept {
      const idle_parent_node* current = node.parent;
      do {
         if (current == this)
            return true;
         //
         if (auto* casted = dynamic_cast<const idle_node*>(current)) {
            current = casted->parent;
         } else {
            current = nullptr;
         }
      } while (current);
      return false;
   }
   size_t idle_parent_node::index_of_child(const form_stub& stub) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i) {
         const idle_node* node = this->children[i];
         if (&node->stub == &stub)
            return i;
      }
      return no_index;
   }
   void idle_parent_node::insert_child_before(idle_node& idle, size_t before) {
      assert(&idle.datastore == &this->datastore);

      if (before > this->children.size()) {
         throw std::out_of_range("Out-of-bounds index to move an idle after");
      }
      
      auto& callbacks   = this->datastore.callbacks.idles.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});

      if (idle.parent == this) {
         size_t from = this->index_of_child(idle);
         size_t to   = before - (from < before ? 1 : 0);
         if (before == from + 1)
            return;
         this->_move_child_to_index(from, to);
      } else {
         this->_adopt_new_child_to_index(idle, before);
      }
   }
   void idle_parent_node::insert_child_after(idle_node& idle, size_t after) {
      assert(&idle.datastore == &this->datastore);

      if (after >= this->children.size()) {
         throw std::out_of_range("Out-of-bounds index to move an idle after");
      }
      
      auto& callbacks   = this->datastore.callbacks.idles.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});

      if (idle.parent == this) {
         size_t from = this->index_of_child(idle);
         size_t to   = after + (after >= from ? 1 : 0);
         if (after == from + 1)
            return;
         this->_move_child_to_index(from, to);
      } else {
         this->_adopt_new_child_to_index(idle, after + 1);
      }
   }
   std::unique_ptr<idle_node> idle_parent_node::take_child(size_t i) {
      if (i >= this->children.size())
         throw std::out_of_range("Index out of range.");

      auto& callbacks   = this->datastore.callbacks.idles.on_taken;
      bool  should_fire = !this->datastore._check_is_building({});

      std::unique_ptr<idle_node> node_ptr;
      idle_node* node = this->children[i];
      assert(node != nullptr);
      if (should_fire && callbacks.before)
         (callbacks.before)(*node);
      this->children.erase(this->children.begin() + i);
      node_ptr.reset(node);
      if (should_fire) {
         this->_on_previous_sibling_changed(i);
         if (callbacks.after)
            (callbacks.after)(*node);
      }
      return node_ptr;
   }

   void idle_parent_node::sort_children(passkeys::idle_sorting) {
      assert(this->datastore._check_is_building({}) == true);

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
         size_t previous_i = std::distance(list.begin(), prev_it);
         cobb::vectors::move_item_after_index(list, i, previous_i);
      } while (i < size);
   }
   void idle_parent_node::sort_descendants(passkeys::idle_sorting) {
      this->sort_children({});
      for (idle_node* child : this->children)
         child->sort_descendants({});
   }

   void idle_parent_node::insert_sorted_child(passkeys::idle_sorting, idle_node& subject, std::vector<warning*>& warnings) {
      assert(this->datastore._check_is_building({}) == true);
      //
      // Insert the idle after its desired previous sibling, if said sibling 
      // is non-null and is already in our child list.
      //
      {
         idle_node* desired_prev = subject._get_sort_state({}).previous_idle;
         if (desired_prev) {
            auto i = this->index_of_child(*desired_prev);
            if (i != no_index)
               this->insert_child_after(subject, i);
            else
               this->append_child(subject);
         } else {
            this->append_child(subject);
         }
      }
      //
      // This idle may potentially be the desired previous sibling of an idle 
      // that was inserted earlier, so crawl the list and reorder the desired 
      // next sibling(s) as appropriate.
      //
      idle_node* current_node = &subject;
      idle_node* next_node    = nullptr;
      do {
         size_t current_index = no_index;
         size_t next_index    = no_index;
         for (size_t i = 0; i < this->children.size(); ++i) {
            idle_node* candidate = this->children[i];
            if (candidate == current_node) {
               current_index = i;
               continue;
            }
            if (candidate->_get_sort_state({}).previous_idle == current_node) {
               if (next_node) {
                  auto& dst = warnings.emplace_back();
                  dst = new warnings::idle_has_multiple_next_siblings(*current_node);
               } else {
                  next_node  = candidate;
                  next_index = i;
               }
            }
         }
         if (next_index == no_index) // no next idle?
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

   void idle_parent_node::_move_child_to_index(size_t from, size_t to) {
      assert(from < this->children.size());
      assert(to   < this->children.size());
      auto& callbacks   = this->datastore.callbacks.idles.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});
      
      auto& idle = *this->children[from];

      idle_node* prior_next_sibling = nullptr;
      if (should_fire) {
         if (callbacks.before)
            (callbacks.before)(idle, *this, to);
         if (from + 1 < this->children.size())
            prior_next_sibling = this->children[from + 1];
      }
      cobb::vectors::move_item_to_index(this->children, from, to);
      if (should_fire) {
         if (prior_next_sibling)
            this->_on_previous_sibling_changed(*prior_next_sibling);
         this->_on_previous_sibling_changed(idle);
         if (callbacks.after)
            (callbacks.after)(idle);
      }
   }
   void idle_parent_node::_adopt_new_child_to_index(idle_node& idle, size_t at) {
      assert(&idle.datastore == &this->datastore);
      assert(idle.parent != this);
      assert(at <= this->children.size()); // intentionally <=, to allow insertion at the end
      auto& callbacks   = this->datastore.callbacks.idles.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});

      if (should_fire && callbacks.before)
         (callbacks.before)(idle, *this, at);
      auto  dst_it  = this->children.insert(this->children.begin() + at, nullptr);
      auto& dst_ptr = *dst_it;
      if (idle.parent) {
         auto i = idle.parent->index_of_child(idle);
         assert(i != no_index);
         auto src_ptr = idle.parent->take_child(i);
         dst_ptr = src_ptr.release();
      } else {
         dst_ptr = &idle;
      }
      idle.parent = this;
      if (should_fire) {
         this->_on_parent_changed(idle, at);
         this->_on_previous_sibling_changed(at + 1);
         if (callbacks.after)
            (callbacks.after)(idle);
      }
   }

   void idle_parent_node::_on_previous_sibling_changed(size_t subject_index) {
      assert(!this->datastore._check_is_building({}) && "Do not do hierarchy updates during an initial datastore build!");
      if (subject_index >= this->children.size())
         return;
      this->children[subject_index]->_on_hierarchy_changed({}, subject_index);
   }
   void idle_parent_node::_on_previous_sibling_changed(idle_node& subject) {
      assert(!this->datastore._check_is_building({}) && "Do not do hierarchy updates during an initial datastore build!");
      subject._on_hierarchy_changed({});
   }
   void idle_parent_node::_on_parent_changed(idle_node& subject, size_t subject_index) {
      assert(!this->datastore._check_is_building({}) && "Do not do hierarchy updates during an initial datastore build!");
      subject._on_hierarchy_changed({}, subject_index);
   }
}