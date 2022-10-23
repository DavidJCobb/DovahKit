#pragma once
#include <QThread>
#include "./queued_nif_load.h"

namespace vulkanDK {
   class surface_renderer;
}

namespace vulkanDK::asset_loading {
   class worker_thread_for_meshes : public QThread {
      friend class surface_renderer;
      protected:
         surface_renderer& owner;
         const size_t index;

      public:
         worker_thread_for_meshes(surface_renderer& sr, size_t batch_index) : owner(sr), index(batch_index) {}

      protected:
         void _load_single_nif(const queued_nif_load&);

         virtual void run() override;
   };
}