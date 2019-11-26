#pragma once
#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

class FormStub;

class FormStubMap { // map of form IDs to form stubs
   protected:
      enum class node_color : uint8_t {
         black, // root node, leaf nodes, and any child of a red node
         red,
      };
      //
      struct lock {
         public:
            std::atomic<bool> lock = false;
            std::thread::id   owner;
            //
            bool try_lock() {
               bool expected = false;
               return this->lock.compare_exchange_strong(expected, true);
            }
            inline bool is_locked() const { return this->lock; }
            inline void unlock() {
               this->lock = false;
            }
      };
      //
      struct element {
         uint32_t  key;
         FormStub* value = nullptr;
      };
      struct node {
         element    data;
         node_color color = node_color::black;
         node&      parent;
         node&      left;  // child
         node&      right; // child
         bool       is_dummy = false;
         lock       lock;
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
            if (this->parent.left == *this)
               return this->parent.right;
            return this->parent.left;
         }
         inline operator bool() const { return !this->is_dummy; }
         inline bool operator<(const node& other) { return this->data.key < other.data.key; }
         //
         inline bool try_lock() { return this->lock.try_lock(); }
         inline void unlock() { return this->lock.unlock(); }
      };
      struct _move_up_struct {
         node* goal = nullptr;
         std::vector<node*> nodes;
         std::thread::id owner;
         std::thread::id defer_to[2];
         //
         bool contains(node& other) const;
      };
   protected:
      node dummy; // used to ensure every node has two children // nil[T], a black node // leaves are "nil" nodes
      node root;
      //
      void insert(node& x);
      void remove(node& z);
      //
      void rotate_right(node& y);
      void rotate_left(node& x);
      //
      void _delete_fixup(node& x);
      void _insert_fixup(node& x);
      //
      bool _apply_move_up_rule(node& x, node& w, _move_up_struct& out);
      bool _spacing_rule_met(node& t, node& z, std::thread::id defer_to);
      void _release_flags(_move_up_struct& move_data, bool success, int node_count, ...);
      bool _get_flags_for_markers(node& start, _move_up_struct& move_data, node& pos1, node& pos2, node& pos3, node& pos4);
      bool _get_flags_and_markers_above(node& start, int additional);
      bool _prepare_insert(node& z);

};