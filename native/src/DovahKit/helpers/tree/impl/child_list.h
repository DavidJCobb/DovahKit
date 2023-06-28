#pragma once
#include <stdexcept>
#include <vector>

namespace cobb::impl::_node {
   template<typename Node, typename Data>
   class child_list {
      friend typename Node;
      protected:
         std::vector<Node*> _list;

      public:
         template<typename Self> constexpr auto&& begin(this Self&& self) {
            return std::forward<Self>(self)._list.begin();
         }
         template<typename Self> constexpr auto&& end(this Self&& self) {
            return std::forward<Self>(self)._list.end();
         }
         template<typename Self> constexpr auto&& rbegin(this Self&& self) {
            return std::forward<Self>(self)._list.rbegin();
         }
         template<typename Self> constexpr auto&& rend(this Self&& self) {
            return std::forward<Self>(self)._list.rend();
         }
         constexpr auto cbegin() const noexcept {
            return this->_list.cbegin();
         }
         constexpr auto cend() const noexcept {
            return this->_list.cend();
         }
         constexpr auto crbegin() const noexcept {
            return this->_list.crbegin();
         }
         constexpr auto crend() const noexcept {
            return this->_list.crend();
         }

         template<typename Self>
         constexpr auto* at(this Self&& self, size_t i) {
            return std::forward<Self>(self)._list.at(i);
         }

         template<typename Self>
         constexpr auto* back(this Self&& self) {
            return std::forward<Self>(self)._list.back();
         }

         constexpr size_t capacity() const noexcept {
            return this->_list.capacity();
         }

         constexpr bool empty() const noexcept {
            return this->_list.empty();
         }

         template<typename Self>
         constexpr auto* front(this Self&& self) {
            return std::forward<Self>(self)._list.front();
         }

         constexpr size_t max_size() const noexcept {
            return this->_list.max_size();
         }

         constexpr void reserve(size_t n) {
            this->_list.reserve(n);
         }

         constexpr void shrink_to_fit() {
            this->_list.shrink_to_fit();
         }

         constexpr size_t size() const noexcept {
            return this->_list.size();
         }

         template<typename Self>
         constexpr auto* operator[](this Self&& self, size_t i) {
            return std::forward<Self>(self)._list[i];
         }
   };

   template<typename Node, typename Data> requires (Node::template data_type_is_leaf<Data>)
   class child_list<Node, Data> {
      protected:
         using vector_type = std::vector<Node*>;
         //
         using iterator_type = vector_type::iterator;
         using const_iterator_type = vector_type::const_iterator;
         using reverse_iterator_type = vector_type::reverse_iterator;
         using const_reverse_iterator_type = vector_type::const_reverse_iterator;

      public:
         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const_iterator_type, iterator_type> end(this Self&& self) {
            return iterator_type{};
         }
         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const_iterator_type, iterator_type> begin(this Self&& self) {
            return end();
         }

         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const_reverse_iterator_type, reverse_iterator_type> rend(this Self&& self) {
            return {};
         }
         template<typename Self>
         constexpr auto&& rbegin(this Self&& self) {
            return rend();
         }

         constexpr const_iterator_type cend() const noexcept {
            return {};
         }
         constexpr const_iterator_type cbegin() const noexcept {
            return cend();
         }
         constexpr const_reverse_iterator_type crend() const noexcept {
            return {};
         }
         constexpr const_reverse_iterator_type crbegin() const noexcept {
            return crend();
         }

         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const Node*, Node*> at(this Self&& self, size_t i) {
            throw std::out_of_range{};
         }

         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const Node*, Node*> back(this Self&& self) {
            throw std::out_of_range{};
         }

         constexpr size_t capacity() const noexcept {
            return 0;
         }

         constexpr bool empty() const noexcept {
            return true;
         }

         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const Node*, Node*> front(this Self&& self) {
            throw std::out_of_range{};
         }

         constexpr size_t max_size() const noexcept {
            return 0;
         }

         constexpr void reserve(size_t n) {}

         constexpr void shrink_to_fit() {}

         constexpr size_t size() const noexcept {
            return 0;
         }

         template<typename Self>
         constexpr std::conditional_t<std::is_const_v<Self>, const Node*, Node*> operator[](this Self&& self, size_t i) {
            throw std::out_of_range{};
         }
   };
}