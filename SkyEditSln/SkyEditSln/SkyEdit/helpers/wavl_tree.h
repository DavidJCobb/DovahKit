#pragma once
#include <algorithm> // std::max
#include <cstdint>
#include <functional>
#include <type_traits>

namespace cobb {
   template<typename key_type, typename value_type>
   class wavl_tree {
      public:
         typedef key_type   key_type;
         typedef value_type value_type;
         //
      protected:
         static constexpr bool _value_type_is_pointer = std::is_pointer<value_type>::value;
      public:
         // This is value_type if value_type is already a pointer, or value_type* otherwise:
         using value_pointer_type = std::conditional_t<_value_type_is_pointer, value_type, std::add_pointer_t<value_type>>;
         // This is value_type& if value_type isn't a pointer, or else equivalent to typedef(f) given { value_type foo; auto& f = *foo; }:
         using value_reference_type = std::conditional_t<_value_type_is_pointer, std::add_lvalue_reference_t<std::remove_pointer_t<value_type>>, value_type&>;
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
         struct node {
            key_type   key;
            value_type value;
            //
            node* left   = nullptr;
            node* right  = nullptr;
            node* parent = nullptr;
            uint8_t rank = 0;
            //
            node() {}
            node(key_type k, value_type v, node* p) : key(k), value(v), parent(p) {}
            //
            bool operator==(const node& other) const noexcept {
               return this->key == other.key && this->value == other.value;
            }
            //
            node* prev() const {
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
                  node* target = this->left;
                  while (target->right)
                     target = target->right;
                  return target;
               }
               //
               // Traverse the parents and stop at the first left-child, i.e.
               //
               //     7
               //    / \
               //   3   10
               //    \
               //   <T=5>
               //
               auto  target = this;
               node* parent = target->parent;
               while (parent && target == parent->right) {
                  target = parent;
                  parent = parent->parent;
               }
               return parent;
            }
            node* next() const {
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
                  node* target = this->right;
                  while (target->left)
                     target = target->left;
                  return target;
               }
               //
               // Traverse the parents and stop at the first right-child, i.e.
               //
               //     4
               //    / \
               //   1   7
               //      / \
               //   <T=5> 10
               //
               auto  target = this;
               node* parent = target->parent;
               while (parent && target == parent->left) {
                  target = parent;
                  parent = parent->parent;
               }
               return parent;
            }
            node* sibling() const {
               auto p = this->parent;
               if (!p)
                  return nullptr;
               if (p->left == this)
                  return p->right;
               return p->left;
            }
            int   delta() const {
               if (this->parent)
                  return this->parent->rank - this->rank;
               return 0; // TODO: is this right?
            }
            inline bool is_leaf() const {
               return !this->left && !this->right;
            }
            inline bool is_unary() const {
               return !(this->left && this->right) && (this->left || this->right);
            }
            bool is_two_two() const {
               if (!this->left || !this->right)
                  return false;
               if (this->rank - this->left->rank != 2)
                  return false;
               if (this->rank - this->right->rank != 2)
                  return false;
               return true;
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
         };
         //
         node*    root = nullptr;
         uint32_t size = 0;
         //
      protected:
         enum class comparison {
            less,
            greater,
            equal
         };
         inline static comparison _compare(key_type a, key_type b) {
            if (a < b)
               return comparison::less;
            if (a > b)
               return comparison::greater;
            return comparison::equal;
         }
         //
         bool _for_each(node* n, std::function<bool(key_type, value_type)> functor) const {
            //
            // Abort early if return value is true.
            //
            if (_for_each(n->left, functor))
               return true;
            if (functor(n->key, n->value))
               return true;
            if (_for_each(n->right, functor))
               return true;
            return false;
         }
         node* _first() const {
            auto n = this->root;
            if (!n)
               return nullptr;
            node* p;
            while (p = n->prev())
               n = p;
            return n;
         }
         node* _last() const {
            auto n = this->root;
            if (!n)
               return nullptr;
            node* p;
            while (p = n->next())
               n = p;
            return n;
         }
         //
         void _rotate_left(node* x) {
            //
            //   z                x
            //  / \              / \
            // C   x     ->     z   A
            //    / \          / \
            //   y   A        C   y
            //
            auto z = x->parent;
            auto y = x->left;
            x->left   = z;
            z->parent = x;
            z->right  = y;
            if (y)
               y->parent = z;
            if (z == this->root) {
               this->root = x;
               x->parent = nullptr;
            } else {
               auto p = z->parent;
               if (p->right == z)
                  p->right = x;
               else
                  p->left = x;
               x->parent = p;
            }
         }
         void _rotate_right(node* x) {
            //
            //     z            x
            //    / \          / \
            //   x   C   ->   A   z
            //  / \              / \
            // A   y            y   C
            //
            auto z = x->parent;
            auto y = x->right;
            x->right  = z;
            z->parent = x;
            z->left   = y;
            if (y)
               y->parent = z;
            if (z == this->root) {
               this->root = x;
               x->parent = nullptr;
            } else {
               auto p = z->parent;
               if (p->left == z)
                  p->left = x;
               else
                  p->right = x;
               x->parent = p;
            }
         }
         void _double_rotate_left(node* x) {
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
         void _double_rotate_right(node* x) {
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
         void _fix_insert(node* x) { // x should be the newly-inserted node
            //
            // Per pages six and seven of <http://sidsen.azurewebsites.net//papers/rb-trees-talg.pdf>.
            //
            while (x = x->parent) {
               x->rank++;
               //
               auto z = x->parent;
               if (x->rank == z->rank) { // rank rule violated: a node cannot have the same rank as its parent
                  bool left = z->left == x;
                  auto y    = left ? x->right : x->left;
                  if (!y || y->rank == x->rank - 2) {
                     if (left)
                        this->_rotate_right(x);
                     else
                        this->_rotate_left(x);
                     z->rank--;
                     return;
                  } else if (y->rank == x->rank - 1) {
                     if (left)
                        this->_double_rotate_right(x);
                     else
                        this->_double_rotate_left(x);
                     y->rank++;
                     x->rank--;
                     z->rank--;
                     return;
                  }
               }
            }
         }
         void _fix_delete(node* parent, node* sibling, node* target) {
            auto x = target;
            auto y = sibling;
            auto z = parent;
            int  deltaX = z->rank - x->rank;
            int  deltaY = z->rank - y->rank;
            while (deltaX == 3 && (deltaY == 2 || y->is_two_two())) {
               if (deltaY == 2)
                  z->rank--;
               else {
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
      public:
         void for_each(std::function<bool(key_type, value_type)> functor) const {
            if (this->root)
               this->_for_each(this->root, functor);
         }
         value_type* get(key_type k) const {
            auto p = this->root;
            while (p) {
               if (k < p->key)
                  p = p->left;
               else if (k > p->key)
                  p = p->right;
               else
                  return &p->value;
            }
            return nullptr;
         }
         void remove(key_type k) {
            node* t = this->root;
            if (!t)
               return;
            do {
               if (k < t->key)
                  t = t->left;
               else if (k > t->key)
                  t = t->right;
               else
                  break;
            } while (t);
            if (!t)
               return;
            this->size--;
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
               t->key   = s->key;
               t->value = s->value;
               //
               t = s;
               l = t->left;
               r = t->right;
            }
            //
            // Now, it is guaranteed that we are deleting a leaf or unary node.
            //
            auto p    = t->parent;
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
               if (p)
                  (left ? p->left : p->right) = nullptr;
               else
                  this->root = nullptr;
            } else { // target is unary
               auto e = l ? l : r;
               if (p)
                  (left ? p->left : p->right) = e;
               else
                  this->root = e;
               e->parent = p;
            }
            this->_fix_delete(p, left ? p->right : p->left, t);
            delete t;
         }
         void set(key_type k, value_type v) {
            node* t = this->root;
            if (t == nullptr) {
               this->root = new node(k, v, nullptr);
               this->size = 1;
               return;
            }
            comparison last;
            node* parent = nullptr;
            do {
               parent = t;
               last   = _compare(k, t->key);
               if (last == comparison::less)
                  t = t->left;
               else if (last == comparison::greater)
                  t = t->right;
               else {
                  t->value = v;
                  return;
               }
            } while (t);
            auto e = new node(k, v, parent);
            if (last == comparison::less)
               parent->left = e;
            else
               parent->right = e;
            if (parent->rank == 0) // a leaf became a branch
               _fix_insert(e);
            this->size++;
         }

         struct iterator {
            iterator(node* n) : target(n) {}
            //
            node* target = nullptr;
            //
            iterator& operator--() {
               if (this->target)
                  this->target = this->target->prev();
               return *this;
            }
            iterator& operator++() {
               if (this->target)
                  this->target = this->target->next();
               return *this;
            }
            template<typename std::enable_if_t<_value_type_is_pointer>* = nullptr> value_pointer_type operator->() const noexcept {
               return this->target->value;
            }
            template<typename std::enable_if_t<!_value_type_is_pointer>* = nullptr> value_pointer_type operator->() const noexcept {
               return &this->target->value;
            }
            template<typename std::enable_if_t<_value_type_is_pointer>* = nullptr> value_reference_type operator*() const noexcept {
               return *this->target->value;
            }
            template<typename std::enable_if_t<!_value_type_is_pointer>* = nullptr> value_reference_type operator*() const noexcept {
               return this->target->value;
            }
         };
         //
         iterator begin() {
            return iterator(this->_first());
         }
         iterator end() {
            return iterator(nullptr);
         }
   };

   namespace unit_tests {
      void wavl_tree();
   }
}