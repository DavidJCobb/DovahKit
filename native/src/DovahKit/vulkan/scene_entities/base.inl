#pragma once
#include <cassert>
#include <concepts>
#include <type_traits>
#include "./base.h"
#include "./concepts/has_frame_drawing_data.h"
#include "./concepts/owns_gpu_resources.h"

namespace vulkanDK::scene_entities {
   template<typename Subclass>
   void base::_mark_for_delete() {
      auto* self = (Subclass*)this;
      //
      this->lifetime.life_state = life_state::pending_delete;
      this->lifetime.sync_state.set_all_out_of_date();
      if constexpr (concepts::owns_gpu_resources<Subclass>) {
         if (!self->owned_gpu_resources.has_outdated())
            std::swap(self->owned_gpu_resources.current, self->owned_gpu_resources.outdated);
      }
   }

   template<typename Subclass>
   void base::_reset() {
      auto* self = (Subclass*)this;
      //
      if constexpr (concepts::owns_gpu_resources<Subclass>) {
         assert(!this->pending_delete() || this->lifetime.sync_state.are_all_up_to_date());
         self->owned_gpu_resources = {};
      }
      if constexpr (concepts::has_frame_drawing_data<Subclass>) {
         self->frame_drawing_data = {};
      }
      this->lifetime = {};
   }
}
