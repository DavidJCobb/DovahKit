#include "scene_item_handle.h"
#include "scene.h"
#include "surface_renderer.h"

namespace vulkanDK {
   #pragma region rendered_bounds_handle
   rendered_bounds& rendered_bounds_handle::operator*() {
      return this->owner->scene.entities_of_type<value_type>()[this->index];
   }
   rendered_bounds* rendered_bounds_handle::operator->() {
      if (this->owner && this->index != -1)
         return &this->owner->scene.entities_of_type<value_type>()[this->index];
      return nullptr;
   }
   void rendered_bounds_handle::destroy() {
      if (empty())
         return;
      this->owner->remove_bounds(this->index);
      this->index = -1;
      this->owner = nullptr;
   }
   #pragma endregion

   #pragma region rendered_landscape_handle
   rendered_landscape& rendered_landscape_handle::operator*() {
      return this->owner->scene.entities_of_type<value_type>()[this->index];
   }
   rendered_landscape* rendered_landscape_handle::operator->() {
      if (this->owner && this->index != -1)
         return &this->owner->scene.entities_of_type<value_type>()[this->index];
      return nullptr;
   }
   void rendered_landscape_handle::destroy() {
      if (empty())
         return;
      this->owner->remove_landscape(this->index);
      this->index = -1;
      this->owner = nullptr;
   }
   #pragma endregion

   #pragma region rendered_light_handle
   rendered_light& rendered_light_handle::operator*() {
      return this->owner->scene.entities_of_type<value_type>()[this->index];
   }
   rendered_light* rendered_light_handle::operator->() {
      if (this->owner && this->index != -1)
         return &this->owner->scene.entities_of_type<value_type>()[this->index];
      return nullptr;
   }
   void rendered_light_handle::destroy() {
      if (empty())
         return;
      this->owner->remove_light(this->index);
      this->index = -1;
      this->owner = nullptr;
   }
   #pragma endregion

   #pragma region rendered_mesh_handle
   rendered_mesh& rendered_mesh_handle::operator*() {
      return this->owner->scene.entities_of_type<value_type>()[this->index];
   }
   rendered_mesh* rendered_mesh_handle::operator->() {
      if (this->owner && this->index != -1)
         return &this->owner->scene.entities_of_type<value_type>()[this->index];
      return nullptr;
   }
   void rendered_mesh_handle::destroy() {
      if (empty())
         return;
      (*this)->owning_nif = nullptr;
      this->owner->remove_mesh(this->index);
      this->index = -1;
      this->owner = nullptr;
   }
   #pragma endregion
}