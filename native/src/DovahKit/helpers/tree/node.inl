#pragma once
#include <cassert>
#include "./node.h"

#pragma push_macro("CLASS_NAME")
#pragma push_macro("TEMPLATE_PARAMS")
#define TEMPLATE_PARAMS template<typename... DataTypes> requires (sizeof...(DataTypes) > 0)
#define CLASS_NAME node<DataTypes...>

namespace cobb {
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::append_child(node& child) {
      if (child.parent() == this)
         return;
      assert(child.parent() == nullptr);
      assert(this->can_have_children());
      if constexpr (can_any_node_have_children) {
         this->_children.push_back(&child);
         child._parent = this;
      }
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::can_have_children() const noexcept {
      return parameters::flags_per_data_type<node_data_attribute::leaf>::has_flag(this->_type) == false;
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
      if constexpr (can_any_node_have_children) {
         if (child.parent() == this) {
            std::vector<node&>& list = this->_children;
            for (size_t i = 0; i < list.size(); ++i)
               if (list[i] == &child)
                  return i;
         }
      }
      return (size_t)-1;
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::insert_child(node& child, size_t at) {
      assert(child.parent() == nullptr);
      assert(this->can_have_children());
      if constexpr (can_any_node_have_children) {
         std::vector<node&>& list = this->_children;
         if (at == list.size()) {
            list.push_back(&child);
         } else {
            list.insert(list.cbegin() + at, &child);
         }
         child._parent = this;
      }
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::remove_child(node& child) {
      assert(child.parent() == this);
      if constexpr (can_any_node_have_children) {
         std::vector<node&>& list = this->_children;
         list.erase(std::remove(list.begin(), list.end(), &child), list.end());
         child._parent = nullptr;
      }
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