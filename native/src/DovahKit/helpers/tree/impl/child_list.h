#pragma once
#include <stdexcept>
#include <vector>

namespace cobb::impl::_node {
   template<typename Node>
   class child_list {
      friend typename Node;
      protected:
         std::vector<Node*> _list;

      public:
         template<typename Self> constexpr auto begin(this Self&& self) {
            return std::forward<Self>(self)._list.begin();
         }
         template<typename Self> constexpr auto end(this Self&& self) {
            return std::forward<Self>(self)._list.end();
         }
         template<typename Self> constexpr auto rbegin(this Self&& self) {
            return std::forward<Self>(self)._list.rbegin();
         }
         template<typename Self> constexpr auto rend(this Self&& self) {
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
}