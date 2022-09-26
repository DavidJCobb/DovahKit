#pragma once
#include <concepts>

namespace vulkanDK::scene_entities::concepts {
   //
   // "Frame culling data" is fixed-length data that will be made available to compute 
   // shaders running on the GPU,  in the form of an array  (one per frame in flight), 
   // to allow for culling via a compute shader.
   // 
   // In order to add frame culling data to an entity type, you should define a nested 
   // type with the name `frame_culling_data_type`,  along with a member function that 
   // has the following signature:
   // 
   //    frame_culling_data_type calculate_frame_culling_data() const;
   //
   template<typename Entity> concept has_frame_culling_data = requires(const Entity& item) {
      typename Entity::frame_culling_data_type;
      { item.calculate_frame_culling_data() } -> std::same_as<typename Entity::frame_culling_data_type>;
   };
}