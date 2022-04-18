#include "scene_item_handle.h"
#include "scene.h"
#include "surface_renderer.h"

namespace vulkanDK {
   rendered_light& rendered_light_handle::operator*() {
      return this->owner->scene.lights[this->index];
   }
   rendered_light* rendered_light_handle::operator->() {
      if (this->owner && this->index != -1)
         return &this->owner->scene.lights[this->index];
      return nullptr;
   }
   void rendered_light_handle::destroy() {
      if (empty())
         return;
      this->owner->remove_light(this->index);
      this->index = -1;
      this->owner = nullptr;
   }

   rendered_mesh& rendered_mesh_handle::operator*() {
      return this->owner->scene.meshes[this->index];
   }
   rendered_mesh* rendered_mesh_handle::operator->() {
      if (this->owner && this->index != -1)
         return &this->owner->scene.meshes[this->index];
      return nullptr;
   }
   void rendered_mesh_handle::destroy() {
      if (empty())
         return;
      this->owner->remove_mesh(this->index);
      this->index = -1;
      this->owner = nullptr;
   }
}