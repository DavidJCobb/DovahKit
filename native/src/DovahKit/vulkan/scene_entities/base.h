#pragma once
#include "./coalesced_vib_settings.h"
#include "./fif_sync_state.h"
#include "./life_state.h"

namespace vulkanDK {
   class scene;
}

namespace vulkanDK::scene_entities {
   // Base class for all scene entities.
   struct base {
      public:
         //
         // Metaprogramming configuration, to be overridden on subclasses. See concepts 
         // for further information.

         // Name values, for use in debug log messages.
         static constexpr const char* name_single = "<unnamed>";
         static constexpr const char* name_plural = "<unnamed>";
         
         static constexpr const auto coalesced_vib_settings = scene_entities::coalesced_vib_settings{};
         using coalesced_vertex_type = void;
         using coalesced_index_type  = void;

         static constexpr const bool owned_gpu_resources_are_descriptors = false;

         // Is this entity rendered using a draw call? Affects how frame drawing data is updated for 
         // the entity, whether command buffers are marked as outdated if the entity is created or 
         // destroyed, and so on.
         static constexpr const bool is_drawn = false;

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
            struct {
               fif_sync_state sync_state;
            } coalescing;
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

         #pragma region Member functions for coalesced resources
         static void coalesce_constant_shared_indices_into(void* write_to) = delete;
         void coalesce_indices_into(void* write_to) const noexcept = delete;
         void coalesce_vertices_into(void* write_to) const noexcept = delete;
         #pragma endregion

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