#pragma once
#include <cassert>
#include <concepts>
#include <cstdint>
#include <unordered_map>

namespace ui::model_utils {
   namespace impl::drag_drop_node_id_map {
      template<typename T>
      concept has_children_pointer_list = requires(T & node) {
         { **(node.children.begin()) } -> std::same_as<T&>;
         { **(node.children.end()) } -> std::same_as<T&>;
      };

      template<typename T>
      concept has_children_getter = requires(T & node) {
         { **(node.children().begin()) } -> std::same_as<T&>;
         { **(node.children().end()) } -> std::same_as<T&>;
      };
   }

   template<typename Node>
   class drag_drop_node_id_map {
      public:
         using node_type = Node;
         using uid_type  = uint64_t;

      public:
         uid_type next_id = 0;
         std::unordered_map<uid_type, node_type*> nodes;
         
      public:
         uid_type track(node_type& node) {
            for (const auto& pair : this->nodes)
               if (pair.second == &node)
                  return pair.first;
            auto id = this->next_id;
            this->next_id++;
            assert(this->next_id != 0 && "overflow of a UI model's drag-and-drop UID counter!");
            this->nodes[id] = &node;
            return id;
         }
         void untrack(node_type& node) {
            auto& map = this->nodes;
            auto  it  = std::find_if(map.begin(), map.end(), [&node](const auto& pair) {
               return pair.second == &node;
            });
            if (it != map.end())
               map.erase(it);
         }
         void clear() {
            this->nodes.clear();
            // we intentionally DO NOT reset the "next ID" field
         }
         node_type* get_by_id(uid_type id) {
            auto& map = this->nodes;
            auto  it  = map.find(id);
            if (it != map.end())
               return it->second;
            return nullptr;
         }

         void on_node_destroyed(node_type& node) {
            //
            // We attempt to handle the most basic case of tree nodes here: when 
            // a node is destroyed, you have to untrack not just the node itself, 
            // but any descendants as well. More complicated node/tree structures 
            // may require you to handle that manually.
            // 
            // We handle the cases of:
            // 
            //  - `node.children` is an iterable container of either bare or 
            //    smart pointers to child nodes.
            // 
            //  - `node.children()` is a getter which returns iterable access (e.g. 
            //    a container reference, or a range/view) to a list of bare or 
            //    smart pointers to child nodes.
            // 
            // If your model contains heterogenous node types, where only some of 
            // those types have child lists (e.g. you have a common base class and 
            // only some subclasses have child lists; or you have a single type and 
            // use a std::variant to store sub-type data, including child lists), 
            // then you will need to manually walk and untrack subtrees.
            //
            if constexpr (impl::drag_drop_node_id_map::has_children_pointer_list<node_type>) {
               [this](this auto&& recurse, node_type& node) -> void {
                  this->untrack(node);
                  for (auto& child_ptr : node.children)
                     recurse(*child_ptr);
               }(node);
            } else if constexpr (impl::drag_drop_node_id_map::has_children_getter<node_type>) {
               [this](this auto&& recurse, node_type& node) -> void {
                  this->untrack(node);
                  for (auto& child_ptr : node.children())
                     recurse(*child_ptr);
               }(node);
            } else {
               this->untrack(node);
            }
         }
   };
}