#pragma once
#include <atomic>
#include <QObject>
#include "nif/file.h"

namespace vulkanDK {
   class surface_renderer;

   class rendered_nif : public QObject, public nifDK::file {
      friend class surface_renderer;
      protected:
         struct {
            std::atomic<bool> loaded   = false;
            std::atomic<bool> canceled = false;
            surface_renderer* manager  = nullptr;
         } multi_thread_state;

         using nifDK::file::read;

      public:

         inline bool is_loaded() const noexcept { return this->multi_thread_state.loaded; }

         // Cancels multi-threaded loading, if the asset is still loading.
         void cancel_load() {
            this->multi_thread_state.canceled = true;
         }

      signals:
         void loadCanceled();
         void loadFinished(bool success);
   };
}