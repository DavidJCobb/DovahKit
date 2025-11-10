#include "./action_parent_node.h"
#include "helpers/vectors/re_sort_item_within.h"
#include "helpers/sort_and_remember.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "../idles.h"
#include "./action_node.h"
#include "./passkeys/check_is_building.h"
#include "./passkeys/idle_sorting.h"

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
      auto& callbacks   = this->datastore.callbacks.actions.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});
      assert(node_ptr != nullptr);
      assert(&node_ptr->datastore == &this->datastore);
      assert(node_ptr->parent == nullptr && "When using unique pointers, a node must be taken (`take_child`) from its parent before it can be appended!");
      auto& node = *node_ptr.get();
      if (should_fire && callbacks.before)
         (callbacks.before)(node, *this, this->children.size());
      this->children.emplace_back() = node_ptr.get();
      node_ptr.release();
      node.parent = this;
      if (should_fire && callbacks.after)
         (callbacks.after)(node);
   }
   void action_parent_node::append_child(action_node& node) {
      assert(&node.datastore == &this->datastore);
      if (node.parent == this)
         return;

      auto& callbacks   = this->datastore.callbacks.actions.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});

      if (should_fire && callbacks.before)
         (callbacks.before)(node, *this, this->children.size());
      auto& dst_ptr = this->children.emplace_back();
      if (node.parent) {
         auto i = node.parent->index_of_child(node);
         assert(i != no_index);
         auto src_ptr = node.parent->take_child(i);
         dst_ptr = src_ptr.release();
      } else {
         dst_ptr = &node;
      }
      node.parent = this;
      if (should_fire && callbacks.after)
         (callbacks.after)(node);
   }
   void action_parent_node::destroy_child(size_t i) {
      if (i >= this->children.size())
         throw std::out_of_range("Index out of range.");
      
      auto& callbacks   = this->datastore.callbacks.actions.on_deleted;
      bool  should_fire = !this->datastore._check_is_building({});

      action_node* node = this->children[i];
      assert(node != nullptr);
      form_stub*   stub = &node->stub;
      assert(stub != nullptr);

      if (should_fire && callbacks.before)
         (callbacks.before(*node));

      delete node;
      this->children.erase(this->children.begin() + i);

      if (should_fire && callbacks.after)
         (callbacks.after)(*stub);
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

      auto& callbacks   = this->datastore.callbacks.actions.on_taken;
      bool  should_fire = !this->datastore._check_is_building({});

      std::unique_ptr<action_node> node_ptr;
      action_node* node = this->children[i];
      assert(node != nullptr);
      if (should_fire && callbacks.before)
         (callbacks.before)(*node);
      this->children.erase(this->children.begin() + i);
      node_ptr.reset(node);
      if (should_fire && callbacks.after)
         (callbacks.after)(*node);
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
   void action_parent_node::sort_descendants(passkeys::idle_sorting passkey) {
      this->sort_children();
      for (action_node* child : this->children)
         child->sort_descendants(passkey);
   }

   void action_parent_node::re_sort_child(size_t n) {
      auto& node = *this->children[n];

      bool fired_before = false;
      cobb::vectors::re_sort_item_within(
         this->children,
         this->children.begin() + n,
         &action_parent_node::sort_comparator,
         [this, &fired_before, &node](auto from_it, auto to_it) {
            auto& callbacks   = this->datastore.callbacks.actions.on_placed;
            bool  should_fire = !this->datastore._check_is_building({});
            if (should_fire && callbacks.before) {
               fired_before = true;
               (callbacks.before)(node, *this, std::distance(this->children.begin(), to_it));
            }
         }
      );
      if (fired_before) {
         auto& callbacks = this->datastore.callbacks.actions.on_placed;
         if (callbacks.after)
            (callbacks.after)(node);
      }
   }
   void action_parent_node::re_sort_child(action_node& node) {
      auto i = this->index_of_child(node);
      assert(i != no_index);
      this->re_sort_child(i);
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
      {
         auto* node = this->action_by_stub(stub);
         if (node)
            return node;
      }
      
      auto& callbacks   = this->datastore.callbacks.actions.on_placed;
      bool  should_fire = !this->datastore._check_is_building({});

      auto  node_ptr = std::make_unique<action_node>(this->datastore, stub);
      auto& node     = *node_ptr;
      if (should_fire && callbacks.before)
         (callbacks.before)(node, *this, this->children.size());
      node_ptr->parent = this;
      this->children.emplace_back(node_ptr.get());
      node_ptr.release();
      if (should_fire && callbacks.after)
         (callbacks.after)(node);
      return &node;
   }
}