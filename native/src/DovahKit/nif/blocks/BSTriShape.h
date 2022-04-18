#pragma once
#include "NiAVObject.h"
#include "../types/BSVertexDataSSE.h"
#include "../types/BSVertexDesc.h"
#include "../types/NiBound.h"
#include "../types/Triangle.h"
#include "vulkan/scene_item_handle.h"

namespace nifDK::block_types {
   class BSDismemberSkinInstance;
   class BSShaderProperty;
   class NiAlphaProperty;

   class BSTriShape : public NiAVObject {
      public:
         static constexpr const char* const type_name = "BSTriShape";
      public:
         struct material {
            std::string name;
            int32_t extra = -1;
         };
      public:
         NiBound bounds;
         BSDismemberSkinInstance* skin = nullptr;
         struct {
            BSShaderProperty* shader = nullptr;
            NiAlphaProperty* alpha = nullptr;
         } properties;
         BSVertexDesc vertex_desc;
         std::vector<Triangle> triangles;
         std::vector<BSVertexDataSSE> vertices;
         struct {
            std::vector<glm::fvec3> per_vertex;
            std::vector<Triangle> triangles; // copy of mesh triangle list
         } particle_data; // user version 2 == 100

         struct {
            //
            // Fields for supporting DovahKit's run-time needs.
            //
            vulkanDK::rendered_mesh_handle mesh_handle;
         } vulkan_state;

         virtual void parse(file_reader&) override;
   };
}