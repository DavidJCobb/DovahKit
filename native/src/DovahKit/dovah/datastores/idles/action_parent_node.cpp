#include "./action_parent_node.h"
#include "./action_node.h"
#include "helpers/sort_and_remember.h"
#include "../../form_stub.h"
#include "../../form_types.h"

namespace dovah::datastores::impl::idles {
   action_parent_node::~action_parent_node() {
      for (auto& ptr : this->children) {
         if (!ptr)
            continue;
         delete ptr;
         ptr = nullptr;
      }
   }

   /*static*/ bool action_parent_node::sort_comparator(const action_node* a, const action_node* b) {
      //
      // Case-insensitive sort by editor ID.
      //
      if (!a)
         return false;
      if (!b)
         return true;
      const auto&  a_name   = a->stub.editorID;
      const auto&  b_name   = b->stub.editorID;
      const size_t min_size = std::min(a_name.size(), b_name.size());
      for (size_t i = 0; i < min_size; ++i) {
         char ca = a_name[i];
         char cb = b_name[i];
         if (ca >= 'a' && ca <= 'z')
            ca -= 0x20;
         if (cb >= 'a' && cb <= 'z')
            cb -= 0x20;
         if (ca != cb)
            return ca < cb;
      }
      return (a_name.size() < b_name.size());
   }

   void action_parent_node::append_child(std::unique_ptr<action_node>&& node_ptr) {
      assert(node_ptr != nullptr);
      assert(node_ptr->parent == nullptr && "When using unique pointers, a node must be taken (`take_child`) from its parent before it can be appended!");
      auto& node = *node_ptr.get();
      this->children.emplace_back() = node_ptr.get();
      node_ptr.release();
      node.parent = this;
   }
   void action_parent_node::append_child(action_node& node) {
      if (node.parent) {
         if (node.parent == this)
            return;
         auto i = node.parent->index_of_child(node);
         assert(i != no_index);
         auto& dst_ptr = this->children.emplace_back();
         auto  src_ptr = node.parent->take_child(i);
         dst_ptr = src_ptr.release();
         node.parent = this;
         return;
      }
      this->children.emplace_back() = &node;
      node.parent = this;
   }
   void action_parent_node::destroy_child(size_t i) {
      if (i >= this->children.size())
         throw std::out_of_range("Index out of range.");
      action_node* node = this->children[i];
      if (node)
         delete node;
      this->children.erase(this->children.begin() + i);
   }
   size_t action_parent_node::index_of_child(const form_stub& stub) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i) {
         const action_node* node = this->children[i];
         if (&node->stub == &stub)
            return i;
      }
      return no_index;
   }
   std::unique_ptr<action_node> action_parent_node::take_child(size_t i) {
      if (i >= this->children.size())
         throw std::out_of_range("Index out of range.");
      std::unique_ptr<action_node> node_ptr;
      action_node* node = this->children[i];
      this->children.erase(this->children.begin() + i);
      node_ptr.reset(node);
      return node_ptr;
   }

   void action_parent_node::sort_children() {
      //
      // Case-insensitive sort by editor ID.
      //
      std::sort(
         this->children.begin(),
         this->children.end(),
         sort_comparator
      );
   }
   [[nodiscard]] std::vector<size_t> action_parent_node::sort_children_and_remember() {
      return cobb::sort_and_remember(
         this->children,
         sort_comparator
      );
   }
   void action_parent_node::sort_descendants() {
      this->sort_children();
      for (action_node* child : this->children)
         child->sort_descendants();
   }

   const action_node* action_parent_node::action_by_stub(const form_stub& action) const noexcept {
      if (action.form_type != form_type::action)
         return nullptr;
      for (const action_node* node : this->children)
         if (&node->stub == &action)
            return node;
      return nullptr;
   }
   action_node* action_parent_node::action_by_stub(const form_stub& action) noexcept {
      return const_cast<action_node*>(std::as_const(*this).action_by_stub(action));
   }

   action_node* action_parent_node::get_or_create_action(form_stub& stub) {
      auto* node = this->action_by_stub(stub);
      if (node)
         return node;
      auto node_ptr = std::make_unique<action_node>(stub);
      node_ptr->parent = this;
      this->children.emplace_back(node_ptr.get());
      return node_ptr.release();
   }
}