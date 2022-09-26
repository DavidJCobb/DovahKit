#pragma once
#include "./fif_sync_state.h"
#include "./life_state.h"

namespace vulkanDK::scene_entities {
   // Base class for all scene entities.
   struct base {
      public:
         //
         // Metaprogramming configuration, to be overridden on subclasses. See concepts 
         // for further information.
         //
         static constexpr const char* name_single = "<unnamed>";
         static constexpr const char* name_plural = "<unnamed>";
         //
         static constexpr const bool owned_gpu_resources_are_coalesced   = false;
         static constexpr const bool owned_gpu_resources_are_descriptors = false;
         static constexpr const bool is_drawn = false; // is this entity rendered using a draw call?
         using frame_drawing_data_type = void;
         using frame_culling_data_type = void;

      public:
         base() {}

         base(const base&) noexcept = default;
         base(base&&) noexcept;
         base& operator=(const base&) noexcept = default;
         base& operator=(base&&) noexcept;

      public:
         struct {
            life_state     life_state = life_state::empty;
            fif_sync_state sync_state;
         } lifetime;

         constexpr bool active() const noexcept {
            switch (lifetime.life_state) {
               using enum life_state;
               case active:
               case active_recycle:
                  return true;
            }
            return false;
         }
         constexpr bool empty() const noexcept { return this->lifetime.life_state == life_state::empty; }
         constexpr bool pending_delete() const noexcept { return this->lifetime.life_state == life_state::pending_delete; }
         constexpr bool recycle_in_progress() const noexcept { return this->lifetime.life_state == life_state::active_recycle; }

         inline void on_frame_drawing_data_changed() {
            this->lifetime.sync_state.set_all_out_of_date();
         }

      protected:
         template<typename Subclass> void _mark_for_delete();
         template<typename Subclass> void _reset();

      public:
         // You must override these on your subclass, and call the CRTP-esque helpers, 
         // to ensure proper management of owned GPU-side resources and so on.
         void mark_for_delete() { _mark_for_delete<base>(); }
         void reset() { _reset<base>(); }
   };
}

#include "base.inl"