#pragma once
#include <stdexcept>
#include <vector>

namespace cobb::impl::_node {
   template<typename Node>
   class children_view {
      friend typename Node;
      protected:
         Node& node;

         using base_list_type      = std::vector<std::remove_const_t<Node>*>;
         using iterator_type       = base_list_type::iterator;
         using const_iterator_type = base_list_type::const_iterator;

         constexpr Node::child_list& _list() {
            return node._children;
         }
         constexpr const Node::child_list& _list() const {
            return node._children;
         }

      public:
         constexpr iterator_type begin() requires (!std::is_const_v<Node>) {
            if constexpr (Node::can_any_node_have_children) {
               if (node.can_have_children())
                  return _list().begin();
            } else {
               return iterator_type{};
            }
         }
         constexpr iterator_type end() requires (!std::is_const_v<Node>) {
            if constexpr (Node::can_any_node_have_children) {
               if (node.can_have_children())
                  return _list().end();
            }
            return iterator_type{};
         }
         constexpr const_iterator_type begin() const {
            if constexpr (Node::can_any_node_have_children) {
               if (node.can_have_children())
                  return _list().begin();
            }
            return const_iterator_type{};
         }
         constexpr const_iterator_type end() const {
            if constexpr (Node::can_any_node_have_children) {
               if (node.can_have_children())
                  return _list().end();
            }
            return const_iterator_type{};
         }
         constexpr const_iterator_type cbegin() const {
            return begin();
         }
         constexpr const_iterator_type cend() const {
            return end();
         }

         constexpr const Node* operator[](size_t i) const {
            if constexpr (Node::can_any_node_have_children) {
               if (node.can_have_children())
                  return _list()[i];
            }
            throw std::out_of_range{};
         }
         constexpr Node* operator[](size_t i) {
            return const_cast<Node*>(std::as_const(*this).operator[](i));
         }
   };
}