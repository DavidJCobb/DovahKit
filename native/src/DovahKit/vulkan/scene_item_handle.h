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
      template<typename Self> class scene_item_handle {
         public:
            using surface_renderer_passkey = cobb::passkey<surface_renderer, Self>;

         protected:
            surface_renderer* owner = nullptr;
            size_t index = -1;

         public:
            scene_item_handle() {}
            scene_item_handle(surface_renderer& sr, size_t i) : owner(&sr), index(i) {}

            scene_item_handle(Self&& o) {
               std::swap(owner, o.owner);
               std::swap(index, o.index);
            }
            scene_item_handle& operator=(Self&& o) {
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
      };
   }

   class rendered_bounds_handle;
   class rendered_bounds_handle : public impl::scene_item_handle<rendered_bounds_handle> {
      public:
         using scene_item_handle::scene_item_handle;

         rendered_bounds& operator*();
         rendered_bounds* operator->();

         void destroy();
   };

   class rendered_landscape_handle;
   class rendered_landscape_handle : public impl::scene_item_handle<rendered_landscape_handle> {
      public:
         using scene_item_handle::scene_item_handle;

         rendered_landscape& operator*();
         rendered_landscape* operator->();

         void destroy();
   };

   class rendered_light_handle;
   class rendered_light_handle : public impl::scene_item_handle<rendered_light_handle> {
      public:
         using scene_item_handle::scene_item_handle;

         rendered_light& operator*();
         rendered_light* operator->();

         void destroy();
   };

   class rendered_mesh_handle;
   class rendered_mesh_handle : public impl::scene_item_handle<rendered_mesh_handle> {
      public:
         using scene_item_handle::scene_item_handle;

         rendered_mesh& operator*();
         rendered_mesh* operator->();

         void destroy();
   };
}