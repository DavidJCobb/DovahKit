#pragma once
#include <cassert>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace cobb {
   struct sorted_list_options {
      bool bidirectional = true;
      bool track_size    = false;

      constexpr sorted_list_options() {}
   };

   template<typename Key, typename Value, sorted_list_options Options = sorted_list_options{}> class sorted_list {
      private:
         struct _no_op {
            constexpr _no_op() {}
            template<typename T> constexpr _no_op(T) {}

            template<typename T> constexpr _no_op& operator=(const T&) { return *this; }
         };

      public:
         using key_type    = Key;
         using mapped_type = Value;
         using value_type  = std::pair<const Key, Value>;
         static constexpr sorted_list_options options = Options;
         
      protected:
         struct node_object {
            node_object* next = nullptr;
            std::conditional_t<options.bidirectional, node_object*, _no_op> prev = nullptr;
            value_type data;

            node_object(const key_type& k) : data({ k, mapped_type() }) {}
            node_object(key_type&& k) : data({ std::move(k), mapped_type() }) {}
            template<typename... Args> node_object(Args&&... args) : data(std::forward<Args&&>(args)...) {}
         };

      public:
         class node_type {
            friend class sorted_list;
            private:
               node_object* data = nullptr;

               node_type(node_object* n) : data(n) {}

            public:
               node_type() {}

               [[nodiscard]] inline bool empty() const noexcept { return data == nullptr; }
               inline value_type& value() const { return data->data; }
               inline key_type& key() const { return const_cast<key_type&>(data->data.first); }
               inline mapped_type& mapped() const { return data->data.second; }

               operator bool() const { return !empty(); }
         };

      public:
         class iterator {
            friend class sorted_list;
            public:
               using iterator_category = std::forward_iterator_tag;
               using value_type        = sorted_list::value_type;
               using pointer           = value_type*;
               using reference         = value_type&;
            private:
               node_object* node = nullptr;

               iterator(node_object* p) : node(p) {}

            public:
               reference operator*() const { return node->data; }
               pointer operator->() { return &node->data; }

               iterator& operator++() {
                  node = node->next;
                  return *this;
               }
               iterator operator++(int) { // postfix
                  iterator out = *this;
                  ++(*this);
                  return out;
               }

               iterator& operator--() requires (options.bidirectional) {
                  node = node->prev;
                  return *this;
               }
               iterator operator--(int) requires (options.bidirectional) { // postfix
                  iterator out = *this;
                  --(*this);
                  return out;
               }

               friend bool operator==(const iterator& a, const iterator& b) noexcept { return a.node == b.node; };
               friend bool operator!=(const iterator& a, const iterator& b) noexcept { return a.node != b.node; };
         };
         class const_iterator {
            friend class sorted_list;
            public:
               using iterator_category = std::forward_iterator_tag;
               using value_type        = sorted_list::value_type;
               using pointer           = const value_type*;
               using reference         = const value_type&;
            private:
               node_object* node = nullptr;

               const_iterator(node_object* p) noexcept : node(p) {}

            public:
               reference operator*() const { return node->data; }
               pointer operator->() { return &node->data; }

               const_iterator& operator++() {
                  node = node->next;
                  return *this;
               }
               const_iterator operator++(int) { // postfix
                  const_iterator out = *this;
                  ++(*this);
                  return out;
               }

               const_iterator& operator--() requires (options.bidirectional) {
                  node = node->prev;
                  return *this;
               }
               const_iterator operator--(int) requires (options.bidirectional) { // postfix
                  const_iterator out = *this;
                  --(*this);
                  return out;
               }

               friend bool operator==(const const_iterator& a, const const_iterator& b) noexcept { return a.node == b.node; };
               friend bool operator!=(const const_iterator& a, const const_iterator& b) noexcept { return a.node != b.node; };
         };

      protected:

         node_object* _first = nullptr;
         std::conditional_t<options.track_size, size_t, _no_op> _count = 0;

         void _insert_after(node_object* next, node_object* prev) {
            if constexpr (options.track_size) {
               ++this->count;
            }
            if (prev) {
               next->next = prev->next;
               prev->next = next;
            } else {
               next->next  = this->_first;
               this->_first = next;
            }
            if constexpr (options.bidirectional) {
               next->prev = prev;
            }
         }
         
         // returns iterator to next
         iterator _delete_node(node_object* kill) {
            node_object* next = kill->next;
            node_object* prev;
            if constexpr (options.bidirectional) {
               prev = kill->prev;
            } else {
               node_object* p = nullptr;
               for (auto* node = this->_first; node; p = node, node = node->next) {
                  if (node == kill) {
                     prev = p;
                     break;
                  }
               }
               if (!prev)
                  assert(this->_first == kill);
            }
            if (kill == this->_first)
               this->_first = next;
            if (prev)
               prev->next = next;
            if (next)
               next->prev = prev;
            delete kill;
            return iterator(next);
         }

      public:
         sorted_list() {}
         sorted_list(sorted_list&& o) noexcept {
            std::swap(this->_first, o._first);
            if constexpr (options.track_size) {
               std::swap(this->count, o._count);
            }
         }
         sorted_list(const sorted_list& o) {
            if (o.empty())
               return;
            this->_first       = new node_object;
            this->_first->data = o._first->data;
            //
            auto* src = o._first->next;
            auto* dst = this->_first; // dst is one node behind src
            while (src) {
               dst->next       = new node_object;
               dst->next->data = src->data;
               if constexpr (options.bidirectional) {
                  dst->next->prev = dst;
               }
               src = src->next;
            }
            if constexpr (options.track_size) {
               this->count = o._count;
            }
         }

         sorted_list& operator=(sorted_list&& o) noexcept {
            std::swap(this->_first, o._first);
            if constexpr (options.track_size) {
               std::swap(this->count, o._count);
            }
            return *this;
         }
         sorted_list& operator=(const sorted_list& o) {
            this->clear();
            if (o.empty())
               return;
            this->_first       = new node_object;
            this->_first->data = o._first->data;
            //
            auto* src = o._first->next;
            auto* dst = this->_first; // dst is one node behind src
            while (src) {
               dst->next       = new node_object;
               dst->next->data = src->data;
               if constexpr (options.bidirectional) {
                  dst->next->prev = dst;
               }
               src = src->next;
            }
            if constexpr (options.track_size) {
               this->count = o._count;
            }
            //
            return *this;
         }

         iterator begin() noexcept { return iterator(this->_first); }
         iterator end() noexcept { return iterator(nullptr); }
         const_iterator begin() const noexcept { return const_iterator(this->_first); }
         const_iterator end() const noexcept { return const_iterator(nullptr); }
         const_iterator cbegin() const noexcept { return const_iterator(this->_first); }
         const_iterator cend() const noexcept { return const_iterator(nullptr); }

         inline bool empty() const noexcept { return this->_first == nullptr; }
         size_t size() const noexcept {
            if constexpr (options.track_size) {
               return this->count;
            } else {
               size_t count = 0;
               for (auto* node = this->_first; node; node = node->next)
                  ++count;
               return count;
            }
         }

         void clear() {
            auto* node = this->_first;
            this->_first = nullptr;
            if constexpr (options.track_size) {
               this->count = 0;
            }
            while (node) {
               auto* kill = node;
               node = node->next;
               delete kill;
            }
         }
         bool contains(const key_type& k) const {
            for (auto* node = this->_first; node; node = node->next) {
               if (node->data.first == k)
                  return true;
               if (node->data.first > k)
                  return false;
            }
            return false;
         }
         inline size_t count(const key_type& k) const {
            return this->contains(k) ? 1 : 0;
         }

         mapped_type& operator[](const key_type& k) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k)
                  return node->data.second;
               if (node->data.first > k)
                  break;
            }
            auto* next = new node_object(k);
            this->_insert_after(next, prev);
            return next->data.second;
         }
         mapped_type& operator[](key_type&& k) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k)
                  return node->data.second;
               if (node->data.first > k)
                  break;
            }
            auto* next = new node_object(k);
            this->_insert_after(next, prev);
            return next->data.second;
         }

         value_type& at(const key_type& k) {
            for (auto* node = this->_first; node; node = node->next) {
               if (node->data.first == k)
                  return node->value;
               if (node->data.first > k)
                  break;
            }
            throw std::out_of_range();
         }
         const value_type& at(const key_type& k) const {
            for (auto* node = this->_first; node; node = node->next) {
               if (node->data.first == k)
                  return node->value;
               if (node->data.first > k)
                  break;
            }
            throw std::out_of_range();
         }

         node_type extract(iterator it) {
            if (!it.node)
               return nullptr;
            if constexpr (options.bidirectional) {
               auto* node = it.node;
               auto* next = node->next;
               auto* prev = node->prev;
               if (this->_first == node)
                  this->_first = next;
               if (next)
                  next->prev = prev;
               if (prev)
                  prev->next = next;
               return node;
            }
            return this->extract(it.node->data.first);
         }
         node_type extract(const key_type& k) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k) {
                  if (this->_first == node)
                     this->_first = node->next;
                  if constexpr (options.bidirectional) {
                     if (node->next)
                        node->next->prev = node->prev;
                  }
                  prev->next = node->next;
                  return node;
               }
               if (node->data.first > k)
                  break;
            }
            return nullptr;
         }

         iterator erase(iterator n) {
            return this->_delete_node(n.node);
         }
         iterator erase(const_iterator n) {
            return this->_delete_node(n.node);
         }
         size_t erase(const key_type& k) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k) {
                  if (node == this->_first)
                     this->_first = node->next;
                  else if (prev)
                     prev->next = node->next;
                  if constexpr (options.bidirectional) {
                     if (node->next)
                        node->next->prev = prev;
                  }
                  delete node;
                  return 1;
               }
            }
            return 0;
         }

         std::pair<iterator, bool> insert(const key_type& k, const value_type& v) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k)
                  return { iterator(node), false };
               if (node->data.first > k)
                  break;
            }
            auto* next = new node_object(k, v);
            this->_insert_after(next, prev);
            return { iterator(next), true };
         }
         std::pair<iterator, bool> insert(const key_type& k, value_type&& v) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k)
                  return { iterator(node), false };
               if (node->data.first > k)
                  break;
            }
            auto* next = new node_object(k, v);
            this->_insert_after(next, prev);
            return { iterator(next), true };
         }
         std::pair<iterator, bool> insert(node_type arg) {
            auto* next = arg.data;
            //
            assert(!next->next);
            if constexpr (options.bidirectional) {
               assert(!next->prev);
            }
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == next->data.first)
                  return { iterator(node), false };
               if (node->data.first > next->data.first)
                  break;
            }
            this->_insert_after(next, prev);
            return { iterator(next), true };
         }

         template<class M> std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& v) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k) {
                  node->value = std::forward<M>(v);
                  return { iterator(node), false };
               }
               if (node->data.first > k)
                  break;
            }
            auto* next = this->_insert_after(prev);
            next->data = value_type(k, std::forward<M>(v));
            return { iterator(next), true };
         }
         template<class M> std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& v) {
            node_object* prev = nullptr;
            for (auto* node = this->_first; node; prev = node, node = node->next) {
               if (node->data.first == k) {
                  node->value = std::forward<M>(v);
                  return { iterator(node), false };
               }
               if (node->data.first > k)
                  break;
            }
            auto* next = this->_insert_after(prev);
            next->data = value_type(std::move(k), std::forward<M>(v));
            return { iterator(next), true };
         }
   };
}