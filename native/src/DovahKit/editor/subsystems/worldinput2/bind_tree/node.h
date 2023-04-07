#pragma once
#include <type_traits>
#include <QVector>

namespace dovahkit::subsystems::worldinput2 {
   class core;
}

namespace dovahkit::subsystems::worldinput2::binds {
   enum class node_type {
      root,
      //
      bound_tool,
      editor_mode,
      modifier,
   };

   namespace nodes {
      class root;
      class bound_tool;
      class editor_mode;
      class modifier;

      class abstract_input_node;
   }
   namespace impl {
      template<node_type nt> struct type_to_class;
      template<> struct type_to_class<node_type::root> { using type = nodes::root; };
      template<> struct type_to_class<node_type::bound_tool> { using type = nodes::bound_tool; };
      template<> struct type_to_class<node_type::editor_mode> { using type = nodes::editor_mode; };
      template<> struct type_to_class<node_type::modifier> { using type = nodes::modifier; };
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
            return const_cast<subclass*>(std::as_const(*this).as<subclass>());
         }
         template<typename subclass> requires std::is_base_of_v<node, subclass> const subclass* as() const {
            if constexpr (std::is_same_v<subclass, nodes::abstract_input_node>) {
               switch (this->type) {
                  case node_type::bound_tool:
                  case node_type::modifier:
                     return (subclass*)this;
               }
            } else {
               if (this->type == subclass::my_type)
                  return (subclass*)this;
            }
            return nullptr;
         }

         void clear_descendants_input_sequence_progress();

         node* clone() const;

         // Returns true if this node is a "parent" of some kind, or false otherwise.
         virtual bool is_valid_parent() const = 0;

      protected:
         virtual node* _clone_impl() const = 0;
   };
}
