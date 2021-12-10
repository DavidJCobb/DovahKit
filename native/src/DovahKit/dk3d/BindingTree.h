#pragma once
#include <QString>
#include "enums/EditorMode.h"
#include "enums/InputDevice.h"
#include "tools/_options.h"
#include "BoundInput.h"

class DK3DInputHandler;
namespace DK3D::tools {
   class base;
}

namespace DK3D::binding_tree_nodes {
   enum class node_type {
      root,
      editor_mode,
      input,
   };

   class root;
   class editor_mode;
   class input;
   namespace impl {
      template<node_type nt> struct type_to_class;
      template<> struct type_to_class<node_type::root> { using type = root; };
      template<> struct type_to_class<node_type::editor_mode> { using type = editor_mode; };
      template<> struct type_to_class<node_type::input> { using type = input; };
   }
   template<node_type nt> using type_to_class = impl::type_to_class<nt>::type;

   class base {
      public:
         const node_type type;

      protected:
         base* parent = nullptr;
         QVector<base*> children; // owned

      public:
         base(node_type t) : type(t) {}
         ~base() {
            for (auto* p : this->children)
               if (p)
                  delete p;
            this->children.clear();
         }

         inline void append(base& o) {
            assert(o.type != node_type::root);
            if (o.parent) {
               if (o.parent == this)
                  return;
               o.parent->remove(o);
            }
            o.parent = this;
            this->children.append(&o);
         }
         inline void remove(base& o) {
            assert(o.type != node_type::root);
            if (o.parent != this)
               return;
            o.parent = nullptr;
            this->children.removeOne(&o);
         }
         inline int index_of(base& o) const { return this->children.indexOf(&o); }

         inline base* parent_node() const noexcept { return this->parent; }
         inline const QVector<base*>& child_nodes() const noexcept { return this->children; }

         template<typename subclass> requires std::is_base_of_v<base, subclass> subclass* as() {
            if (this->type == subclass::my_type)
               return (subclass*)this;
            return nullptr;
         }
         template<typename subclass> requires std::is_base_of_v<base, subclass> const subclass* as() const {
            if (this->type == subclass::my_type)
               return (subclass*)this;
            return nullptr;
         }

         virtual base* clone() const {
            auto* copy = this->_clone_impl();
            auto& list = this->children;
            auto  size = list.size();
            copy->children.resize(size);
            for (decltype(size) i = 0; i < size; ++i) {
               auto* c = list[i]->clone();
               c->parent = copy;
               copy->children[i] = c;
            }
            return copy;
         }

         // Used at the start of a frame, when traversing up from the active node to see if it and its ancestors are still active. 
         // Only relevant for editor modes and modifiers.
         virtual bool check_still_active(DK3DInputHandler&) const = 0;

         // Used after (check_still_active), to enter an editor mode, or execute editor functions or newly-activated modifiers, as 
         // appropriate.
         virtual bool check_is_now_active(DK3DInputHandler&) const = 0;

         // Returns true if this node is a "parent" of some kind, or false otherwise.
         virtual bool is_valid_parent() const = 0;

      protected:
         virtual base* _clone_impl() const = 0;
   };

   class root final : public base {
      public:
         static constexpr node_type my_type = node_type::root;
      public:
         root() : base(my_type) {}

         virtual bool check_still_active(DK3DInputHandler&) const override { return true; }
         virtual bool check_is_now_active(DK3DInputHandler&) const override { return true; }

      protected:
         virtual base* _clone_impl() const override { return new root; };
   };

   class editor_mode final : public base {
      public:
         static constexpr node_type my_type = node_type::editor_mode;
      public:
         editor_mode(EditorMode em) : base(my_type), mode(em) {}

         EditorMode mode;

         virtual bool check_still_active(DK3DInputHandler&) const override { return false; }  // TODO: should return (true) if the current editor mode is (mode)
         virtual bool check_is_now_active(DK3DInputHandler&) const override { return false; } // TODO: should return (true) if the current editor mode is (mode)

      protected:
         virtual base* _clone_impl() const override { return new editor_mode(mode); };
   };

   class input : public base {
      public:
         static constexpr node_type my_type = node_type::input;
      public:
         input() : base(my_type) {}

         QString    name;
         bool       is_modifier = false;
         BoundInput mapping;
         //
         const tools::base*  tool = nullptr;
         tools::option_union params;

         virtual bool check_still_active(DK3DInputHandler&) const override; // modifiers: check if the key is still active ("while") or if it isn't active (non-"while" acts as toggle)
         virtual bool check_is_now_active(DK3DInputHandler&) const override;

      protected:
         virtual base* _clone_impl() const override;
   };
}

namespace DK3D {
   class BindingTree {
      public:
         using node_type = binding_tree_nodes::base;
         using root_type = binding_tree_nodes::root;
      public:
         BindingTree(InputDevice d) : device(d) {
            this->root = new binding_tree_nodes::root;
         }

         BindingTree& operator=(const BindingTree& o) {
            if (this->root)
               delete this->root;
            this->root = (binding_tree_nodes::root*)o.root->clone();
            return *this;
         }

         InputDevice device;
         root_type*  root   = nullptr;
         node_type*  active = nullptr;

         void process();
   };
}