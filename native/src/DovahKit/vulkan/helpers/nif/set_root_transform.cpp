#include "./set_root_transform.h"
#include "./../../../helpers/dynamic_fast_cast.h"
#include "./../../../helpers/string/strieq_ascii.h"

#include "../../rendered_mesh.h"
#include "../../rendered_nif.h"

#include "nif/blocks/_DKVulkanInterface.h"
#include "nif/blocks/BSTriShape.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiTriBasedGeom.h"

namespace vulkanDK::helpers::nif {
   extern void set_root_transform(rendered_nif& nif, const glm::mat4& transform) {
      if (!nif.root_node)
         return;

      using namespace nifDK::block_types;

      struct _state {
         glm::mat4 transform;
      };
      _state state;
      state.transform = transform;

      //
      // NOTE: `vulkanDK::asset_loading::worker_thread_for_meshes::_load_single_nif` is the 
      // function that fully configures the `rendered_mesh` data for a NIF that's being loaded, 
      // including the transforms for each mesh. The logic for handling transforms should be 
      // kept roughly consistent between that function and this one, though this function will 
      // be much simpler since it only needs to handle transforms.
      //
      nif.root_node->walk_tree(
         state,
         [](NiNode* node, _state& state) {
            if (node->parent) {
               state.transform = state.transform * node->transform.to_matrix();
            }
         },
         [](NiAVObject* object, const _state& state) {
            rendered_mesh* mesh = nullptr;
            glm::mat4 local_transform;

            if (auto* intfc = dynamic_cast<nifDK::block_interfaces::_DKVulkanMeshInterface*>(object)) {
               auto& handle = intfc->vulkan_state.mesh_handle;
               if (!handle.empty()) {
                  mesh = handle.entity();
               }
               if (!mesh)
                  return;

               if (auto* geom = dynamic_cast<NiTriBasedGeom*>(object)) {
                  local_transform = geom->transform.to_matrix();
               } else if (auto* geom = dynamic_cast<BSTriShape*>(object)) {
                  local_transform = geom->transform.to_matrix();
               } else {
                  return;
               }

               mesh->set_transform(state.transform * local_transform);
            }
         }
      );
   }
}