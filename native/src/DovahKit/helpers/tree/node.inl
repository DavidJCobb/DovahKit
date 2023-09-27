#pragma once
#include <cassert>
#include "./node.h"
#include "../unreachable.h"

#pragma push_macro("CLASS_NAME")
#pragma push_macro("TEMPLATE_PARAMS")
#define TEMPLATE_PARAMS template<typename... DataTypes> requires (sizeof...(DataTypes) > 0)
#define CLASS_NAME node<DataTypes...>

namespace cobb {
   TEMPLATE_PARAMS
   template<typename Data>
   /*static*/ constexpr const size_t CLASS_NAME::_data_offset() noexcept {
      using type = typed_node<node, Data>;
      return offsetof(type, data);
   }

   TEMPLATE_PARAMS
   /*static*/ constexpr const CLASS_NAME::_offset_list CLASS_NAME::_all_data_offsets() noexcept {
      std::array<size_t, all_data_types::count> out = {};

      size_t i = 0;
      all_data_types::for_each([&out, &i]<typename Data>() {
         out[i++] = _data_offset<Data>();
      });

      return out;
   }

   TEMPLATE_PARAMS
   /*static*/ constexpr const size_t CLASS_NAME::_fixed_data_offset() noexcept {
      size_t offset   = 0;
      bool   all_same = all_data_types::for_each_until_false([&offset]<typename Data>() {
         constexpr auto o = _data_offset<Data>();
         if (offset && offset != o)
            return false;
         offset = o;
         return true;
      });
      if (!all_same)
         return 0;
      return offset;
   }

   //

   TEMPLATE_PARAMS
   template<typename Data>
   constexpr const typed_node<CLASS_NAME, Data>* CLASS_NAME::as() const noexcept requires supports_data_type<Data> {
      if (this->_type == all_data_types::template index_of_type<Data>)
         return (const typed_node<node, Data>*)this;
      return nullptr;
   }

   //

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::append_child(node& child) {
      assert(this->can_have_children());
      if (child.parent() == this)
         return;
      assert(child.parent() == nullptr);

      // `child` must not be of a type flagged as a root node
      assert(parameters::flags_per_data_type<node_data_attribute::root>::has_flag(child._type) == false);

      this->children._list.push_back(&child);
      child._parent = this;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::can_have_children() const noexcept {
      return parameters::flags_per_data_type<node_data_attribute::leaf>::has_flag(this->_type) == false;
   }

   TEMPLATE_PARAMS
   void CLASS_NAME::clear_all_children() {
      if (!this->can_have_children()) {
         assert(this->children.empty());
      }
      for (auto* child : this->children) {
         //
         // NOTE: This actually fails to meet the requirements for constexpr execution. 
         //       We lose type information on a child when we store it (i.e. we don't 
         //       know what data type it holds), and we don't use a virtual destructor, 
         //       so as far as the compiler knows, we can't properly run the subclass 
         //       destructor. We manually destroy the data as per the above, but the 
         //       compiler can't know that, and "Trust me, bro" won't fly in constexpr.
         //
         delete child;
      }
      this->children._list.clear();
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME* CLASS_NAME::clone(bool shallow) const noexcept requires (_clonable) {
      using clone_handler_t = node*(*)(const node&);
      constexpr const auto handlers = []() {
         std::array<clone_handler_t, all_data_types::count> out = {};
         size_t i = 0;
         all_data_types::for_each([&out, &i]<typename Data>() {
            out[i++] = [](const node& src) {
               node* copy;
               if constexpr (std::is_copy_constructible_v<Data>) {
                  copy = node::make<Data>(src.as<Data>()->data);
               } else if constexpr (std::is_default_constructible_v<Data>) {
                  auto* typed_copy = node::make<Data>();
                  copy = typed_copy;
                  typed_copy->data = src.as<Data>()->data;
               } else {
                  cobb::unreachable();
               }
               return copy;
            };
         });
         return out;
      }();

      node* copy = (handlers[this->_type])(*this);
      if (!shallow) {
         for (const node* child : this->children)
            copy->append_child(*child->clone(false));
      }
      return copy;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::contains(const node& descendant) const noexcept {
      const node* parent = &descendant;
      for (; parent; parent = parent->_parent) {
         if (parent == this)
            return true;
      }
      return false;
   }

   TEMPLATE_PARAMS
   constexpr size_t CLASS_NAME::index_of_child(const node& child) const noexcept {
      constexpr const auto no_index = (size_t)-1;

      if (!this->can_have_children())
         return no_index;

      if (child.parent() != this)
         return no_index;

      auto& list = this->children._list;
      for (size_t i = 0; i < list.size(); ++i)
         if (list[i] == &child)
            return i;

      return no_index;
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::insert_child(node& child, size_t at) {
      assert(this->can_have_children());
      assert(child.parent() == nullptr);

      // `child` must not be of a type flagged as a root node
      assert(parameters::flags_per_data_type<node_data_attribute::root>::has_flag(child._type) == false);

      auto& list = this->children._list;
      if (at == list.size()) {
         list.push_back(&child);
      } else {
         list.insert(list.cbegin() + at, &child);
      }
      child._parent = this;
   }

   TEMPLATE_PARAMS
   constexpr const CLASS_NAME& CLASS_NAME::nth_child(size_t i) const {
      assert(this->can_have_children());
      return *(this->children[i]);
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::remove_child(node& child) {
      assert(this->can_have_children());
      assert(child.parent() == this);
      auto& list = this->children._list;
      list.erase(std::remove(list.begin(), list.end(), &child), list.end());
      child._parent = nullptr;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::operator==(const node& other) const noexcept requires (_equality_comparable) {
      if (this->_type != other._type)
         return false;
      if (this->children.size() != other.children.size())
         return false;
      
      using compare_handler_t = bool(*)(const node&, const node&);
      constexpr const auto handlers = []() {
         std::array<compare_handler_t, all_data_types::count> out = {};
         size_t i = 0;
         all_data_types::for_each([&out, &i]<typename Data>() {
            out[i++] = [](const node& a, const node& b) {
               return a.as<Data>()->data == b.as<Data>()->data;
            };
         });
         return out;
      }();

      bool data_is_equal = (handlers[this->_type])(*this, other);
      if (!data_is_equal)
         return false;

      size_t size = this->children.size();
      for (size_t i = 0; i < size; ++i)
         if (*this->children[i] != *other.children[i])
            return false;

      return true;
   }
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")