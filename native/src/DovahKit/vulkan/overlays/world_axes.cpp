#include "world_axes.h"
#include <QPainter>
#include <QResource>
#include "../surface_renderer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace vulkanDK::overlays {
   world_axes::world_axes() {
   }
   void world_axes::set_owner(surface_renderer& o) {
      this->owner = &o;
   }

   /*static*/ void world_axes::_update_shader_render_area(shader::area_override_data& aod, VkExtent2D extent) {
      aod.viewport = {
         .x        = 0,
         .y        = 0,
         .width    = viewport_w,
         .height   = viewport_h,
         .minDepth = 0.0, // must be >= 0
         .maxDepth = 1.0, // must be <= 1
      };
      aod.scissor = {
         .offset = { 0, 0 },
         .extent = { viewport_w, viewport_h },
      };
   }
   /*static*/ void world_axes::setup_shaders(surface_renderer& sr) {
      auto* s = sr.get_or_create_shader(shader_id);
      s->set_render_pass(sr.render_passes_by_name.ui);
      s->set_layout_info(
         {  // Descriptor set layouts
            sr.descriptor_set_layouts[descriptor_set_index].handle,
         }
      );
      //
      auto& dfn = s->definition;
      //
      shader_module* frag = nullptr;
      shader_module* vert = nullptr;
      {
         frag = new shader_module(sr.logical_device, QResource("shaders/overlay-world-axes.frag.spv").uncompressedData());
         vert = new shader_module(sr.logical_device, QResource("shaders/overlay-world-axes.vert.spv").uncompressedData());
         if (frag->empty()) {
            throw std::runtime_error("[vulkanDK::overlays::world_axes::setup_shaders] Failed to load fragment shader.");
         }
         if (vert->empty()) {
            throw std::runtime_error("[vulkanDK::overlays::world_axes::setup_shaders] Failed to load vertex shader.");
         }
         sr.shader_modules.push_back(frag);
         sr.shader_modules.push_back(vert);
      }
      dfn.stages = {
         {
            .module              = frag,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
            .specialization_info = nullptr,
         },
         {
            .module              = vert,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_VERTEX_BIT,
            .specialization_info = nullptr,
         },
      };
      dfn.color_blending.blends.emplace_back(material_definition::default_alpha_blend);
      {
         auto& vertex     = dfn.inputs.vertex;
         auto  attributes = _vertex::get_attribute_descriptions();
         vertex.bindings.push_back(_vertex::get_binding_description());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      dfn.dynamic_states = {
         VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH,
      };
      dfn.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      {
         s->set_area_override_info({ .handler = &_update_shader_render_area });
      }
      s->setup_pipeline_layout(sr);
   }
   void world_axes::initialize_descriptor_sets(swap_chain_image& sci) {
      auto buffer_info = VkDescriptorBufferInfo{
         .buffer = this->shader_params.uniform.handle,
         .offset = 0,
         .range  = sizeof(_shader_state), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
      };
      //
      auto descriptor_writes = std::array{
         VkWriteDescriptorSet{ // uniform buffer object
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet           = sci.descriptor_sets[descriptor_set_index],
            .dstBinding       = 0, // this should match the binding value in the shader
            .dstArrayElement  = 0, // index of the first descriptor in the raray to update
            .descriptorCount  = 1, // you can update multiple descriptors at once if they're in an array
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &buffer_info,
            .pTexelBufferView = nullptr,
         },
      };
      vkUpdateDescriptorSets(this->owner->logical_device, (uint32_t)descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
   }
   void world_axes::setup_shader_parameter_buffers() {
      constexpr VkDeviceSize buffer_size = sizeof(_shader_state);
      this->shader_params.uniform = this->owner->create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      //
      {
         auto& sre = this->owner->surface_extent;
         this->active = (sre.width >= viewport_w) && (sre.height >= viewport_h);
      }
      //
      // Only need to update these when the viewport size changes, which is always accompanied by the swap 
      // chain images being set up; they set us up; ergo we only need to set this initially, during setup.
      //
      auto& state = *(_shader_state*) this->shader_params.uniform.map_memory();
      state.view = glm::mat4(1);
      state.proj = glm::ortho<float>(0, viewport_w, viewport_h, 0, 0, axis_arrow_length * 2.5);
      this->shader_params.uniform.unmap_memory(&state);
   }
   void world_axes::create_geometry() {
      auto& sr  = *this->owner;
      auto& vib = this->vertex_and_index_buffer;
      //
      constexpr VkDeviceSize buffer_size_v = sizeof(_vertex)  * vertex_count;
      constexpr VkDeviceSize buffer_size_i = sizeof(uint16_t) * index_count;
      constexpr VkDeviceSize buffer_size   = buffer_size_v + buffer_size_i;
      //
      auto  staging = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data    = staging.map_memory();
      memset(data, 0, buffer_size);
      //
      {
         static_assert(vertices_per_axis == 2,      "Model design changed; rewrite this code.");
         static_assert(index_count == vertex_count, "Model design changed; rewrite this code.");
         auto* vertices = (_vertex*)data;
         auto* indices  = (uint16_t*)((std::intptr_t)data + buffer_size_v);
         //
         for (size_t i = 0; i < axis_count; ++i) {
            auto& start = vertices[(i * axis_count) + 0];
            auto& end   = vertices[(i * axis_count) + 1];
            //
            start.pos  = { 0, 0, 0 };
            end.pos    = { 0, 0, 0 };
            end.pos[i] = axis_arrow_length;
            //
            start.color    = { 0, 0, 0 };
            start.color[i] = 1;
            end.color = start.color;
            //
            start.radius = 0;
            end.radius   = 1;
            //
            indices[(i * axis_count) + 0] = (i * axis_count) + 0;
            indices[(i * axis_count) + 1] = (i * axis_count) + 1;
         }
      }
      staging.unmap_memory(data);
      //
      vib = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      vib.copy_from(staging);
   }

   bool world_axes::needs_redraw() const {
      auto& scene  = this->owner->scene;
      auto& camera = scene.camera;
      auto& prior  = this->state.last_camera_rotation;
      if (camera.pitch != prior.x)
         return true;
      if (camera.roll != prior.y)
         return true;
      if (camera.yaw != prior.z)
         return true;
      return false;
   }

   void world_axes::commands_pre_pass(VkCommandBuffer command_buffer) {
      auto& state = *(_shader_state*)this->shader_params.uniform.map_memory();
      auto& scene = this->owner->scene;
      //state.view = glm::mat3(scene.global_state.view);
      auto& camera = scene.camera;
      state.view = glm::inverse(
         glm::translate(
            glm::eulerAngleZ(camera.yaw) * glm::eulerAngleY(camera.roll) * glm::eulerAngleX(camera.pitch),
            glm::vec3{ 0, 0, -4 }
         )
      );
      this->shader_params.uniform.unmap_memory(&state);
      this->state.last_camera_rotation = { camera.pitch, camera.roll, camera.yaw };
   }
   void world_axes::draw_call(VkCommandBuffer command_buffer) {
      if (!this->active)
         return;
      //
      auto& vib = this->vertex_and_index_buffer;
      //
      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(command_buffer, 0, 1, &vib.handle, &offset);
      vkCmdBindIndexBuffer  (command_buffer, vib.handle, _vib_indices_offset, VK_INDEX_TYPE_UINT16);
      {
         float width = 1.0;
         if (const auto* di = this->owner->device_info) {
            auto& wl = di->support.wide_lines;
            if (wl.available)
               width = std::clamp(this->style.thickness, wl.width_range.minimum, wl.width_range.maximum);
         }
         vkCmdSetLineWidth(command_buffer, width);
      }
      vkCmdDrawIndexed(command_buffer, (uint32_t)index_count, 1, 0, 0, 0);
   }
}