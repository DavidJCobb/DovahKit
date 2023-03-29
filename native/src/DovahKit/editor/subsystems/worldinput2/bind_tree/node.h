#pragma once
#include <type_traits>
#include <QVector>

namespace dovahkit::subsystems::worldinput {
   class core;
}

namespace dovahkit::subsystems::worldinput::binds {
   enum class node_type {
      root,
      editor_mode,
      input,
   };

   namespace nodes {
      class root;
      class editor_mode;
      class input;
   }
   namespace impl {
      template<node_type nt> struct type_to_class;
      template<> struct type_to_class<node_type::root> { using type = nodes::root; };
      template<> struct type_to_class<node_type::editor_mode> { using type = nodes::editor_mode; };
      template<> struct type_to_class<node_type::input> { using type = nodes::input; };
   }
   template<node_type nt> using type_to_class = impl::type_to_class<nt>::type;

   class node {
      public:
         const node_type type;

      protected:
         node* parent = nullptr;
         QVector<node*> children; // owned

      protected:
         node(node_type t);
      public:
         ~node();

         void append(node& o);
         void insert(node& o, size_t before);
         void remove(node& o);
         inline int index_of(const node& o) const { return this->children.indexOf(const_cast<node*>(&o)); }

         inline node* parent_node() const noexcept { return this->parent; }
         inline const QVector<node*>& child_nodes() const noexcept { return this->children; }

         template<typename subclass> requires std::is_base_of_v<node, subclass> subclass* as() {
            if (this->type == subclass::my_type)
               return (subclass*)this;
            return nullptr;
         }
         template<typename subclass> requires std::is_base_of_v<node, subclass> const subclass* as() const {
            if (this->type == subclass::my_type)
               return (subclass*)this;
            return nullptr;
         }

         node* clone() const;

         // Used at the start of a frame, when traversing up from the active node to see if it and its ancestors are still active. 
         // Only relevant for editor modes and modifiers.
         virtual bool check_still_active(core&) const = 0;

         // Returns true if this node is a "parent" of some kind, or false otherwise.
         virtual bool is_valid_parent() const = 0;

      protected:
         virtual node* _clone_impl() const = 0;
   };
}
