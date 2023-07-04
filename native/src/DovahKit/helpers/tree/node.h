#pragma once
#include <array>
#include <bit>
#include <type_traits>
#include <vector>
#include "../class_array.h"

#include "./impl/child_list.h"
#include "./impl/data_destroyer.h"
#include "./impl/parameters.h"
#include "./impl/strip_node_data_options.h"
#include "./impl/typecode.h"

#include "./node_data_attribute.h"
#include "./node_data_with_attributes.h"

namespace cobb {
   template<typename Node, typename Data> requires (Node::template supports_data_type<Data>)
   class typed_node;

   template<typename... DataTypes> requires (sizeof...(DataTypes) > 0)
   class node {
      protected:
         using parameters = impl::_node::parameters<DataTypes...>;
         using typecode   = parameters::typecode_t;

         static constexpr const bool _clonable = (
            (std::is_default_constructible_v<typename impl::_node::strip_node_data_options<DataTypes>::type> && ...)
            ||
            (std::is_copy_constructible_v<typename impl::_node::strip_node_data_options<DataTypes>::type> && ...)
         );
         static constexpr const bool _equality_comparable = (std::equality_comparable<typename impl::_node::strip_node_data_options<DataTypes>::type> && ...);

      public:
         using all_data_types = parameters::all_types;

         template<typename Data>
         static constexpr const bool supports_data_type = all_data_types::template contains_type<Data>;

      protected:
         constexpr node(typecode t) : _type(t) {}

         [[no_unique_address]] const typecode _type;
         //
         bool  _undergoing_typed_destroy : 1 = false;
         node* _parent = nullptr;
         //
      public:
         impl::_node::child_list<node> children;

      protected:
         template<typename Data>
         static constexpr const size_t _data_offset() noexcept;

         using _offset_list = std::array<size_t, all_data_types::count>;
         static constexpr const _offset_list _all_data_offsets() noexcept;

         static constexpr const size_t _fixed_data_offset() noexcept;

         void* _untyped_data() {
            auto addr = (std::intptr_t)this; // this is one of the things disqualifying us from being constexpr
            if constexpr (_fixed_data_offset()) {
               addr += _fixed_data_offset();
            } else {
               addr += _all_data_offsets()[this->_type];
            }
            return (void*)addr;
         }

      public:
         constexpr node(const node&) = delete;
         node& operator=(const node&) = delete;

         constexpr ~node() {
            //
            // We've chosen not to use polymorphism in order to potentially avoid the 
            // relevant overhead -- "you don't pay for what you don't use" taken to the 
            // extreme.
            // 
            // This means that someone may delete a node via a bare `node` pointer or via 
            // a `typed_node` pointer. We need to make sure to destroy the node data held 
            // in the `typed_node` subclass either way, and we need to make sure we only 
            // destroy it once. We do this by having the subclass set a flag on the base 
            // class, which we can use to tell if the subclass destructor ever ran.
            //
            if (!this->_undergoing_typed_destroy) {
               constexpr const bool all_trivially_destructible = (std::is_trivially_destructible_v<typename impl::_node::strip_node_data_options<DataTypes>::type> && ...);
               if constexpr (!all_trivially_destructible) {
                  impl::_node::data_destroyer<all_data_types>::destroy(this->_untyped_data(), this->_type);
               }
            }
            this->clear_all_children();
         }

         template<typename Data, typename... Args> requires (supports_data_type<Data>)
         static constexpr typed_node<node, Data>* make(Args&&... args) {
            return new typed_node<node, Data>(std::forward<Args>(args)...);
         }

         template<typename Data> requires (supports_data_type<Data>)
         static constexpr typed_node<node, Data>* from_data(const Data& data) {
            if constexpr (std::is_copy_constructible_v<Data>) {
               return new typed_node<node, Data>(data);
            } else {
               auto* out = new typed_node<node, Data>();
               out->data = data;
               return out;
            }
         }

         //

         template<typename Data>
         constexpr const typed_node<node, Data>* as() const noexcept requires supports_data_type<Data>;
         //
         template<typename Data>
         constexpr typed_node<node, Data>* as() noexcept requires supports_data_type<Data> {
            return const_cast<typed_node<node, Data>*>(std::as_const(*this).as<Data>());
         }

         //

         // Takes ownership of the child node: when this node is deleted, the child node will be, too.
         // Requires that the child node not already have a parent (unless that parent is `this`).
         constexpr void append_child(node&);

         constexpr bool can_have_children() const noexcept;

         void clear_all_children();

         constexpr node* clone(bool shallow = false) const noexcept requires (_clonable);

         // Checks if the argument is a child or descendant node of `this`.
         constexpr bool contains(const node&) const noexcept;

         constexpr size_t index_of_child(const node&) const noexcept;

         // Takes ownership of the child node: when this node is deleted, the child node will be, too.
         // Requires that the child node not already have a parent.
         constexpr void insert_child(node&, size_t at);

         constexpr const node& nth_child(size_t) const;
         constexpr node& nth_child(size_t i) {
            return const_cast<node&>(std::as_const(*this).nth_child(i));
         }

         constexpr const node* parent() const noexcept { return this->_parent; }
         constexpr node* parent() noexcept { return const_cast<node*>(std::as_const(*this).parent()); }

         // Cedes ownership of the child node: it is not deleted; it is simply abandoned.
         constexpr void remove_child(node&);

         // Deep compare, not shallow.
         constexpr bool operator==(const node&) const noexcept requires (_equality_comparable);
   };

   template<typename Node, typename Data> requires (Node::template supports_data_type<Data>)
   class typed_node final : public Node {
      public:
         using value_type = Data;

      public:
         template<typename... Args> requires std::is_constructible_v<Data, Args...>
         constexpr typed_node(Args&&... args) : Node(Node::all_data_types::template index_of_type<Data>), data(std::forward<Args>(args)...) {
         }

         constexpr ~typed_node() {
            this->_undergoing_typed_destroy = true;
         }

         value_type data;
   };
}

#include "./node.inl"