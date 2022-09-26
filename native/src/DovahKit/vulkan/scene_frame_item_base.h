#pragma once
#include <concepts>
#include <type_traits>
#include "./helpers/frame_dirty_state.h"

namespace vulkanDK {
   struct scene_frame_item_base {
      public:
         enum class life_state_enum {
            // The scene frame item is "inactive" or "dead."
            empty,

            // The scene frame item should be used when rendering the scene.
            active,

            // The scene frame item is pending deletion. Its life state will change to 
            // "empty" and its owned GPU-side resources will be wholly deleted once it 
            // is no longer in use by any frame in flight.
            pending_delete,

            // The scene frame item was pending deletion, but has been recycled and 
            // made active. The owned GPU-side resources that were pending deletion are 
            // still pending deletion; when they are finally deleted, its life state 
            // will change to "active."
            active_recycle,
         };

      public:
         // Override these, for metaprogramming.

         static constexpr const bool gpu_resources_are_descriptors = false;

         static constexpr const bool gpu_resources_are_coalesced = false;

      public:
         life_state_enum   life_state = life_state_enum::empty;
         frame_dirty_state frame_in_flight_state;

         constexpr bool active() const noexcept {
            switch (life_state) {
               using enum life_state_enum;
               case active:
               case active_recycle:
                  return true;
            }
            return false;
         }
         constexpr bool empty() const noexcept { return this->life_state == life_state_enum::empty; }
         constexpr bool pending_delete() const noexcept { return this->life_state == life_state_enum::pending_delete; }
   };

   template<typename Data>
   struct scene_frame_item_owned_gpu_resources {
      using data_type = Data;

      data_type current;
      data_type outdated;
   };

   namespace scene_frame_item_traits {
      template<typename SFI> concept has_frame_drawing_data = requires(const SFI& item) {
         typename SFI::frame_drawing_data_type;
         { item.frame_drawing_data } -> std::same_as<const typename SFI::frame_drawing_data_type&>;
      };
      template<typename SFI> concept has_frame_culling_data = requires(const SFI& item) {
         typename SFI::frame_culling_data_type;
         { item.calculate_frame_culling_data() } -> std::same_as<typename SFI::frame_culling_data_type>;
      };

      template<typename SFI> concept owns_gpu_resources = requires (SFI & item) {
         { item.owned_gpu_resources };
         typename std::decay_t<decltype(item.owned_gpu_resources)>::data_type;
         std::is_same_v<
            std::decay_t<decltype(item.owned_gpu_resources)>,
            scene_frame_item_owned_gpu_resources<typename std::decay_t<decltype(item.owned_gpu_resources)>::data_type>
         >;
      };

      // True if this scene frame item type's owned GPU resources are the underlying 
      // data for descriptors in a descriptor array.
      template<typename SFI> concept owns_gpu_descriptors = requires {
         requires owns_gpu_resources<SFI>;
         requires (SFI::gpu_resources_are_descriptors == false);
      };
   }
}