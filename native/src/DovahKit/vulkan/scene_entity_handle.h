#pragma once
#include <algorithm> // std::swap
#include "helpers/passkey.h"

namespace vulkanDK {
   class rendered_bounds;
   class rendered_landscape;
   class rendered_light;
   class rendered_mesh;
   class surface_renderer;

   namespace impl {
      //
      // We use the curiously-recurring template pattern (wherein the base class is 
      // templated on its own subclass) to prevent cross-assignment of different 
      // handle types.
      //
      template<typename Self, typename Entity> class scene_entity_handle {
         public:
            using surface_renderer_passkey = cobb::passkey<surface_renderer, Self>;
            using value_type = Entity;

         protected:
            surface_renderer* owner = nullptr;
            size_t index = -1;

            Self& as_self() { return *(Self*)this; }

         public:
            scene_entity_handle() {}
            scene_entity_handle(surface_renderer& sr, size_t i) : owner(&sr), index(i) {}

            scene_entity_handle(Self&& o) {
               std::swap(owner, o.owner);
               std::swap(index, o.index);
            }
            scene_entity_handle& operator=(Self&& o) {
               std::swap(owner, o.owner);
               std::swap(index, o.index);
               return *this;
            }

            bool operator==(const Self& o) const noexcept {
               return owner == o.owner && index == o.index;
            }
            bool operator==(std::nullptr_t) const noexcept {
               return empty();
            }

            surface_renderer* renderer() const { return owner; }

            inline size_t list_index(surface_renderer_passkey) const noexcept { return this->index; }

            inline bool empty() const noexcept {
               return owner == nullptr || index == -1;
            }

            Entity* entity() {
               if (empty())
                  return nullptr;
               // 1. Cast to CRTP self-type to get access ot operator*
               // 2. Dereference self to invoke operator*, getting an Entity&
               // 3. Return address of the Entity&.
               return std::addressof(*as_self());
            };
      };
   }

   class rendered_bounds_handle;
   class rendered_bounds_handle : public impl::scene_entity_handle<rendered_bounds_handle, rendered_bounds> {
      public:
         using scene_entity_handle::scene_entity_handle;

         rendered_bounds& operator*();
         rendered_bounds* operator->();

         void destroy();
   };

   class rendered_landscape_handle;
   class rendered_landscape_handle : public impl::scene_entity_handle<rendered_landscape_handle, rendered_landscape> {
      public:
         using scene_entity_handle::scene_entity_handle;

         rendered_landscape& operator*();
         rendered_landscape* operator->();

         void destroy();
   };

   class rendered_light_handle;
   class rendered_light_handle : public impl::scene_entity_handle<rendered_light_handle, rendered_light> {
      public:
         using scene_entity_handle::scene_entity_handle;

         rendered_light& operator*();
         rendered_light* operator->();

         void destroy();
   };

   class rendered_mesh_handle;
   class rendered_mesh_handle : public impl::scene_entity_handle<rendered_mesh_handle, rendered_mesh> {
      public:
         using scene_entity_handle::scene_entity_handle;

         rendered_mesh& operator*();
         rendered_mesh* operator->();

         void destroy();
   };
}