#include "reserve_meshes_for_nif.h"
#include "helpers/string/strieq_ascii.h"
#include "../rendered_nif.h"
#include "../surface_renderer.h"

#include "nif/blocks/BSTriShape.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiTriBasedGeom.h"
#include "nif/blocks/NiTriShape.h"
#include "nif/blocks/NiTriShapeData.h"

#include "./reserve_mesh_textures_given_ni_shader.h"

namespace vulkanDK::asset_loading {
   void reserve_meshes_for_nif(surface_renderer& sr, rendered_nif& nif) {
      if (!nif.root_node) {
         qDebug("[vulkanDK::asset_loading::reserve_meshes_for_nif] Model has no root node.");
         return;
      }

      auto available = sr.scene.entity_slots_available<rendered_mesh>();
      if (available == 0) {
         qDebug("[vulkanDK::asset_loading::reserve_meshes_for_nif] No mesh slots available for this NIF.");
         return;
      }

      // We're going to assume we have enough mesh slots. If it turns out we don't, then we'll just 
      // instantly free the meshes we've allocated. (We could check that in advance, but that would 
      // entail a lot of dynamic-casts that are going to be redundant with the ones in the loop we 
      // use below to actually create our meshes. In practice, this will be faster until we hit the 
      // mesh cap, and then we'll "hover" at the cap for a while, crossing back and forth over it, 
      // which will admittedly be slower.)
      bool cancel = false;

      using namespace nifDK::block_types;
      using mesh_interface_t = nifDK::block_interfaces::_DKVulkanMeshInterface;
      for (auto* block : nif.all_blocks) {
         mesh_interface_t* intfc  = nullptr;
         BSShaderProperty* shader = nullptr;
         if (auto* cast = dynamic_cast<NiTriBasedGeom*>(block)) {
            intfc  = cast;
            shader = cast->properties.shader;
         } else if (auto* cast = dynamic_cast<BSTriShape*>(block)) {
            intfc  = cast;
            shader = cast->properties.shader;
         }
         //
         // It's possible, but unlikely, that a NiTriShapeData or BSTriShape may have zero vertices 
         // or zero triangles, or that a NiGeometry may have no valid NiTriShapeData. Since these 
         // cases are unlikely, we'll just always reserve a mesh; we can free the mesh later if it 
         // turns out there's no valid data to give it.
         //
         if (intfc) {
            if (available == 0) {
               cancel = true;
               break;
            }
            --available;

            auto mesh_index = sr.scene.insert_new_scene_entity<rendered_mesh>();
            assert(mesh_index != scene::index_of_none);
            intfc->vulkan_state.mesh_handle = rendered_mesh_handle(sr, mesh_index);
            //
            auto& mesh = sr.scene.entities_of_type<rendered_mesh>()[mesh_index];
            mesh.lifetime.life_state = scene_entities::life_state::active_background_loading;
            mesh.owning_nif = &nif;
            //
            reserve_mesh_textures_given_ni_shader(sr, mesh, shader);
         }
      }

      if (cancel) {
         for (auto* block : nif.all_blocks) {
            mesh_interface_t* intfc  = nullptr;
            if (auto* cast = dynamic_cast<NiTriBasedGeom*>(block)) {
               intfc  = cast;
            } else if (auto* cast = dynamic_cast<BSTriShape*>(block)) {
               intfc  = cast;
            }
            if (!intfc)
               continue;

            auto& handle = intfc->vulkan_state.mesh_handle;
            if (handle.empty())
               break;
            //
            // TODO: extract the list index directly, and destroy the mesh directly. the code below 
            //       marks the mesh for delete, signals to all FiFs, etc., which is less efficient.
            // 
            //       we know with absolute certainty that at this point, we haven't loaded anything 
            //       for the mesh, and nothing has had time to react to it being created. we can 
            //       literally just reset it and mark it as empty directly.
            //
            intfc->vulkan_state.mesh_handle.destroy();
         }
      }
   }
}