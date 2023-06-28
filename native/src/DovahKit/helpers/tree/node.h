#pragma once
#include <array>
#include <bit>
#include <type_traits>
#include <vector>
#include "../class_array.h"

#include "./impl/child_list.h"
#include "./impl/children_view.h"
#include "./impl/data_destroyer.h"
#include "./impl/parameters.h"
#include "./impl/strip_node_data_options.h"
#include "./impl/typecode.h"

#include "./node_data_attribute.h"
#include "./node_data_with_attributes.h"

namespace cobb {
   namespace impl::_node {
      template<typename Node>
      class children_view;
   }

   template<typename Node, typename Data> requires (Node::template supports_data_type<Data>)
   class typed_node;

   template<typename... DataTypes> requires (sizeof...(DataTypes) > 0)
   class node {
      friend class impl::_node::children_view<node>;
      protected:
         using parameters = impl::_node::parameters<DataTypes...>;
         using typecode   = parameters::typecode_t;

      public:
         static constexpr const bool can_any_node_have_children = parameters::attribute_flag_info<node_data_attribute::leaf>::count > 0;

      protected:
         using child_list = std::conditional_t<
            can_any_node_have_children,
            std::vector<node*>,
            typename parameters::dummy
         >;

      public:
         using all_data_types = parameters::all_types;

         using children_view = impl::_node::children_view<node>;
         using const_children_view = impl::_node::children_view<const node>;

         template<typename Data>
         static constexpr const bool supports_data_type = all_data_types::template contains_type<Data>;

         template<typename Data>
         static constexpr const bool data_type_is_leaf = parameters::template data_has_attribute<Data, node_data_attribute::leaf>;

         static constexpr const size_t data_type_count = all_data_types::count;

      protected:
         constexpr node(typecode t) : _type(t) {}

         [[no_unique_address]] const typecode _type;
         //
         bool  _undergoing_typed_destroy : 1 = false;
         node* _parent = nullptr;

         template<typename Data>
         static constexpr const size_t _children_offset() noexcept;
         template<typename Data>
         static constexpr const size_t _data_offset() noexcept;

         using _offset_list = std::array<size_t, data_type_count>;
         static constexpr const _offset_list _all_children_offsets() noexcept;
         static constexpr const _offset_list _all_data_offsets() noexcept;

         static constexpr const size_t _fixed_children_offset() noexcept;
         static constexpr const size_t _fixed_data_offset() noexcept;

         void* _untyped_data() {
            auto addr = (std::intptr_t)this;
            if constexpr (_fixed_data_offset()) {
               addr += _fixed_data_offset();
            } else {
               addr += _all_data_offsets()[this->_type];
            }
            return (void*)addr;
         }

         const std::vector<node*>* _child_list() const {
            auto addr = (std::intptr_t)this;
            if constexpr (_fixed_children_offset()) {
               addr += _fixed_children_offset();
            } else {
               auto o = _all_children_offsets()[this->_type];
               if (!o)
                  return nullptr;
               addr += o;
            }
            return (const std::vector<node*>*)addr;
         }
         std::vector<node*>* _child_list() {
            return const_cast<std::vector<node*>*>(std::as_const(*this)._child_list());
         }

      public:
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
               this->clear_all_children();
            }
         }

         template<typename Data, typename... Args> requires supports_data_type<Data>
         static constexpr typed_node<node, Data>* make(Args&&... args) {
            return new typed_node<node, Data>(std::forward<Args>(args)...);
         }

         //

         void append_child(node&);

         bool can_have_children() const noexcept;

         size_t child_count() const noexcept;

         void clear_all_children();

         bool contains(const node&) const noexcept;

         size_t index_of_child(const node&) const noexcept;

         void insert_child(node&, size_t at);

         const node& nth_child(size_t) const;
         node& nth_child(size_t i) {
            return const_cast<node&>(std::as_const(*this).nth_child(i));
         }

         void remove_child(node&);

         //

         template<typename Data>
         constexpr const typed_node<node, Data>* as() const noexcept requires supports_data_type<Data>;
         //
         template<typename Data>
         constexpr typed_node<node, Data>* as() noexcept requires supports_data_type<Data> {
            return const_cast<typed_node<node, Data>*>(std::as_const(*this).as<Data>());
         }

         constexpr const node* parent() const noexcept { return this->_parent; }
         constexpr node* parent() noexcept { return const_cast<node*>(std::as_const(*this).parent()); }
   };

   template<typename Node, typename Data> requires (Node::template supports_data_type<Data>)
   class typed_node : public Node {
      protected:
         using child_list = impl::_node::child_list<Node, Data>;
      public:
         using value_type = Data;

      public:
         template<typename... Args>
         constexpr typed_node(Args&&... args) : Node(Node::all_data_types::template index_of_type<Data>), data(std::forward<Args>(args)...) {
         }

         constexpr ~typed_node() {
            this->_undergoing_typed_destroy = true;
            this->clear_all_children();
         }

         [[no_unique_address]] child_list children;
         value_type data;
   };
}

#include "./node.inl"