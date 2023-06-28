#pragma once
#include <cassert>
#include "./node.h"

#pragma push_macro("CLASS_NAME")
#pragma push_macro("TEMPLATE_PARAMS")
#define TEMPLATE_PARAMS template<typename... DataTypes> requires (sizeof...(DataTypes) > 0)
#define CLASS_NAME node<DataTypes...>

namespace cobb {
   TEMPLATE_PARAMS
   template<typename Data>
   /*static*/ constexpr const size_t CLASS_NAME::_children_offset() noexcept {
      if constexpr (data_type_is_leaf<Data>) {
         return 0;
      } else {
         using type = typed_node<node, Data>;
         return offsetof(type, children);
      }
   }

   TEMPLATE_PARAMS
   template<typename Data>
   /*static*/ constexpr const size_t CLASS_NAME::_data_offset() noexcept {
      using type = typed_node<node, Data>;
      return offsetof(type, data);
   }

   TEMPLATE_PARAMS
   /*static*/ constexpr const CLASS_NAME::_offset_list CLASS_NAME::_all_children_offsets() noexcept {
      std::array<size_t, all_data_types::count> out = {};

      size_t i = 0;
      all_data_types::for_each([&out, &i]<typename Data>() {
         out[i++] = _children_offset<Data>();
      });

      return out;
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
   /*static*/ constexpr const size_t CLASS_NAME::_fixed_children_offset() noexcept {
      size_t offset   = 0;
      bool   all_same = all_data_types::for_each_until_false([&offset]<typename Data>() {
         constexpr auto o = _children_offset<Data>();
         if (offset && offset != o)
            return false;
         offset = o;
         return true;
      });
      if (!all_same)
         return 0;
      return offset;
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

   TEMPLATE_PARAMS
   void CLASS_NAME::append_child(node& child) {
      if (child.parent() == this)
         return;
      assert(child.parent() == nullptr);
      assert(this->can_have_children());
      this->_child_list()->push_back(&child);
      child._parent = this;
   }

   TEMPLATE_PARAMS
   bool CLASS_NAME::can_have_children() const noexcept {
      return parameters::flags_per_data_type<node_data_attribute::leaf>::has_flag(this->_type) == false;
   }

   TEMPLATE_PARAMS
   void CLASS_NAME::clear_all_children() {
      if (auto* list = this->_child_list()) {
         auto& storage = *list;
         for (auto* child : storage)
            //
            // NOTE: This actually fails to meet the requirements for constexpr execution. 
            //       We lose type information on a child when we store it (i.e. we don't 
            //       know what data type it holds), and we don't use a virtual destructor, 
            //       so as far as the compiler knows, we can't properly run the subclass 
            //       destructor. We manually destroy the data as per the above, but the 
            //       compiler can't know that, and "Trust me, bro" won't fly in constexpr.
            //
            delete child;
         storage.clear();
      }
   }

   TEMPLATE_PARAMS
   size_t CLASS_NAME::child_count() const noexcept {
      if (auto* list = this->_child_list())
         return list->size();
      return 0;
   }

   TEMPLATE_PARAMS
   bool CLASS_NAME::contains(const node& descendant) const noexcept {
      const node* parent = &descendant;
      for (; parent; parent = parent->_parent) {
         if (parent == this)
            return true;
      }
      return false;
   }

   TEMPLATE_PARAMS
   size_t CLASS_NAME::index_of_child(const node& child) const noexcept {
      if (auto* list_ptr = this->_child_list()) {
         if (child.parent() == this) {
            auto& list = *list_ptr;
            for (size_t i = 0; i < list.size(); ++i)
               if (list[i] == &child)
                  return i;
         }
      }
      return (size_t)-1;
   }

   TEMPLATE_PARAMS
   void CLASS_NAME::insert_child(node& child, size_t at) {
      assert(child.parent() == nullptr);
      assert(this->can_have_children());
      std::vector<node*>& list = *this->_child_list();
      if (at == list.size()) {
         list.push_back(&child);
      } else {
         list.insert(list.cbegin() + at, &child);
      }
      child._parent = this;
   }

   TEMPLATE_PARAMS
   const CLASS_NAME& CLASS_NAME::nth_child(size_t i) const {
      assert(this->can_have_children());
      return (*this->_child_list())[i];
   }

   TEMPLATE_PARAMS
   void CLASS_NAME::remove_child(node& child) {
      assert(child.parent() == this);
      assert(this->can_have_children());
      std::vector<node*>& list = *this->_child_list();
      list.erase(std::remove(list.begin(), list.end(), &child), list.end());
      child._parent = nullptr;
   }

   TEMPLATE_PARAMS
   template<typename Data>
   constexpr const typed_node<CLASS_NAME, Data>* CLASS_NAME::as() const noexcept requires supports_data_type<Data> {
      if (this->_type == all_data_types::template index_of_type<Data>)
         return (const typed_node<node, Data>*)this;
      return nullptr;
   }
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")