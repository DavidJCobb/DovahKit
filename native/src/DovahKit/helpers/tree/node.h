#pragma once
#include <array>
#include <type_traits>
#include <vector>
#include "../class_array.h"

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

      protected:
         constexpr node(typecode t) : _type(t) {}

         [[no_unique_address]] const typecode _type;
         bool  _is_typed_destroy : 1 = false;
         node* _parent = nullptr;
         [[no_unique_address]] child_list _children;

         template<typename Data>
         static constexpr const size_t _offset_of() noexcept {
            using node_subtype = typed_node<node, Data>; // offsetof is a macro and therefore chokes to death on commas
            return offsetof(node_subtype, data);
         }

         template<typename Data>
         static constexpr const auto _all_offsets() noexcept {
            std::array<size_t, all_data_types::count> out = {};
            
            size_t i = 0;
            all_data_types::for_each([&out, &i]<typename Data>() {
               out[i++] = _offset_of<Data>();
            });

            return out;
         }

         static constexpr const size_t _fixed_offset() noexcept {
            size_t offset   = 0;
            bool   all_same = all_data_types::for_each_until_false([&offset]<typename Data>() {
               constexpr auto o = _offset_of<Data>();
               if (offset && offset != o)
                  return false;
               offset = o;
               return true;
            });
            if (!all_same)
               return 0;
            return offset;
         }

         void* _untyped_data() {
            auto addr = (std::intptr_t)this;
            if constexpr (_fixed_offset()) {
               return (void*)(addr + _fixed_offset());
            } else {
               return (void*)(addr + _all_offsets()[this->_type]);
            }
         }

      public:
         constexpr ~node() {
            constexpr const bool all_trivially_destructible = (std::is_trivially_destructible_v<typename impl::_node::strip_node_data_options<DataTypes>::type> && ...);
            if constexpr (!all_trivially_destructible) {
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
               if (!this->_is_typed_destroy)
                  impl::_node::data_destroyer<all_data_types>::destroy(this->_untyped_data(), this->_type);
            }
            if constexpr (can_any_node_have_children) {
               //
               // NOTE: This actually fails to meet the requirements for constexpr execution. 
               //       We lose type information on a child when we store it (i.e. we don't 
               //       know what data type it holds), and we don't use a virtual destructor, 
               //       so as far as the compiler knows, we can't properly run the subclass 
               //       destructor. We manually destroy the data as per the above, but the 
               //       compiler can't know that, and "Trust me, bro" won't fly in constexpr.
               //
               for (auto* child : this->_children)
                  delete child;
            }
         }

         template<typename Data, typename... Args> requires supports_data_type<Data>
         static constexpr typed_node<node, Data>* make(Args&&... args) {
            return new typed_node<node, Data>(std::forward<Args>(args)...);
         }

         //

         constexpr void append_child(node&);

         constexpr bool can_have_children() const noexcept;

         constexpr bool contains(const node&) const noexcept;

         constexpr size_t index_of_child(const node&) const noexcept;

         constexpr void insert_child(node&, size_t at);

         constexpr void remove_child(node&);

         //

         template<typename Data>
         constexpr const typed_node<node, Data>* as() const noexcept requires supports_data_type<Data>;
         //
         template<typename Data>
         constexpr typed_node<node, Data>* as() noexcept requires supports_data_type<Data> {
            return const_cast<typed_node<node, Data>*>(std::as_const(*this).as<Data>());
         }

         constexpr const const_children_view children() const noexcept { return const_children_view{ *this }; }
         constexpr children_view children() noexcept { return children_view{ *this }; }

         constexpr const node* parent() const noexcept { return this->_parent; }
         constexpr node* parent() noexcept { return const_cast<node*>(std::as_const(*this).parent()); }
   };

   template<typename Node, typename Data> requires (Node::template supports_data_type<Data>)
   class typed_node : public Node {
      public:
         template<typename... Args>
         constexpr typed_node(Args&&... args) : Node(Node::all_data_types::template index_of_type<Data>), data(std::forward<Args>(args)...) {}

         constexpr ~typed_node() {
            this->_is_typed_destroy = true;
         }

         Data data;
   };
}

#include "./node.inl"