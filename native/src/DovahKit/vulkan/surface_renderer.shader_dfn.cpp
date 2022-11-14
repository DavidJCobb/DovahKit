#include "surface_renderer.h"
#include <array>
#include "./config/grid.h"
#include "./config/shadow_maps.h"
#include "./config/use_inverted_depth.h"
#include "./helpers/cubemap_helpers.h"

namespace vulkanDK {
   void surface_renderer::_setup_scene_background_shader() {
      assert(this->render_passes_by_name.main != nullptr);
      //
      auto* s = this->create_graphics_shader(scene_background_shader_id);
      s->set_render_pass(this->render_passes_by_name.main);
      s->set_layout_info({ this->descriptor_set_layouts.scene_state.handle });
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/util/full-screen-triangle.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/util/scene-fog-color-far.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (Scene Background: util/full-screen-triangle.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (Scene Background: util/scene-fog-color-far.frag.spv)");
      }
      //
      options.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE; // the vertex shader produces a clockwise triangle
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
      options.color_blending.blends.emplace_back(graphics_shader::color_blend{});
      options.depth.testing = false;
      options.depth.writing = false;
      options.depth.comparison = VK_COMPARE_OP_ALWAYS;
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }
   void surface_renderer::_setup_oit_composite_shader() {
      if (!this->can_do_alpha()) {
         //
         // If the device doesn't support the features we need for OIT, then we don't even 
         // define the render pass, so we also shouldn't load any shaders that rely on it.
         //
         return;
      }
      assert(this->render_passes_by_name.main_oit != nullptr);
      //
      auto* s = this->create_graphics_shader(oit_composite_shader_id);
      s->set_render_pass(this->render_passes_by_name.main_oit, 1);
      s->set_layout_info({ this->descriptor_set_layouts.oit_compositing.handle });
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/util/full-screen-triangle.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/util/oit-composite.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (OIT Composite: util/full-screen-triangle.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (OIT Composite: util/oit-composite.frag.spv)");
      }
      //
      options.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE; // the vertex shader produces a clockwise triangle
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
      options.color_blending.blends.emplace_back(graphics_shader::color_blend{
         .source = {
            .color = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
         },
         .destination = {
            .color = VK_BLEND_FACTOR_SRC_ALPHA,
            .alpha = VK_BLEND_FACTOR_SRC_ALPHA,
         },
      });
      options.depth.testing    = true;
      options.depth.writing    = false;
      options.depth.comparison = VK_COMPARE_OP_ALWAYS;
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }

   #pragma region rendered_mesh shaders
   void surface_renderer::_setup_rendered_mesh_shaders() {
      this->_setup_rendered_mesh_color_shader();
      this->_setup_rendered_mesh_wboit_shader();
      this->_setup_rendered_mesh_shadows_caster_shaders();
      this->_setup_rendered_mesh_shadows_sun_shader();
   }
      void surface_renderer::_setup_rendered_mesh_color_shader() {
         auto* s = this->create_graphics_shader(mesh_color_base_shader_id);
         s->set_render_pass(this->render_passes_by_name.main);
         s->set_layout_info(
            {  // Descriptor set layouts
               this->descriptor_set_layouts.scene_state.handle,
               this->descriptor_set_layouts.all_textures.handle,
               this->descriptor_set_layouts.all_meshes.handle,
               this->descriptor_set_layouts.all_lights.handle,
               this->descriptor_set_layouts.shadow_maps.handle,
            },
            {  // Push constants
               VkPushConstantRange{
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                  .offset     = 0,
                  .size       = sizeof(rendered_mesh::push_constant),
               }
            }
         );
         s->add_variant({ // double-sided shader variant
            .face_cull_mode = VK_CULL_MODE_NONE,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/color-main.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Color Main: rendered_mesh/bslp/color.vert.spv)");
            this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Color Main: rendered_mesh/bslp/color-main.frag.spv)");
         }
         //
         options.stages = {
            {
               .module              = frag,
               .entry_point_name    = "main",
               .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
               .specialization_info = pipeline_stage_specialization_info((int32_t)config::max_rendered_lights),
            },
            {
               .module              = vert,
               .entry_point_name    = "main",
               .stage               = VK_SHADER_STAGE_VERTEX_BIT,
               .specialization_info = pipeline_stage_specialization_info((int32_t)config::max_rendered_lights),
            },
         };
         //dfn.color_blending.blends.emplace_back(graphics_shader::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.rasterization.depthBiasEnable = VK_TRUE; // needed so we can selectively use depth bias during rendering; we'll leave the actual settings at 0, which is functionally off
         options.dynamic_states = {
            VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS, // for decals
         };
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex::getAttributeDescriptions();
            vertex.bindings.push_back(vertex::getBindingDescription());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         // And be sure to set up the pipeline layout when you're done!
         //
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_mesh_wboit_shader() {
         if (!this->can_do_alpha()) {
            //
            // If the device doesn't support the features we need for OIT, then we don't even 
            // define the render pass, so we also shouldn't load any shaders that rely on it.
            //
            return;
         }
         assert(this->render_passes_by_name.main_oit != nullptr);
         //
         auto* s = this->create_graphics_shader(mesh_color_oit_shader_id);
         s->set_render_pass(this->render_passes_by_name.main_oit, 0);
         s->set_layout_info(
            {  // Descriptor set layouts
               this->descriptor_set_layouts.scene_state.handle,
               this->descriptor_set_layouts.all_textures.handle,
               this->descriptor_set_layouts.all_meshes.handle,
               this->descriptor_set_layouts.all_lights.handle,
               this->descriptor_set_layouts.shadow_maps.handle,
            },
            {  // Push constants
               VkPushConstantRange{
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                  .offset     = 0,
                  .size       = sizeof(rendered_mesh::push_constant),
               }
            }
         );
         s->add_variant({ // double-sided shader variant
            .face_cull_mode = VK_CULL_MODE_NONE,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/color-oit.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Color WBOIT: rendered_mesh/bslp/color.vert.spv)");
            this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Color WBOIT: rendered_mesh/bslp/color-oit.frag.spv)");
         }
         //
         options.stages = {
            {
               .module              = frag,
               .entry_point_name    = "main",
               .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
               .specialization_info = pipeline_stage_specialization_info(
                  (int32_t)config::max_rendered_lights
               ),
            },
            {
               .module              = vert,
               .entry_point_name    = "main",
               .stage               = VK_SHADER_STAGE_VERTEX_BIT,
               .specialization_info = pipeline_stage_specialization_info(
                  (int32_t)config::max_rendered_lights
               ),
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{ // accumulator
            .source = {
               .color = VK_BLEND_FACTOR_ONE,
               .alpha = VK_BLEND_FACTOR_ONE,
            },
            .destination = {
               .color = VK_BLEND_FACTOR_ONE,
               .alpha = VK_BLEND_FACTOR_ONE,
            },
            .operations = {
               .color = VK_BLEND_OP_ADD,
               .alpha = VK_BLEND_OP_ADD,
            },
         });
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{ // reveal
            .source = {
               .color = VK_BLEND_FACTOR_ZERO,
               .alpha = VK_BLEND_FACTOR_ZERO,
            },
            .destination = {
               .color = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
               .alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            },
            .operations = {
               .color = VK_BLEND_OP_ADD,
               .alpha = VK_BLEND_OP_ADD,
            },
         });
         options.depth.testing = true;
         options.depth.writing = false;
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.rasterization.depthBiasEnable = VK_TRUE; // needed so we can selectively use depth bias during rendering; we'll leave the actual settings at 0, which is functionally off
         options.dynamic_states = {
            VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS, // for decals
         };
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex::getAttributeDescriptions();
            vertex.bindings.push_back(vertex::getBindingDescription());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         // And be sure to set up the pipeline layout when you're done!
         //
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_mesh_shadows_sun_shader() {
         auto* s = this->create_graphics_shader(shader_id_mesh_shadows_sun);
         s->set_render_pass(this->render_passes_by_name.main_shadow);
         s->set_layout_info(
            {  // Descriptor set layouts
               this->descriptor_set_layouts.scene_state.handle,
               this->descriptor_set_layouts.all_meshes.handle,
               this->descriptor_set_layouts.all_textures.handle,
            },
            {  // Push constants
               VkPushConstantRange{
                  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                  .offset     = 0,
                  .size       = sizeof(rendered_mesh::push_constant),
               }
            }
         );
         s->add_variant({ // double-sided shader variant
            .face_cull_mode = VK_CULL_MODE_NONE,
         });
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-sun.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-sun.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Sun Shadow: rendered_mesh/bslp/shadows-sun.vert.spv)");
            this->set_debug_object_name(frag->handle, "Shader Module (Sun Shadow: rendered_mesh/bslp/shadows-sun.frag.spv)");
         }
         //
         options.stages = {
            {
               .module           = vert,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_VERTEX_BIT,
            },
            {
               .module           = frag,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
         };
         if constexpr (config::sun_shadow_invert_culling) {
            options.rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
         }
         options.rasterization.depthBiasEnable         = VK_TRUE;
         options.rasterization.depthBiasConstantFactor = 1.25F;
         options.rasterization.depthBiasSlopeFactor    = 1.75F;
         options.rasterization.depthBiasClamp          = 0.00F;
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
         if constexpr (config::sun_shadow_invert_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
         } else {
            options.depth.comparison = VK_COMPARE_OP_LESS_OR_EQUAL;
         }
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex::getAttributeDescriptions();
            vertex.bindings.push_back(vertex::getBindingDescription());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         options.area = {
            .mode = graphics_shader::area_mode::custom,
            .scissor = {
               .offset = { .x = 0, .y = 0 },
               .extent = {
                  .width  = config::sun_shadow_map_resolution_x,
                  .height = config::sun_shadow_map_resolution_y,
               },
            },
            .viewport = {
               .x        = 0,
               .y        = 0,
               .width    = config::sun_shadow_map_resolution_x,
               .height   = config::sun_shadow_map_resolution_y,
               .minDepth = 0.0,
               .maxDepth = 1.0,
            },
         };
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_mesh_shadows_caster_shaders() {
         shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-caster.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-caster.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Shadows/Caster: rendered_mesh/bslp/shadows-caster.vert.spv)");
            this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Shadows/Caster: rendered_mesh/bslp/shadows-caster.frag.spv)");
         }
         //
         static_assert(shadow_caster_count < 10, "The way we generate shader IDs here won't work for 10 or more shadow casters.");
         for (size_t i = 0; i < shadow_caster_count; ++i) {
            auto id = shader_id_mesh_shadows_caster;
            id.bytes[7] += i;
            //
            auto* s = this->create_graphics_shader(id);
            s->set_render_pass(this->render_passes_by_name.main_shadow_placed, i);
            s->set_layout_info(
               {  // Descriptor set layouts
                  this->descriptor_set_layouts.scene_state.handle,
                  this->descriptor_set_layouts.all_textures.handle,
                  this->descriptor_set_layouts.all_meshes.handle,
                  this->descriptor_set_layouts.all_lights.handle,
                  this->descriptor_set_layouts.shadow_caster_map_render.handle,
               },
               {  // Push constants
                  VkPushConstantRange{
                     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     .offset     = 0,
                     .size       = sizeof(rendered_mesh::push_constant),
                  }
               }
            );
            s->add_variant({ // double-sided shader variant
               .face_cull_mode = VK_CULL_MODE_NONE,
            });
            //
            auto& options = s->options;
            options.stages = {
               {
                  .module              = vert,
                  .entry_point_name    = "main",
                  .stage               = VK_SHADER_STAGE_VERTEX_BIT,
                  .specialization_info = pipeline_stage_specialization_info((int32_t)i, (int32_t)config::max_rendered_lights),
               },
               {  // Fragment shader needed to discard alpha-tested pixels
                  .module              = frag,
                  .entry_point_name    = "main",
                  .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
               },
            };
            if constexpr (config::light_shadow_invert_culling) {
               options.rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
            }
            options.rasterization.depthBiasEnable         = VK_TRUE;
            options.rasterization.depthBiasConstantFactor = 1.25F;
            options.rasterization.depthBiasSlopeFactor    = 1.75F;
            options.rasterization.depthBiasClamp          = 0.00F;
            options.rasterization.frontFace               = cubemaps_are_lefthanded ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE; // cubemaps are lefthanded in Vulakn, borrowing OpenGL conventions
            options.color_blending.blends.emplace_back(graphics_shader::color_blend{
               .enabled = true,
               .operations = {
                  .color = config::light_shadow_invert_depth ? VK_BLEND_OP_MAX : VK_BLEND_OP_MIN,
                  .alpha = config::light_shadow_invert_depth ? VK_BLEND_OP_MAX : VK_BLEND_OP_MIN,
               },
            });
            options.depth.comparison = VK_COMPARE_OP_ALWAYS;
            //
            {
               auto& vertex     = options.inputs.vertex;
               auto  attributes = vertex::getAttributeDescriptions();
               vertex.bindings.push_back(vertex::getBindingDescription());
               vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
            }
            options.area = {
               .mode = graphics_shader::area_mode::custom,
               .scissor = {
                  .offset = { .x = 0, .y = 0 },
                  .extent = {
                     .width  = config::light_shadow_map_resolution_x,
                     .height = config::light_shadow_map_resolution_y,
                  },
               },
               .viewport = {
                  .x        = 0,
                  .y        = 0,
                  .width    = config::light_shadow_map_resolution_x,
                  .height   = config::light_shadow_map_resolution_y,
                  .minDepth = 0.0,
                  .maxDepth = 1.0,
               },
            };
            //
            // And be sure to set up the pipeline layout when you're done!
            //
            s->setup_pipeline_layout();
         }
      }
   #pragma endregion
   void surface_renderer::_setup_frustum_cull_shader() {
      auto* s = this->create_compute_shader(frustum_cull_shader_id);
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.shared_layouts.compute_cull_frustum.handle,
         }
      );
      shader_module* comp = this->load_shader_module("shaders/compute/culling/frustum.comp.spv");
      {
         assert(comp);
         this->set_debug_object_name(comp->handle, "Shader Module (Frustum Cull: compute/culling/frustum.comp.spv)");
      }
      s->config.stage = pipeline_stage_info{
         .module              = comp,
         .entry_point_name    = "main",
         .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
         .specialization_info = pipeline_stage_specialization_info(
            (int32_t)config::max_rendered_meshes//,
         ),
      };
      s->setup(*this);
   }
   void surface_renderer::_setup_shadow_caster_cull_shaders() {
      static_assert(config::max_active_shadow_casters < 9, "if we want more than 10 shadow casters, then we need to change how we generate these shader IDs");
      for (size_t i = 0; i < config::max_active_shadow_casters; ++i) {
         auto id = shadow_caster_cull_shader_base_id;
         id.bytes[7] += i;
         //
         auto* s = this->create_compute_shader(id);
         s->set_layout_info(
            {  // Descriptor set layouts
               this->descriptor_set_layouts.shared_layouts.compute_cull_caster.handle,
            }
         );
         shader_module* comp = this->load_shader_module("shaders/compute/culling/shadows-caster.comp.spv");
         {
            assert(comp);
            this->set_debug_object_name(comp->handle, "Shader Module (Shadow Caster Cull: compute/culling/shadows-caster.comp.spv)");
         }
         s->config.stage = pipeline_stage_info{
            .module              = comp,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
            .specialization_info = pipeline_stage_specialization_info(
               (int32_t)config::max_rendered_meshes,
               (int32_t)config::max_active_shadow_casters,
               (int32_t)i//,
            ),
         };
         s->setup(*this);
      }
   }

   void surface_renderer::_setup_scene_bounds_shaders() {
      #pragma region Bounding box
      {
         auto* s = this->create_graphics_shader(bounding_box_shader_id);
         s->set_render_pass(this->render_passes_by_name.bounds);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_bounds.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_bounds/box/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_bounds/box/color.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Bounding Box Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Bounding Box Frag)");
         }
         //
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
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
         options.rasterization.cullMode    = VK_CULL_MODE_NONE;
         if (this->device_info->support.non_solid_polygon_fill_modes) {
            options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
         }
         //
         s->setup_pipeline_layout();
      }
      #pragma endregion
      #pragma region Pivot
      {
         auto* s = this->create_graphics_shader(bounding_origin_shader_id);
         s->set_render_pass(this->render_passes_by_name.bounds);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_bounds.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_bounds/pivot/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_bounds/pivot/color.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Bounding Box Pivot Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Bounding Box Pivot Frag)");
         }
         //
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
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
         options.rasterization.cullMode    = VK_CULL_MODE_NONE;
         if (this->device_info->support.non_solid_polygon_fill_modes) {
            options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
         }
         //
         s->setup_pipeline_layout();
      }
      #pragma endregion
   }

   #pragma region rendered_landscape shaders
   void surface_renderer::_setup_rendered_landscape_shaders() {
      this->_setup_rendered_landscape_color_shader();
      this->_setup_rendered_landscape_shadows_caster_shaders();
      this->_setup_rendered_landscape_shadows_sun_shader();
      this->_setup_rendered_landscape_border_shader();
      this->_setup_rendered_landscape_wireframe_shader();
      this->_setup_rendered_landscape_normals_shader();
   }
      void surface_renderer::_setup_rendered_landscape_color_shader() {
         auto* s = this->create_graphics_shader(landscape_shader_id);
         s->set_render_pass(this->render_passes_by_name.main);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_textures.handle,
            this->descriptor_set_layouts.all_landscapes.handle,
            this->descriptor_set_layouts.all_lights.handle,
            this->descriptor_set_layouts.shadow_maps.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_landscape/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_landscape/color.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Landscape Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Landscape Frag)");
         }
         //
         options.stages = {
            {
               .module = frag,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
               .specialization_info = pipeline_stage_specialization_info((int32_t)config::max_rendered_lights),
            },
            {
               .module = vert,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex_landscape::attribute_descriptions();
            vertex.bindings.push_back(vertex_landscape::binding_description());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_landscape_shadows_caster_shaders() {
         shader_module* vert = this->load_shader_module("shaders/rendered_landscape/shadows-caster.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_landscape/shadows-caster.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Shadows/Caster: rendered_landscape/shadows-caster.vert.spv)");
            this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Shadows/Caster: rendered_landscape/shadows-caster.frag.spv)");
         }
         //
         static_assert(shadow_caster_count < 10, "The way we generate shader IDs here won't work for 10 or more shadow casters.");
         for (size_t i = 0; i < shadow_caster_count; ++i) {
            auto id = shader_id_landscape_shadows_caster;
            id.bytes[7] += i;
            //
            auto* s = this->create_graphics_shader(id);
            s->set_render_pass(this->render_passes_by_name.main_shadow_placed, i);
            s->set_layout_info(
               {  // Descriptor set layouts
                  this->descriptor_set_layouts.scene_state.handle,
                  this->descriptor_set_layouts.all_landscapes.handle,
                  this->descriptor_set_layouts.all_lights.handle,
                  this->descriptor_set_layouts.shadow_caster_map_render.handle,
               }
            );
            //
            auto& options = s->options;
            options.stages = {
               {
                  .module              = vert,
                  .entry_point_name    = "main",
                  .stage               = VK_SHADER_STAGE_VERTEX_BIT,
                  .specialization_info = pipeline_stage_specialization_info((int32_t)i, (int32_t)config::max_rendered_lights),
               },
               {
                  .module              = frag,
                  .entry_point_name    = "main",
                  .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
               },
            };
            if constexpr (config::light_shadow_invert_culling) {
               options.rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
            }
            options.rasterization.depthBiasEnable         = VK_TRUE;
            options.rasterization.depthBiasConstantFactor = 1.25F;
            options.rasterization.depthBiasSlopeFactor    = 1.75F;
            options.rasterization.depthBiasClamp          = 0.00F;
            options.rasterization.frontFace               = cubemaps_are_lefthanded ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE; // cubemaps are lefthanded in Vulakn, borrowing OpenGL conventions
            options.color_blending.blends.emplace_back(graphics_shader::color_blend{
               .enabled = true,
               .operations = {
                  .color = config::light_shadow_invert_depth ? VK_BLEND_OP_MAX : VK_BLEND_OP_MIN,
                  .alpha = config::light_shadow_invert_depth ? VK_BLEND_OP_MAX : VK_BLEND_OP_MIN,
               },
            });
            options.depth.comparison = VK_COMPARE_OP_ALWAYS;
            //
            {
               auto& vertex     = options.inputs.vertex;
               auto  attributes = vertex_landscape::attribute_descriptions();
               vertex.bindings.push_back(vertex_landscape::binding_description());
               vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
            }
            options.area = {
               .mode = graphics_shader::area_mode::custom,
               .scissor = {
                  .offset = { .x = 0, .y = 0 },
                  .extent = {
                     .width  = config::light_shadow_map_resolution_x,
                     .height = config::light_shadow_map_resolution_y,
                  },
               },
               .viewport = {
                  .x        = 0,
                  .y        = 0,
                  .width    = config::light_shadow_map_resolution_x,
                  .height   = config::light_shadow_map_resolution_y,
                  .minDepth = 0.0,
                  .maxDepth = 1.0,
               },
            };
            //
            // And be sure to set up the pipeline layout when you're done!
            //
            s->setup_pipeline_layout();
         }
      }
      void surface_renderer::_setup_rendered_landscape_shadows_sun_shader() {
         auto* s = this->create_graphics_shader(shader_id_landscape_shadows_sun);
         s->set_render_pass(this->render_passes_by_name.main_shadow);
         s->set_layout_info(
            {  // Descriptor set layouts
               this->descriptor_set_layouts.scene_state.handle,
               this->descriptor_set_layouts.all_landscapes.handle,
            }
         );
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_landscape/shadows-sun.vert.spv");
         {
            assert(vert);
            this->set_debug_object_name(vert->handle, "Shader Module (Sun Shadow: rendered_landscapep/shadows-sun.vert.spv)");
         }
         //
         options.stages = {
            {
               .module           = vert,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         if constexpr (config::sun_shadow_invert_culling) {
            options.rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
         }
         options.rasterization.depthBiasEnable         = VK_TRUE;
         options.rasterization.depthBiasConstantFactor = 1.25F;
         options.rasterization.depthBiasSlopeFactor    = 1.75F;
         options.rasterization.depthBiasClamp          = 0.00F;
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
         if constexpr (config::sun_shadow_invert_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
         } else {
            options.depth.comparison = VK_COMPARE_OP_LESS_OR_EQUAL;
         }
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex_landscape::attribute_descriptions();
            vertex.bindings.push_back(vertex_landscape::binding_description());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         options.area = {
            .mode = graphics_shader::area_mode::custom,
            .scissor = {
               .offset = { .x = 0, .y = 0 },
               .extent = {
                  .width  = config::sun_shadow_map_resolution_x,
                  .height = config::sun_shadow_map_resolution_y,
               },
            },
            .viewport = {
               .x        = 0,
               .y        = 0,
               .width    = config::sun_shadow_map_resolution_x,
               .height   = config::sun_shadow_map_resolution_y,
               .minDepth = 0.0,
               .maxDepth = 1.0,
            },
         };
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_landscape_border_shader() {
         auto* s = this->create_graphics_shader(landscape_border_shader_id);
         s->set_render_pass(this->render_passes_by_name.main);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_landscapes.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_landscape/border.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_landscape/border.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Landscape Border Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Landscape Border Frag)");
         }
         //
         options.stages = {
            {
               .module = frag,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
            {
               .module = vert,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         options.depth.comparison = VK_COMPARE_OP_LESS_OR_EQUAL; // include "equal" so the wireframe shows up over the actual landscape
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
         }
         options.rasterization.cullMode = VK_CULL_MODE_NONE;
         options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex_landscape::attribute_descriptions();
            vertex.bindings.push_back(vertex_landscape::binding_description());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_landscape_wireframe_shader() {
         auto* s = this->create_graphics_shader(landscape_wireframe_shader_id);
         s->set_render_pass(this->render_passes_by_name.main);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_landscapes.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_landscape/wireframe.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_landscape/wireframe.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Landscape Wire Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Landscape Wire Frag)");
         }
         //
         options.stages = {
            {
               .module = frag,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
            {
               .module = vert,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         options.depth.comparison = VK_COMPARE_OP_LESS_OR_EQUAL; // include "equal" so the wireframe shows up over the actual landscape
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
         }
         options.rasterization.cullMode = VK_CULL_MODE_NONE;
         if (this->device_info->support.non_solid_polygon_fill_modes) {
            options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
         } else {
            //
            // Rendering a bog-standard mesh as a wireframe isn't supported on this card? 
            // We could construct a vertex buffer specifically designed for wireframes, 
            // but instead, let's just fall back to a point cloud.
            //
            options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
         }
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex_landscape::attribute_descriptions();
            vertex.bindings.push_back(vertex_landscape::binding_description());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         s->setup_pipeline_layout();
      }
      void surface_renderer::_setup_rendered_landscape_normals_shader() {
         if (!this->device_info->support.geometry_shaders.available)
            return;
         //
         auto* s = this->create_graphics_shader(landscape_normals_shader_id);
         s->set_render_pass(this->render_passes_by_name.main);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_landscapes.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_landscape/normals.vert.spv");
         shader_module* geom = this->load_shader_module("shaders/rendered_landscape/normals.geom.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_landscape/normals.frag.spv");
         {
            assert(vert);
            assert(geom);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Landscape Normals Vert)");
            this->set_debug_object_name(geom->handle, "Shader Module (Landscape Normals Geom)");
            this->set_debug_object_name(frag->handle, "Shader Module (Landscape Normals Frag)");
         }
         //
         options.stages = {
            {
               .module = frag,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
            {
               .module = geom,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
            },
            {
               .module = vert,
               .entry_point_name = "main",
               .stage = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
         options.rasterization.cullMode = VK_CULL_MODE_NONE;
         if (this->device_info->support.non_solid_polygon_fill_modes) {
            options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
         }
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex_landscape::attribute_descriptions();
            vertex.bindings.push_back(vertex_landscape::binding_description());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         s->setup_pipeline_layout();
      }
   #pragma endregion
   void surface_renderer::_setup_debug_grid_shader() {
      if constexpr (config::debug_grid_uses_wboit) {
         if (!this->can_do_alpha()) {
            return;
         }
         assert(this->render_passes_by_name.main_oit != nullptr);
      }
      //
      auto* s = this->create_graphics_shader(debug_grid_color_shader_id);
      if constexpr (config::debug_grid_uses_wboit) {
         s->set_render_pass(this->render_passes_by_name.main_oit, 0);
      } else {
         s->set_render_pass(this->render_passes_by_name.main);
      }
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.scene_state.handle,
         }
      );
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/bespoke/grid/color.vert.spv");
      shader_module* frag;
      if constexpr (config::debug_grid_uses_wboit) {
         frag = this->load_shader_module("shaders/bespoke/grid/color-oit.frag.spv");
         assert(frag);
         this->set_debug_object_name(frag->handle, "Shader Module (Grid Color: bespoke/grid/color-oit.frag.spv)");
      } else {
         frag = this->load_shader_module("shaders/bespoke/grid/color-main.frag.spv");
         assert(frag);
         this->set_debug_object_name(frag->handle, "Shader Module (Grid Color: bespoke/grid/color-main.frag.spv)");
      }
      assert(vert);
      this->set_debug_object_name(vert->handle, "Shader Module (Grid Color: rendered_mesh/bslp/color.vert.spv)");
      //
      options.stages = {
         {
            .module              = frag,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .module              = vert,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_VERTEX_BIT,
         },
      };
      options.rasterization.cullMode = VK_CULL_MODE_NONE; // no backface culling
      if constexpr (config::debug_grid_uses_wboit) {
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{ // accumulator
            .source = {
               .color = VK_BLEND_FACTOR_ONE,
               .alpha = VK_BLEND_FACTOR_ONE,
            },
            .destination = {
               .color = VK_BLEND_FACTOR_ONE,
               .alpha = VK_BLEND_FACTOR_ONE,
            },
            .operations = {
               .color = VK_BLEND_OP_ADD,
               .alpha = VK_BLEND_OP_ADD,
            },
         });
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{ // reveal
            .source = {
               .color = VK_BLEND_FACTOR_ZERO,
               .alpha = VK_BLEND_FACTOR_ZERO,
            },
            .destination = {
               .color = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
               .alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            },
            .operations = {
               .color = VK_BLEND_OP_ADD,
               .alpha = VK_BLEND_OP_ADD,
            },
         });
      } else {
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
      }
      if constexpr (config::use_inverted_depth) {
         options.depth.comparison = VK_COMPARE_OP_GREATER;
      }
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }

   void surface_renderer::_setup_gizmo_shader() {
      auto* s = this->create_graphics_shader(edit_gizmo_color_shader_id);
      s->set_render_pass(this->render_passes_by_name.main);
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.gizmo_state.handle,
         }
      );
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/gizmo/color.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/gizmo/color.frag.spv");
      assert(frag);
      this->set_debug_object_name(frag->handle, "Shader Module (Edit Gizmo Color: gizmo/color.frag.spv)");
      assert(vert);
      this->set_debug_object_name(vert->handle, "Shader Module (Edit Gizmo Color: gizmo/color.vert.spv)");
      //
      options.stages = {
         {
            .module              = frag,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .module              = vert,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_VERTEX_BIT,
         },
      };
      options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend);
      options.rasterization.cullMode = VK_CULL_MODE_NONE; // no backface culling
      if constexpr (config::use_inverted_depth) {
         options.depth.comparison = VK_COMPARE_OP_GREATER;
      }
      {
         auto va = std::array{
            VkVertexInputAttributeDescription{
               .location = 0, // should match the location value in the shader's code
               .binding  = 0,
               .format   = VK_FORMAT_R32G32B32A32_SFLOAT, // vec4
               .offset   = 0,
            },
         };

         auto& vertex = options.inputs.vertex;
         vertex.bindings.push_back(VkVertexInputBindingDescription{
            .binding   = 0,
            .stride    = sizeof(glm::vec4),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
         });
         vertex.attributes.insert(vertex.attributes.end(), va.begin(), va.end());
      }
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }

   void surface_renderer::_setup_shaders() {
      this->_setup_scene_background_shader();
      //
      this->_setup_oit_composite_shader();
      //
      this->_setup_rendered_mesh_shaders();
      this->_setup_rendered_landscape_shaders();
      this->_setup_frustum_cull_shader();
      this->_setup_shadow_caster_cull_shaders();
      this->_setup_scene_bounds_shaders();
      //
      this->_setup_debug_grid_shader();
      //
      this->_setup_gizmo_shader();
      //
      // FPS counter:
      //
      vulkanDK::overlays::fps::setup_shaders(*this);
      vulkanDK::overlays::world_axes::setup_shaders(*this);
   }
}