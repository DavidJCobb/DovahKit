#pragma once
#include <QThread>

namespace vulkanDK {
   class loaded_texture;
   class surface_renderer;
}

namespace vulkanDK::asset_loading {
   class worker_thread_for_textures : public QThread {
      friend class surface_renderer;
      protected:
         surface_renderer& owner;
         size_t index;

      public:
         worker_thread_for_textures(surface_renderer& sr, size_t batch_index) : owner(sr), index(batch_index) {}

      protected:
         void _load_single_texture(loaded_texture&);

         virtual void run() override;
   };
}