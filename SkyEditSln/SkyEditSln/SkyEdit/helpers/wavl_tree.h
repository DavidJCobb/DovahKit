#pragma once
#include <cstdint>
#include <functional>

namespace cobb {
   template<typename key_type, typename value_type> class wavl_map {
      public:
         typedef key_type   key_type;
         typedef value_type value_type;
         //
         // GLOSSARY:
         //
         // external node
         // leaf node
         //    A node with no children.
         //
         //
         //
      protected:
         struct node {
            key_type   key;
            value_type value;
            //
            node* left   = nullptr;
            node* right  = nullptr;
            node* parent = nullptr;
            int8_t rank = 0;
            //
            node() {
               this->rank = -1;
            }
            node(key_type k, value_type v, node* p) : key(k), value(v), parent(p) {}
            //
            bool operator==(const node& other) const noexcept {
               return this->key == other.key && this->value == other.value;
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
         bool _for_each(node* n, std::function<bool(key_type, value_type)> functor) {
            //
            // Abort early if return value is true.
            //
            if (!n)
               return false;
            if (_for_each(n->left, functor))
               return true;
            if (functor(n->key, n->value))
               return true;
            if (_for_each(n->right, functor))
               return true;
            return false;
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
         void _fix_insert(node* x) {

            //
            // TODO: I loosely based this on pseudocode elsewhere but should rewrite it to 
            // match bottom-up rebalancing as described on pages 6 and 7 of:
            //  <http://sidsen.azurewebsites.net//papers/rb-trees-talg.pdf> 
            //

            x->rank = 0;
            x = x->parent;
            for (auto z = x->parent; z; x = z, z = z->parent) {
               x->rank++;
               if (z->rank == x->rank + 1)
                  break;
               else if (z->rank == x->rank) {
                  //
                  // The source here <http://sidsen.azurewebsites.net//papers/rb-trees-talg.pdf> 
                  // describes this as a check for whether p(x) == 0,2. That is: if we describe 
                  // each node in terms of the difference between its rank and its children's 
                  // ranks, then if after incrementing x's rank, the differences are 0 and 2 
                  // respectively (i.e. x has the same rank as its parent, and x's sibling is 
                  // separated from its parent's rank by 2), then we need to rotate, because 
                  // we cannot increase the rank of x's parent (in the next loop iteration) 
                  // without breaking the relationship between it and x's sibling (the rank 
                  // difference cannot exceed 2 in a WAVL tree).
                  //
                  if (z->left == x) { // x is the left-side child
                     auto y = x->right;
                     if (!y || y->rank == x->rank - 2) { // cannot increase the parent's rank. must rotate.
                        this->_rotate_right(x);
                        z->rank--;
                        return;
                     } else if (y->rank == x->rank - 1) {
                        this->_double_rotate_right(x);
                        y->rank++;
                        x->rank--;
                        z->rank--;
                        return;
                     }
                  } else { // x is the right-side child
                     auto y = x->left;
                     if (!y || y->rank == z->rank - 2) { // cannot increase the parent's rank. must rotate.
                        this->_rotate_left(x);
                        z->rank--;
                        return;
                     } else if (y->rank == x->rank - 1) {
                        this->_double_rotate_left(x);
                        y->rank++;
                        x->rank--;
                        z->rank--;
                        return;
                     }
                  }
               }
            }
         }
         //
      public:
         void for_each(std::function<bool(key_type, value_type)> functor) {
            this->_for_each(this->root, functor);
         }
         value_type* get(key_type k) {
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
               last = _compare(k, t->key);
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
            if (parent->rank == 0) {
               parent->rank++;
            }
            _fix_insert(e);
            this->size++;
         }
   };
}