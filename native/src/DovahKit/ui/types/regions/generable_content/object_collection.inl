#pragma once
#include "./object_collection.h"

namespace ui::types::regions::generable_content {
   #pragma region object_collection::object
      constexpr bool object_collection::object::contains(const object& node) const noexcept {
         for (auto* parent = node.parent; parent; parent = parent->parent)
            if (parent == this)
               return true;
         return false;
      }
      constexpr size_t object_collection::object::index_of(const object& node) const noexcept {
         for (size_t i = 0; i < this->children.size(); ++i)
            if (this->children[i].get() == &node)
               return i;
         return index_of_none;
      }

      constexpr void object_collection::object::clamp_slope_to_ancestor_range(bool descendants_too) {
         if (this->parent) {
            auto& this_params = this->data.params;
            auto& ance_params = this->parent->data.params;
            if (this_params.slope.min < ance_params.slope.min)
               this_params.slope.min = ance_params.slope.min;
            if (this_params.slope.max < ance_params.slope.max)
               this_params.slope.max = ance_params.slope.max;
         }
         if (descendants_too) {
            for (auto& child_ptr : this->children)
               child_ptr->clamp_slope_to_ancestor_range(true);
         }
      }
   #pragma endregion

   #pragma region object_collection
      constexpr bool object_collection::empty() const noexcept {
         return this->objects.empty();
      }
      constexpr size_t object_collection::index_of(const object& node) const noexcept {
         for (size_t i = 0; i < this->objects.size(); ++i)
            if (this->objects[i].get() == &node)
               return i;
         return object::index_of_none;
      }
   #pragma endregion
}