#pragma once
#include <concepts>

namespace vulkanDK::scene_entities::concepts {
   //
   // "Frame drawing data" is fixed-length data that will be made available to shaders 
   // running on the GPU, in the form of an array (one per frame in flight). This data 
   // can be updated on the GPU instantly before a frame draw without the need for any 
   // additional synchronization; after making your CPU-side changes, you just have to 
   // flag the entity as "out of date" for all frames in flight.
   // 
   // In order to add frame culling data to an entity type, you should define a nested 
   // type with the name  `frame_drawing_data_type`,  along with a member of that type 
   // named `frame_drawing_data`.
   // 
   template<typename Entity> concept has_frame_drawing_data = requires(const Entity& item) {
      typename Entity::frame_drawing_data_type;
      { item.frame_drawing_data } -> std::same_as<const typename Entity::frame_drawing_data_type&>;
   };
}