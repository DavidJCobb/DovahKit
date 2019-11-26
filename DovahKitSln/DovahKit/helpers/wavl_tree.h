#pragma once
#include <algorithm> // std::max
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>

namespace cobb {
   template<typename key_type, typename value_type>
   struct wavl_node {
      std::pair<key_type, value_type> data;
      //
      wavl_node* left   = nullptr;
      wavl_node* right  = nullptr;
      wavl_node* parent = nullptr;
      uint8_t    rank   = 0;
      //
      wavl_node() {}
      wavl_node(key_type k, value_type v, wavl_node* p) : data(k, v), parent(p) {}
      //
      bool operator==(const wavl_node& other) const noexcept {
         return this->data == other.data;
      }
      inline key_type   key()   const noexcept { return this->data.first; }
      inline value_type value() const noexcept { return this->data.second; }
      inline value_type* value_pointer() noexcept { return &this->data.second; }
      //
      wavl_node* prev() const noexcept {
         if (this->left) {
            //
            // Find the rightmost descendant of the left child, i.e.
            //
            //         <T=5>
            //        /     \
            //       3       7
            //      / \     / \
            //     1   4   6   10
            //
            wavl_node* target = this->left;
            while (target->right)
               target = target->right;
            return target;
         }
         //
         // Traverse the parents and stop at the parent of the first right-child, i.e.
         //
         //           5
         //          / \
         //         /   7
         //        /   / \
         //       3   6   7
         //      / \       \
         //     1  <T=4>    10
         //
         auto  target = this;
         wavl_node* parent = this->parent;
         while (parent && target == parent->left) {
            target = parent;
            parent = parent->parent;
         }
         return parent;
      }
      wavl_node* next() const noexcept {
         if (this->right) {
            //
            // Find the leftmost descendant of the right child, i.e.
            //
            //         <T=5>
            //        /     \
            //       3       7
            //      / \     / \
            //     1   4   6   10
            //
            wavl_node* target = this->right;
            while (target->left)
               target = target->left;
            return target;
         }
         //
         // Traverse the parents and stop at the parent of the first left-child, i.e.
         //
         //           5
         //          / \
         //         3   \
         //        / \   \
         //       /   4   7
         //      /       / \
         //     1     <T=6> 10
         //
         auto  target = this;
         wavl_node* parent = this->parent;
         while (parent&& target == parent->right) {
            target = parent;
            parent = parent->parent;
         }
         return parent;
      }
      wavl_node* sibling() const noexcept {
         auto p = this->parent;
         if (!p)
            return nullptr;
         if (p->left == this)
            return p->right;
         return p->left;
      }
      int delta() const noexcept {
         if (this->parent)
            return this->parent->rank - this->rank;
         return 0; // TODO: is this right?
      }
      inline bool is_leaf() const noexcept {
         return !this->left && !this->right;
      }
      inline bool is_unary() const noexcept {
         return !(this->left && this->right) && (this->left || this->right);
      }
      bool is_two_two() const noexcept {
         if (!this->left || !this->right)
            return false;
         if (this->rank - this->left->rank != 2)
            return false;
         if (this->rank - this->right->rank != 2)
            return false;
         return true;
      }
      int child_delta(bool left) const noexcept {
         auto c = left ? this->left : this->right;
         if (c)
            return this->rank - c->rank;
         return this->rank - (-1);
      }
      //
      int32_t height() const {
         int32_t l = this->left  ? this->left->height()  : -1;
         int32_t r = this->right ? this->right->height() : -1;
         return std::max(l, r) + 1;
      }
      uint32_t size() const {
         uint32_t l = this->left  ? this->left->size()  : 0;
         uint32_t r = this->right ? this->right->size() : 0;
         return l + r + 1;
      }
      //
      void _debugbreak_if_invalid() {
         #if _DEBUG
            if (this == this->left || this == this->right || this == this->parent)               __debugbreak();
         #endif
      }
   };

   template<typename key_type, typename value_type, typename allocator_type = std::allocator<wavl_node<key_type, value_type>>>
   class wavl_tree {
      public:
         typedef key_type   key_type;
         typedef value_type value_type;
         using node = wavl_node<key_type, value_type>;
         //
         static_assert(std::is_same_v<node, typename allocator_type::value_type>, "cobb::wavl_tree<key_type, value_type> was instantiated with an allocator templated on the wrong type. The allocator must be templated on cobb::wavl_node<key_type, value_type>; it must match exactly, including const specifiers and the like.");
         //
      public:
         //
         // This class is a Weak AVL -- that is, a particular kind of self-
         // balancing binary tree.
         //
         // Unlike MSVC/GCC std::map, this class actually uses the allocator you 
         // supply to it (if any), instead of silently converting that allocator 
         // to a different type via std::allocator::rebind. This means that if 
         // you template your allocator on the correct wavl_node type, you can 
         // use an allocator to wrap a custom heap singleton -- something that 
         // would be a tremendous pain with std::map.
         //
         // BASED ON:
         //    <http://sidsen.azurewebsites.net//papers/rb-trees-talg.pdf>
         //
         // GLOSSARY:
         //
         // external node
         // leaf node
         //    A node with no children.
         //
         // internal node
         //    A node with children.
         //
         // i,j
         //    Notation used for a node whose left-child has a rank that is 
         //    i less, and whose right-child has a rank that is j less.
         //
      protected:
         node*    root = nullptr;
         uint32_t _size = 0;
         //
      protected:
         enum class comparison {
            less,
            greater,
            equal
         };
         inline static comparison _compare(key_type a, key_type b) noexcept {
            if (a < b)
               return comparison::less;
            if (a > b)
               return comparison::greater;
            return comparison::equal;
         }
         //
         bool _for_each(node* n, std::function<bool(key_type, value_type)> functor) const noexcept {
            //
            // Abort early if return value is true.
            //
            if (n->left)
               if (_for_each(n->left, functor))
                  return true;
            if (functor(n->key(), n->value()))
               return true;
            if (n->right)
               if (_for_each(n->right, functor))
                  return true;
            return false;
         }
         node* _first() const noexcept {
            auto n = this->root;
            if (n)
               while (n->left)
                  n = n->left;
            return n;
         }
         node* _last() const noexcept {
            auto n = this->root;
            if (n)
               while (n->right)
                  n = n->right;
            return n;
         }
         //
         void _rotate_left(node* x) noexcept {
            //
            //   z                x
            //  / \              / \
            // C   x     ->     z   A
            //    / \          / \
            //   y   A        C   y
            //
            auto z = x->parent;
            auto y = x->left;
            auto p = z->parent;
            x->left   = z;
            z->parent = x;
            z->right  = y;
            if (y)
               y->parent = z;
            if (z == this->root) {
               this->root = x;
               x->parent = nullptr;
            } else {
               if (p->right == z)
                  p->right = x;
               else
                  p->left = x;
               x->parent = p;
               #if _DEBUG
                  p->_debugbreak_if_invalid();
               #endif
            }
            #if _DEBUG
               x->_debugbreak_if_invalid();
               if (y) y->_debugbreak_if_invalid();
            #endif
         }
         void _rotate_right(node* x) noexcept {
            //
            //     z            x
            //    / \          / \
            //   x   C   ->   A   z
            //  / \              / \
            // A   y            y   C
            //
            auto z = x->parent;
            auto y = x->right;
            auto p = z->parent;
            x->right  = z;
            z->parent = x;
            z->left   = y;
            if (y)
               y->parent = z;
            if (z == this->root) {
               this->root = x;
               x->parent = nullptr;
            } else {
               if (p->left == z)
                  p->left = x;
               else
                  p->right = x;
               x->parent = p;
               #if _DEBUG
                  p->_debugbreak_if_invalid();
               #endif
            }
            #if _DEBUG
               x->_debugbreak_if_invalid();
               if (y) y->_debugbreak_if_invalid();
            #endif
         }
         void _double_rotate_left(node* x) noexcept {
            //
            //     z                  y
            //    / \               /   \
            //   D   x            z       x
            //      / \    ->    / \     / \
            //     y   A        D   C   B   A
            //    / \
            //   C   B
            //
            auto y = x->left;
            auto z = x->parent;
            auto r = y->right;
            auto l = y->left;
            x->left  = r;
            z->right = l;
            if (r)
               r->parent = x;
            if (l)
               l->parent = z;
            y->right = x;
            y->left = z;
            auto p = z->parent;
            x->parent = z->parent = y;
            if (z == this->root) {
               this->root = y;
               y->parent = nullptr;
            } else {
               y->parent = p;
               if (p->right == z)
                  p->right = y;
               else
                  p->left = y;
            }
         }
         void _double_rotate_right(node* x) noexcept {
            //
            //     z                y
            //    / \             /  \
            //   x   D          x      z
            //  / \      ->    / \    / \
            // A   y          A   B  C   D
            //    / \
            //   B   C
            //
            auto y = x->right;
            auto z = x->parent;
            auto l = y->left;
            auto r = y->right;
            x->right  = l;
            z->left   = r;
            if (l)
               l->parent = x;
            if (r)
               r->parent = z;
            y->left   = x;
            y->right  = z;
            auto p = z->parent;
            x->parent = z->parent = y;
            if (z == this->root) {
               this->root = y;
               y->parent = nullptr;
            } else {
               y->parent = p;
               if (p->left == z)
                  p->left = y;
               else
                  p->right = y;
            }
         }
         void _fix_insert(node* x) noexcept { // x should be the newly-inserted node
            //
            // Per pages six and seven of <http://sidsen.azurewebsites.net//papers/rb-trees-talg.pdf>.
            //
            while (node* p = x->parent) {
               x = p;
               x->rank++;
               //
               auto z = x->parent;
               if (!z)
                  return;
               if (x->rank == z->rank) { // rank rule violated: a node cannot have the same rank as its parent
                  bool left = z->left == x;
                  auto y = left ? x->right : x->left;
                  if (!y || y->rank == x->rank - 2) {
                     if (left)
                        this->_rotate_right(x);
                     else
                        this->_rotate_left(x);
                     z->rank--;
                  } else if (y->rank == x->rank - 1) {
                     if (left)
                        this->_double_rotate_right(x);
                     else
                        this->_double_rotate_left(x);
                     y->rank++;
                     x->rank--;
                     z->rank--;
                  }
                  return;
               }
            }
         }
         void _fix_delete(node* parent, node* sibling, node* target) noexcept {
            auto x = target;
            auto y = sibling;
            auto z = parent;
            int  deltaX = z->rank - x->rank;
            int  deltaY = z->rank - (y ? y->rank : -1);
            while (deltaX == 3 && (deltaY == 2 || (y && y->is_two_two()))) {
               if (deltaY == 2)
                  z->rank--;
               else {
                  if (y)
                     y->rank--;
                  z->rank--;
               }
               x = z;
               y = x->sibling();
               z = x->parent;
            }
            if (z->rank - x->rank == 3) { // z is 1,3 or 3,1, violating the rank rules
               if (x == z->left) {
                  auto v = y->left;
                  auto w = y->right;
                  if (y->rank - w->rank == 2) {
                     this->_rotate_left(y);
                     y->rank++;
                     z->rank--;
                     if (z->is_leaf())
                        z->rank--;
                     return;
                  } else if (y->rank - w->rank == 1) {
                     this->_double_rotate_left(y);
                     v->rank += 2;
                     y->rank -= 1;
                     z->rank -= 2;
                     return;
                  }
               } else {
                  auto v = y->right;
                  auto w = y->left;
                  if (y->rank - w->rank == 2) {
                     this->_rotate_right(y);
                     y->rank++;
                     z->rank--;
                     if (z->is_leaf())
                        z->rank--;
                     return;
                  } else if (y->rank - w->rank == 1) {
                     this->_double_rotate_right(y);
                     v->rank += 2;
                     y->rank -= 1;
                     z->rank -= 2;
                     return;
                  }
               }
            }
         }
         //
         static node* _make_node(key_type k, value_type v, node* parent) {
            auto uninitialized = allocator_type().allocate(1);
            return new (uninitialized) node(k, v, parent);
         }
         static void _free_node(node* target) {
            target->~node();
            allocator_type().deallocate(target, 1);
         }
         static void _destroy_node_descendants(node* target) {
            if (target->left) {
               _destroy_node_descendants(target->left);
               _free_node(target->left);
               target->left = nullptr;
            }
            if (target->right) {
               _destroy_node_descendants(target->right);
               _free_node(target->right);
               target->right = nullptr;
            }
         }
         //
         node* _get(key_type k) const noexcept {
            auto p = this->root;
            while (p) {
               auto ok = p->key();
               if (k < ok)
                  p = p->left;
               else if (k > ok)
                  p = p->right;
               else
                  return p;
            }
            return nullptr;
         }
         node* _set(key_type k, value_type v) noexcept {
            //
            // The reason the "set" code is in a protected helper function is because 
            // we don't want the public "set" function to give access to the created 
            // or altered node, but we need that access to optimize operator[].
            //
            node* t = this->root;
            if (t == nullptr) {
               this->root = _make_node(k, v, nullptr);
               this->_size = 1;
               return this->root;
            }
            comparison last;
            node* parent = nullptr;
            do {
               parent = t;
               last = _compare(k, t->key());
               if (last == comparison::less)
                  t = t->left;
               else if (last == comparison::greater)
                  t = t->right;
               else {
                  t->data.second = v;
                  return t;
               }
            } while (t);
            auto e = _make_node(k, v, parent);
            if (last == comparison::less)
               parent->left = e;
            else
               parent->right = e;
            if (parent->rank == 0) // a leaf became a branch
               _fix_insert(e);
            this->_size++;
            return e;
         }
         //
      public:
         ~wavl_tree() noexcept {
            this->clear();
         }
         void for_each(std::function<bool(key_type, value_type)> functor) const {
            if (this->root)
               this->_for_each(this->root, functor);
         }
         bool contains(key_type k) const noexcept {
            return this->_get(k) != nullptr;
         }
         value_type* get(key_type k) const noexcept {
            auto node = this->_get(k);
            if (node)
               return node->value_pointer();
            return nullptr;
         }
         void remove(key_type k) noexcept {
            node* t = this->root;
            if (!t)
               return;
            do {
               auto ok = t->key();
               if (k < ok)
                  t = t->left;
               else if (k > ok)
                  t = t->right;
               else
                  break;
            } while (t);
            if (!t)
               return;
            this->_size--;
            //
            // t == the node to remove
            //
            auto l = t->left;
            auto r = t->right;
            if (l && r) {
               auto s = t->next();
               //
               // Swap the deletion target with its successor, and then delete the 
               // successor instead.
               //
               t->data = s->data;
               //
               t = s;
               l = t->left;
               r = t->right;
            }
            //
            // Now, it is guaranteed that we are deleting a leaf or unary node.
            //
            auto p = t->parent;
            if (!p) {
               //
               // The root node is unary or a leaf, and we are removing it.
               //
               this->root = l ? l : r;
               if (this->root)
                  this->root->parent = nullptr;
               _free_node(t);
               return;
            }
            auto diff = p->rank - t->rank;
            bool left = p && p->left == t;
            if (!l && !r) { // target is a leaf
               //
               //      7              7
               //     / \              \
               // <T=5>  10      =>     10
               //       /  \           /  \
               //      8    12        8    12
               //
               (left ? p->left : p->right) = nullptr;
            } else { // target is unary
               auto e = l ? l : r;
               (left ? p->left : p->right) = e;
               e->parent = p;
            }
            this->_fix_delete(p, left ? p->right : p->left, t);
            _free_node(t);
         }
         void set(key_type k, value_type v) noexcept {
            this->_set(k, v);
         }
         //
         size_t size() const noexcept { return this->_size; }
         bool empty() const noexcept { return !this->_size; }
         void clear() noexcept {
            if (this->root) {
               _destroy_node_descendants(this->root);
               _free_node(this->root);
               this->root = nullptr;
               this->_size = 0;
            }
         }
         //
         /*//
         void _debug_dump(node* n) const {
            printf("Node <%d!%d=%d>", n->key(), n->rank, n->value());
            if (n->left || n->right) {
               printf(": ");
               if (n->left) {
                  printf("LEFT is <%d!%d=%d>", n->left->key(), n->left->rank, n->left->value());
                  if (n->right)
                     printf("; ");
               }
               if (n->right)
                  printf("RIGHT is <%d!%d=%d>", n->right->key(), n->right->rank, n->right->value());
            }
            printf("\n");
            if (n->left)
               _debug_dump(n->left);
            if (n->right)
               _debug_dump(n->right);
         }
         void _debug_dump() const {
            printf("Printing nodes...\n");
            if (this->root)
               this->_debug_dump(this->root);
            printf("Done.\n");
         }
         //*/
         //
         // BELOW: Rough/partial parity with STL containers.
         //
         struct iterator {
            friend wavl_tree;
            protected:
               iterator(node* n) : target(n) {}
               //
               node* target = nullptr;
               //
            public:
               iterator& operator--() noexcept { // prefix only i.e. --it but not it--
                  if (this->target)
                     this->target = this->target->prev();
                  return *this;
               }
               iterator& operator++() noexcept { // prefix only i.e. ++it but not it++
                  if (this->target)
                     this->target = this->target->next();
                  return *this;
               }
               std::pair<key_type, value_type>* operator->() noexcept {
                  if (!this->target)
                     return nullptr;
                  return &this->target->data;
               }
               //
               bool operator==(const iterator& other) const noexcept { return this->target == other.target; }
               bool operator!=(const iterator& other) const noexcept { return !(*this == other); }
         };
         struct reverse_iterator : public iterator {
            friend wavl_tree;
            protected:
               reverse_iterator(node* n) : iterator(n) {}
            public:
               reverse_iterator& operator--() noexcept {
                  iterator::operator++();
                  return *this;
               }
               reverse_iterator& operator++() noexcept {
                  iterator::operator--();
                  return *this;
               }
         };
         //
         iterator begin() noexcept { return iterator(this->_first()); }
         iterator end()   noexcept { return iterator(nullptr); }
         const iterator begin() const noexcept { return iterator(this->_first()); }
         const iterator end()   const noexcept { return iterator(nullptr); }
         reverse_iterator rbegin() noexcept { return reverse_iterator(this->_last()); }
         reverse_iterator rend()   noexcept { return reverse_iterator(nullptr); }
         const reverse_iterator rbegin() const noexcept { return reverse_iterator(this->_last()); }
         const reverse_iterator rend()   const noexcept { return reverse_iterator(nullptr); }
         //
         value_type& at(key_type k) {
            auto e = this->get(k);
            if (e)
               return *e;
            throw std::out_of_range("element not present");
         }
         value_type& operator[](key_type k) noexcept {
            auto e = this->get(k);
            if (e)
               return *e;
            auto node = this->_set(k, value_type());
            return node->data.second;
         }
         //
         void swap(wavl_tree& other) noexcept {
            auto a = this->root;
            auto b = other.root;
            this->root = b;
            other.root = a;
            auto c = this->_size;
            auto d = other._size;
            this->_size = c;
            other._size = d;
         }
         //
         // Faster than using std::find since that has to traverse the entire container (i.e. from begin to end).
         iterator find(key_type k) noexcept { return iterator(this->_get(k)); }
         const iterator find(key_type k) const noexcept { return iterator(this->_get(k)); }
   };

   namespace unit_tests {
      void wavl_tree();
   }
}