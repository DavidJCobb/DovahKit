#pragma once

namespace vulkanDK {
   class rendered_light;
   class rendered_mesh;
   class surface_renderer;

   class rendered_light_handle {
      protected:
         surface_renderer* owner = nullptr;
         size_t index = -1;

      public:
         rendered_light_handle() {}
         rendered_light_handle(surface_renderer& sr, size_t i) : owner(&sr), index(i) {}

         rendered_light& operator*();
         rendered_light* operator->();

         bool operator==(const rendered_light_handle&) const noexcept = default;
         bool operator==(std::nullptr_t) const noexcept {
            return empty();
         }

         surface_renderer* renderer() const { return owner; }

         inline bool empty() const noexcept {
            return owner == nullptr || index == -1;
         }

         void destroy();
   };

   class rendered_mesh_handle {
      protected:
         surface_renderer* owner = nullptr;
         size_t index = -1;

      public:
         rendered_mesh_handle() {}
         rendered_mesh_handle(surface_renderer& sr, size_t i) : owner(&sr), index(i) {}

         rendered_mesh& operator*();
         rendered_mesh* operator->();

         bool operator==(const rendered_mesh_handle&) const noexcept = default;
         bool operator==(std::nullptr_t) const noexcept {
            return empty();
         }

         surface_renderer* renderer() const { return owner; }

         inline bool empty() const noexcept {
            return owner == nullptr || index == -1;
         }

         void destroy();
   };
}