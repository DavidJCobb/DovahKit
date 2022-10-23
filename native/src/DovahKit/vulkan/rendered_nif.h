#pragma once
#include <atomic>
#include <QObject>
#include "nif/file.h"

namespace vulkanDK {
   class surface_renderer;
   namespace asset_loading {
      class worker_thread_for_nifs;
   }

   class rendered_nif : public QObject, public nifDK::file {
      friend class surface_renderer;
      friend class asset_loading::worker_thread_for_nifs;
      protected:
         struct {
            std::atomic<bool> loaded   = false;
            std::atomic<bool> canceled = false;
            surface_renderer* manager  = nullptr; // set by main thread before worker threads run; cleared by main thread after worker threads finish
         } multi_thread_state;

         using nifDK::file::read;

      public:

         // Checks if the asset is being loaded in the background. The `manager` variable will 
         // have been set before any off-thread work began, and will be cleared only after the 
         // off-thread work has ended.
         inline bool is_background_loading() const noexcept { return this->multi_thread_state.manager != nullptr; }

         // See comments on `cancel_load` for caveats.
         inline bool is_background_load_canceled() const noexcept { return this->multi_thread_state.canceled; }

         // Cancels multi-threaded loading, if the asset is still loading in the background. 
         // Note that you must wait until the load-related code has actually reacted to the 
         // cancellation before you can safely assume the NIF will be left alone (i.e. wait 
         // for the appropriate signal).
         void cancel_load() {
            this->multi_thread_state.canceled = true;
         }

      signals:
         void loadCanceled();
         void loadFinished(bool success);
   };
}