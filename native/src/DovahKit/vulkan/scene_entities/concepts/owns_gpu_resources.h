#pragma once
#include <concepts>
#include "../owned_gpu_resource_sets.h"

namespace vulkanDK::scene_entities::concepts {
   //
   // "Owned GPU resources" are GPU-side resources  that belong exclusively to a scene 
   // entity.  Examples include `buffer` and `owned_image_and_view` instances  present 
   // inside of the entity struct.
   // 
   // To attach  owned GPU resources to an  entity type, create a member  on that type 
   // named `owned_gpu_resources`. The member's type should be an instantiation of the 
   // `owned_gpu_resource_sets` template. This will allow the entity to store two sets 
   // of resources:  "current" resources and  "outdated" resources.  This in turn will 
   // make it possible to "recycle" a pending-deletion entity.
   // 
   // If the resources need to be made available to shaders as entries in a descriptor 
   // array, then refer to "owns_gpu_resources.h" for further requirements.
   //
   template<typename Entity> concept owns_gpu_resources = requires (Entity& item) {
      { item.owned_gpu_resources };
      typename std::decay_t<decltype(item.owned_gpu_resources)>::data_type;
      requires std::is_same_v<
         std::decay_t<decltype(item.owned_gpu_resources)>,
         owned_gpu_resource_sets<typename std::decay_t<decltype(item.owned_gpu_resources)>::data_type>
      >;
   };
}