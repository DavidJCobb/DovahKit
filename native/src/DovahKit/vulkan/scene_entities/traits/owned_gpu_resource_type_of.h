#pragma once
#include "../concepts/owns_gpu_resources.h"
#include "../owned_gpu_resource_sets.h"

namespace vulkanDK::scene_entities::traits {
   template<typename Entity> requires vulkanDK::scene_entities::concepts::owns_gpu_resources<Entity>
   using owned_gpu_resource_type_of = std::decay_t<decltype(Entity::owned_gpu_resource_sets)>::data_type;
}