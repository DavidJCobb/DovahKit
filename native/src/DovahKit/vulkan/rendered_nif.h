#pragma once
#include <atomic>
#include <cstdint>
#include <type_traits>
#include "nif/file.h"

namespace vulkanDK {
   class surface_renderer;
   namespace asset_loading {
      class worker_thread_for_meshes;
      class worker_thread_for_nifs;
   }

   class rendered_nif : public nifDK::file {
      friend class surface_renderer;
      friend class asset_loading::worker_thread_for_meshes;
      friend class asset_loading::worker_thread_for_nifs;
      public:
         struct deleter {
            void operator()(rendered_nif* n) { n->delete_when_able(); }
         };
      protected:
         struct loading_flag {
            enum type : uint32_t {
               load_queued       = 0x00000001, // at minimum, loading has been queued; we may be actively loading but between load stages
               loading           = 0x00000002, // being used on a worker thread to load data
               generating_meshes = 0x00000004, // being used on a worker thread to prep a `rendered_mesh`
               marked_for_delete = 0x08000000, // deletion requested; carry it out when background use ends
               load_canceled     = 0x10000000, // load operation finished (cancellation request honored)
               load_success      = 0x20000000, // load operation finished (success)
               load_failure      = 0x40000000, // load operation finished (failure)
               cancel_requested  = 0x80000000, // cancel requested (may not have been honored yet; may never be honored at all)

               is_in_background_use = load_queued | loading | generating_meshes,

               load_ended = load_canceled | load_success | load_failure,
            };
         };
         using loading_flags = std::underlying_type_t<loading_flag::type>;

         struct {
            std::atomic<loading_flags> flags = 0;
            surface_renderer* manager = nullptr; // set by main thread before worker threads run; cleared by main thread after worker threads finish
         } multi_thread_state;

         // Runs `delete this` if the NIF is marked for delete.
         void _on_background_use_ended();

         using nifDK::file::read;

      public:
      
         inline bool is_background_loading() const noexcept {
            return this->multi_thread_state.flags & loading_flag::loading;
         }
         inline bool is_in_background_use() const noexcept {
            return this->multi_thread_state.flags & loading_flag::is_in_background_use;
         }

         inline bool is_marked_for_delete() const noexcept {
            return this->multi_thread_state.flags & loading_flag::marked_for_delete;
         }

         inline bool did_load_end() const noexcept {
            return this->multi_thread_state.flags & loading_flag::load_ended;
         }
         inline bool did_load_succeed() const noexcept {
            return this->multi_thread_state.flags & loading_flag::load_success;
         }
         inline bool did_load_fail() const noexcept {
            return this->multi_thread_state.flags & loading_flag::load_failure;
         }
         inline bool did_load_cancel() const noexcept {
            return this->multi_thread_state.flags & loading_flag::load_canceled;
         }

         inline bool is_cancel_requested() const noexcept {
            return this->multi_thread_state.flags & loading_flag::cancel_requested;
         }
         void request_cancel_load() {
            this->multi_thread_state.flags |= loading_flag::cancel_requested;
         }

         void delete_when_able();
   };
}