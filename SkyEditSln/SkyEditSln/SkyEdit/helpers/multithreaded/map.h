#pragma once
#include <atomic>
#include <cstdint>
#include <thread>

// an attempt at making a lock-free red-black tree.
// well, technically, it's not that the tree is lock-free, but rather that 
// the tree itself doesn't lock, preferring instead to have subtrees lock 
// independently of one another.

// based on pseudocode at <https://www.cs.umanitoba.ca/~hacamero/Research/RBTreesKim.pdf>
// but i'm struggling with it a bit rn. the pseudocode doesn't make it clear when 
// structs are created or whatnot, doesn't contain struct definitions, and in some 
// places it doesn't even demonstrate what to do (preferring to give vague notes 
// that are missing extremely critical information, though i'm sure i can find 
// that information buried somewhere in a dozen paragraphs of prose that often 
// aren't even located within a few pages of the code they describe).
//
// like, it's not even remotely clear from the pseudocode where objects or values 
// come from.
//
// i might come back to this later

namespace cobb {
   class multithreaded_map {
      public:
         multithreaded_map() : dummy(dummy, dummy, dummy), root(dummy, dummy, dummy) {
            this->dummy.is_dummy = true;
         }
      protected:
         enum class node_color : uint8_t {
            black, // root node, leaf nodes, and any child of a red node
            red,
         };
         //
         struct usage {
            volatile char value;
            //
            char compare_exchange(bool expected, bool desired) {
               return _InterlockedCompareExchange8(&this->value, desired, expected);
            }
            inline bool try_lock() {
               return this->compare_exchange(false, true);
            }
            inline void unlock() {
               this->value = false;
            }
            inline operator bool() const noexcept { return this->value != 0; }
            //
            usage() noexcept : value(0) {}
            usage(bool a) noexcept : value(a) {}
            usage& operator=(bool a) noexcept {
               this->value = a;
               return *this;
            }
         };
         //
         struct element {
            uint32_t key;
            void*    value = nullptr;
         };
         struct node {
            element    data;
            node_color color = node_color::black;
            node&      parent;
            node&      left;  // child
            node&      right; // child
            bool       is_dummy = false;
            usage      in_use   = false;
            std::thread::id   threadID;
            //
            node(node& p, node& l, node& r) : parent(p), left(l), right(r) {}
            //
            node& operator=(const node& other) {
               this->data   = other.data;
               this->color  = other.color;
               this->parent = other.parent;
               this->left   = other.left;
               this->right  = other.right;
            }
            node& sibling() const {
               if (this->parent.left == this)
                  return this->parent.right;
               return this->parent.left;
            }
            inline operator bool() const { return !this->is_dummy; }
            inline bool operator<(const node& other) { return this->data.key < other.data.key; }
         };
         node dummy; // used to ensure every node has two children // nil[T], a black node // leaves are "nil" nodes
         node root;
         //
         node* make_node() {
            auto n = new node(this->dummy, this->dummy, this->dummy);
            return n;
         }
         //
         struct _move_up_struct {
            std::vector<node*> nodes;
            std::thread::id owner;
            std::thread::id defer_to[2];
         };
         bool _apply_move_up_rule(node& x, node& w, _move_up_struct& out) {
            bool w_has  = w.threadID != std::thread::id();
            bool wl_has = w.left.threadID != std::thread::id();
            bool wr_has = w.right.threadID != std::thread::id();
            bool wr_consistent = w.threadID == w.right.threadID;
            //
            bool a = (w.threadID == w.parent.threadID && wr_consistent && w_has && wl_has);
            bool b = (wr_consistent && w_has && wl_has);
            bool c = (!w_has && wl_has && wr_has);
            //
            if (a || b || c) {
               //
               // TODO: modify (out) to list the nodes we hold flags on
               //
               out.owner = w.right.threadID;
               return true;
            }
            return false;
         }
         bool _spacing_rule_met(node& t, node& z, std::thread::id defer_to) {
            if (t != z)
               if (t.threadID != std::thread::id())
                  return false;
            auto& tp = t.parent;
            if (tp != z) {
               if (this->_is_in(tp, move_data) && !tp.in_use.try_lock())
                  return false;
               if (tp != t.parent) {
                  tp.in_use = false;
                  return false;
               }
               if (tp.threadID != std::thread::id()) {
                  tp.in_use = false;
                  return false;
               }
            }
            auto& ts = (t == tp.left) ? tp.right : tp.left;
            if (!this->_is_in(ts, move_data) && !ts.in_use.try_lock()) {
               if (tp != z)
                  this->_release_flags(move_data, false, tp);
               return false;
            }
            if (ts.threadID != std::thread::id() && ts.threadID != defer_to) {
               this->_release_flags(move_data, false, ts);
               if (tp != z)
                  this->_release_flags(move_data, false, tp);
               return false;
            }
            if (tp != z)
               this->_release_flags(move_data, false, tp);
            this->_release_flags(move_data, false, ts);
            return true;
         }
         void _release_flags(_move_up_struct& move_data, bool success, node* n1 = nullptr, node* n2 = nullptr, node* n3 = nullptr, node* n4 = nullptr) {
            if (success) {
               if (!this->_is_in(n1, move_data))
                  n1->in_use = false;
               else {
                  if (this->_is_goal_node(n1, move_data)) {
                     //
                     // TODO: release unneeded flags in move_data and discard it.
                     //
                     return;
                  }
               }
               //
               // TODO: Repeat for n2, n3, n4
               //
            } else {
               if (!this->_is_in(n1, move_data))
                  n1->in_use = false;
               //
               // TODO: Repeat for n2, n3, n4
               //
            }
         }
         bool _get_flags_for_markers(node& start, _move_up_struct& move_data, node& pos1, node& pos2, node& pos3, node& pos4) {
            pos1 = start.parent;
            if (!this->_is_in(pos1, move_data) && !pos1.in_use.try_lock())
               return false;
            if (pos1 != start.parent) {
               this->_release_flags(move_data, false, pos1);
               return false;
            }
            //
            pos2 = pos1.parent;
            if (!this->_is_in(pos2, move_data) && !pos2.in_use.try_lock())
               return false;
            if (pos2 != pos1.parent) {
               this->_release_flags(move_data, false, pos2, pos1);
               return false;
            }
            //
            pos3 = pos2.parent;
            if (!this->_is_in(pos3, move_data) && !pos3.in_use.try_lock())
               return false;
            if (pos3 != pos2.parent) {
               this->_release_flags(move_data, false, pos3, pos2, pos1);
               return false;
            }
            //
            pos4 = pos3.parent;
            if (!this->_is_in(pos4, move_data) && !pos4.in_use.try_lock())
               return false;
            if (pos4 != pos3.parent) {
               this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
               return false;
            }
            //
            return true;
         }
         bool _get_flags_and_markers_above(node& start, int additional) {
            _move_up_struct move_data;
            node& pos1 = this->dummy;
            node& pos2 = this->dummy;
            node& pos3 = this->dummy;
            node& pos4 = this->dummy;
            if (!this->_get_flags_for_markers(start, move_data, pos1, pos2, pos3, pos4))
               return false;
            auto& first_new = pos4.parent;
            if (!this->_is_in(first_new, move_data) && !first_new.in_use.try_lock()) {
               this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
               return false;
            }
            if (first_new != pos4.parent && !this->_spacing_rule_met(first_new, start, PIDtoIgnore, move_data)) {
               this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
               return false;
            }
            auto& second_new = this->dummy;
            if (additional == 2) {
               second_new = first_new.parent;
               if (!this->_is_in(second_new, move_data) && !second_new.in_use.try_lock()) {
                  this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
                  return false;
               }
               if (second_new != first_new.parent && !this->_spacing_rule_met(second_new, start, PIDtoIgnore, move_data)) {
                  this->_release_flags(move_data, false, pos4, pos3, pos2, pos1);
                  return false;
               }
            }
            first_new.threadID = std::this_thread::get_id();
            if (additional == 2) {
               second_new.threadID = first_new.threadID;
               this->_release_flags(move_data, true, second_new);
            }
            this->_release_flags(move_data, true, first_new, pos4, pos3);
            if (additional == 1)
               this->_release_flags(move_data, true, pos2);
            return true;
         }
         bool _prepare_insert(node& z) {
            auto& zp = z.parent;
            if (!zp.in_use.compare_exchange(false, true))
               return false;
            if (zp != z.parent) {
               zp.in_use = false;
               return false;
            }
            node& uncle = (z == z.parent.left) ? z.parent.right : z.parent.left;
            if (!uncle.in_use.compare_exchange(false, true)) {
               z.parent.in_use = false;
               return false;
            }
            if (!this->_get_flags_and_markers_above(z.parent, z)) {
               z.parent.in_use = uncle.in_use = false;
               return false;
            }
            return true;
         }
         //
         void insert(node& x) {
            auto& z = this->dummy;
            auto& y = this->root;
            while (y != this->dummy) {
               z = y;
               if (x < y)
                  y = y.left;
               else
                  y = y.right;
            }
            x.parent = z;
            if (z == this->dummy) {
               this->root = x;
            } else if (x < z)
               z.left = x;
            else
               z.right = x;
            x.left  = this->dummy;
            x.right = this->dummy;
            x.color = node_color::red;
            this->_insert_fixup(x);
         }
         void remove(node& z) {
            node& y = this->dummy;
            if (!z.left || !z.right) {
               y = z;
            } else {
               y = SUCCESSOR(z); // next key
            }
            node& x = y.left ? y.left : y.right;
            x.parent = y.parent;
            if (!y.parent)
               this->root = x;
            else {
               if (y == y.parent.left)
                  y.parent.left = x;
               else
                  y.parent.right = x;
            }
            if (y != z)
               z.data = y.data;
            if (y.color == node_color::black)
               this->_delete_fixup(x);
         }
         void _delete_fixup(node& x) {
            while (x != this->root && x.color == node_color::black) {
               if (x == x.parent.left) {
                  auto& w = x.parent.right;
                  if (w.color == node_color::red) {
                     w.color = node_color::black;
                     x.parent.color = node_color::red;
                     this->rotate_left(x.parent);
                     w = x.parent.right;
                  }
                  if (w.left.color == node_color::black && x.right.color == node_color::black) {
                     w.color = node_color::red;
                     x = x.parent;
                  } else {
                     if (w.right.color == node_color::black) {
                        x.left.color = node_color::black;
                        w.color = node_color::red;
                        this->rotate_right(w);
                        w = x.parent.right;
                     }
                     w.color = x.parent.color;
                     x.parent.color = node_color::black;
                     w.right.color = node_color::black;
                     this->rotate_left(x.parent);
                     x = this->root;
                  }
               } else
                  x.parent = x.parent.parent.right;
            }
            x.color = node_color::black;
         }
         void _insert_fixup(node& x) {
            node& p = this->dummy;
            while ((p = x.parent).color == node_color::red) {
               auto& gp = p.parent;
               if (p == gp.left) {
                  auto& y = gp.right;
                  if (y.color == node_color::red) {
                     p.color = node_color::black;
                     y.color = node_color::black;
                     gp.color = node_color::red;
                     //x = gp;
                     x = this->_move_inserter_up(x);
                  } else {
                     if (x == p.right) {
                        x = p;
                        this->rotate_left(x);
                        p  = x.parent;
                        gp = p.parent;
                     }
                     p.color = node_color::black;
                     gp.color = node_color::red;
                     this->rotate_right(gp);
                  }
               } else
                  x.parent = gp.right;
            }
            this->root.color = node_color::black;
         }
         void rotate_right(node& y) {
            auto& x = y.left;
            y.left = x.right;
            x.right.parent = y;
            x.parent = y.parent;
            if (y.parent == this->root)
               this->root = x;
            else if (y == y.parent.left)
               y.parent.left = x;
            else
               y.parent.right = x;
            x.right = y;
            y.parent = x;
         }
         void rotate_left(node& x) {
            auto& y = x.right;
            x.right = y.left;
            y.left.parent = x;
            y.parent = x.parent;
            if (!x.parent) {
               this->root = y;
            } else if (x == x.parent.left)
               x.parent.left = y;
            else
               x.parent.right = y;
            y.left = x;
            x.parent = y;
         }
   };
}