#include "world_axes.h"
#include <algorithm>
#include <QPainter>
#include <QResource>
#include "../frame_in_flight.h"
#include "../surface_renderer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace {
}

namespace vulkanDK::overlays {
   world_axes::world_axes() {
   }
   void world_axes::set_owner(surface_renderer& o) {
      this->owner = &o;
   }

   /*static*/ void world_axes::_update_shader_render_area(graphics_shader& shader, VkExtent2D surface_extent) {
      auto& area = shader.options.area;
      area.viewport = {
         .x        = 0,
         .y        = (std::max)(0.0F, (float)surface_extent.height - viewport_h),
         .width    = viewport_w,
         .height   = viewport_h,
         .minDepth = 0.0, // must be >= 0
         .maxDepth = 1.0, // must be <= 1
      };
      area.scissor = {
         .offset = { 0, (int32_t)area.viewport.y },
         .extent = { viewport_w, viewport_h },
      };
   }
   /*static*/ void world_axes::setup_shaders(surface_renderer& sr) {
      auto* s = sr.create_graphics_shader(shader_id);
      s->set_render_pass(sr.render_passes_by_name.ui);
      s->set_layout_info(
         {  // Descriptor set layouts
            sr.descriptor_set_layouts.overlay_world_axes.handle,
         }
      );
      //
      auto& options = s->options;
      //
      shader_module* frag = nullptr;
      shader_module* vert = nullptr;
      {
         frag = new shader_module(sr.logical_device, QResource("shaders/overlays/world_axes/main.frag.spv").uncompressedData());
         vert = new shader_module(sr.logical_device, QResource("shaders/overlays/world_axes/main.vert.spv").uncompressedData());
         if (frag->empty()) {
            throw std::runtime_error("[vulkanDK::overlays::world_axes::setup_shaders] Failed to load fragment shader.");
         }
         if (vert->empty()) {
            throw std::runtime_error("[vulkanDK::overlays::world_axes::setup_shaders] Failed to load vertex shader.");
         }
         sr.shader_modules.push_back(frag);
         sr.shader_modules.push_back(vert);
      }
      options.stages = {
         {
            .module           = frag,
            .entry_point_name = "main",
            .stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .module           = vert,
            .entry_point_name = "main",
            .stage            = VK_SHADER_STAGE_VERTEX_BIT,
         },
      };
      options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend);
      {
         auto& vertex     = options.inputs.vertex;
         auto  attributes = _vertex::get_attribute_descriptions();
         vertex.bindings.push_back(_vertex::get_binding_description());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      options.dynamic_states = {
         VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH,
      };
      options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      options.rasterization.cullMode    = VK_CULL_MODE_NONE;
      if (sr.device_info->support.non_solid_polygon_fill_modes) {
         options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
      }
      options.area.mode = graphics_shader::area_mode::custom;
      s->on_resize = &_update_shader_render_area;
      s->setup_pipeline_layout();
   }
   void world_axes::initialize_descriptor_sets(frame_in_flight& fif) {
      auto buffer_info = VkDescriptorBufferInfo{
         .buffer = this->shader_params.uniform.handle,
         .offset = 0,
         .range  = sizeof(_shader_state), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
      };
      //
      auto descriptor_writes = std::array{
         VkWriteDescriptorSet{ // uniform buffer object
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet           = fif.descriptor_sets.overlay_world_axes,
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
      {
         constexpr float halfwidth = axis_arrow_length * 1.5;
         state.proj = glm::ortho<float>(-halfwidth, halfwidth, halfwidth, -halfwidth, axis_arrow_length * -2, axis_arrow_length * 8);
      }
      this->shader_params.uniform.unmap_memory(&state);
   }
   void world_axes::create_geometry() {
      auto& sr  = *this->owner;
      auto& vib = this->vertex_and_index_buffer;
      //
      vib = sr.create_buffer(_vib_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      this->update_geometry();
   }
   void world_axes::update_geometry() {
      auto& sr  = *this->owner;
      auto& vib = this->vertex_and_index_buffer;
      //
      auto  staging = sr.create_buffer(_vib_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data    = staging.map_memory();
      memset(data, 0, _vib_size);
      //
      {
         static_assert(vertices_per_axis == 2,      "Model design changed; rewrite this code.");
         static_assert(index_count == vertex_count, "Model design changed; rewrite this code.");
         auto* vertices = (_vertex*)data;
         auto* indices  = (uint16_t*)((std::intptr_t)data + _vib_indices_offset);
         //
         constexpr auto model = std::array{
            // x:
            _vertex{
               .pos    = { 0, 0, 0 },
               .color  = { 1, 0, 0 },
               .radius = 0,
            },
            _vertex{
               .pos    = { axis_arrow_length, 0, 0 },
               .color  = { 1, 0, 0 },
               .radius = 4,
            },
            // y:
            _vertex{
               .pos    = { 0, 0, 0 },
               .color  = { 0, 1, 0 },
               .radius = 0,
            },
            _vertex{
               .pos    = { 0, axis_arrow_length, 0 },
               .color  = { 0, 1, 0 },
               .radius = 4,
            },
            // z:
            _vertex{
               .pos    = { 0, 0, 0 },
               .color  = { 0.1, 0.4, 1.0 },
               .radius = 0,
            },
            _vertex{
               .pos    = { 0, 0, axis_arrow_length },
               .color  = { 0.1, 0.4, 1.0 },
               .radius = 4,
            },
         };
         static_assert(model.size() == vertex_count);
         for (size_t i = 0; i < vertex_count; ++i) {
            vertices[i] = model[i];
            indices[i]  = i;
         }

         {
            auto& oc = this->style.override_colors;
            if (oc.enabled) {
               vertices[0].color = oc.x;
               vertices[1].color = oc.x;

               vertices[2].color = oc.y;
               vertices[3].color = oc.y;

               vertices[4].color = oc.z;
               vertices[5].color = oc.z;
               if (oc.x == glm::vec3{1, 0, 0} && oc.y == glm::vec3{0, 1, 0} && oc.z == glm::vec3{0, 0, 1}) {
                  //
                  // HACK: If this is just the standard gizmo color scheme, use a slightly lighter 
                  //       shade of blue. This is purely so that if I'm testing with no loaded cell, 
                  //       at night, with a bluelight filter, I don't think the Z-axis is missing.
                  //
                  vertices[4].color = { 0.1, 0.4, 1.0 };
                  vertices[5].color = { 0.1, 0.4, 1.0 };
               }
            }
         }
      }
      staging.unmap_memory(data);
      //
      vib.copy_from(staging);
   }

   void world_axes::handle_resize(surface_renderer& sr) {
      auto& sre = this->owner->surface_extent;
      this->active = (sre.width >= viewport_w) && (sre.height >= viewport_h);
   }

   void world_axes::set_colors(glm::vec3 x, glm::vec3 y, glm::vec3 z) {
      auto& oc = this->style.override_colors;
      oc.enabled = true;
      oc.x = x;
      oc.y = y;
      oc.z = z;
   }
   void world_axes::clear_colors() {
      auto& oc = this->style.override_colors;
      oc.enabled = false;
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

   void world_axes::prepare_for_render() {
      auto& state  = *(_shader_state*)this->shader_params.uniform.map_memory();
      auto& scene  = this->owner->scene;
      auto& camera = scene.camera;
      state.view = glm::inverse(glm::translate(
         glm::eulerAngleZYX(-camera.yaw, -camera.roll, -camera.pitch),
         glm::vec3{ 0, 0, axis_arrow_length * 4 }
      ));
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