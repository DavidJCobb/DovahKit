#pragma once
#include <concepts>
#include "./owns_gpu_resources.h"

namespace vulkanDK::scene_entities::concepts {
   //
   // If an entity's owned GPU-side resources  are made available to shaders by way of 
   // a descriptor array,  then some additional considerations are  needed. Updating a 
   // descriptor set  (including entries in a descriptor array)  requires re-recording 
   // of any command buffers that  used the descriptor set,  even if the updates would 
   // not affect any already-queued commands.
   //
   template<typename Entity> concept owns_gpu_descriptors = requires {
      requires owns_gpu_resources<Entity>;
      requires (Entity::owned_gpu_resources_are_descriptors == true); // static constexpr const bool member on the entity struct
   };
}