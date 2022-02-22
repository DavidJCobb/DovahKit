#include "fps.h"
#include <QPainter>
#include <QResource>
#include "../surface_renderer.h"

namespace {
   static constexpr int  ABSURDLY_LARGE_SIZE       = 9999;
   static constexpr bool unnormalized_coordinates  = true; // refer to texture sampler's options
}

namespace vulkanDK::overlays {
   /*static*/ std::array<VkVertexInputAttributeDescription, 2> fps::_vertex::getAttributeDescriptions() {
      return std::array{
         VkVertexInputAttributeDescription{
            .location = 0, // should match the location value in the shader's code
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT, // vec3
            .offset   = offsetof(_vertex, pos),
         },
         VkVertexInputAttributeDescription{ // UVs
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = offsetof(_vertex, uv),
         },
      };
   }
   /*static*/ VkVertexInputBindingDescription fps::_vertex::getBindingDescription() {
      return VkVertexInputBindingDescription{
         .binding   = 0,
         .stride    = sizeof(_vertex),
         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
      };
   }

   fps::fps() {
      this->style.font.setFamily("Lucida Console");
      this->style.label = QLatin1String("FPS: ");
      this->style.space_between_digits = 1.0F;
   }

   void fps::_set_quad_x(qreal x, _vertex* v, const QRect& glyph) {
      v[0].pos.x = x;
      v[1].pos.x = x + glyph.width();
      v[2].pos.x = v[1].pos.x;
      v[3].pos.x = x;
      //
      for (int i = 0; i < vertices_per_quad; ++i)
         v[i].pos.z = 0.0F;
   }
   void fps::_set_quad_pos(QPoint pos, _vertex* v, const QRect& glyph) {
      _set_quad_x(pos.x(), v, glyph);
      v[0].pos.y = pos.y();
      v[1].pos.y = pos.y();
      v[2].pos.y = pos.y() + glyph.height();
      v[3].pos.y = v[2].pos.y;
   }
   void fps::_set_quad_uv(_vertex* v, const QRect& glyph) {
      auto x = (float)glyph.x();
      auto y = (float)glyph.y();
      auto r = x + glyph.width();
      auto b = y + glyph.height();
      if constexpr (!unnormalized_coordinates) {
         x /= (float)this->atlas_info.size.width();
         y /= (float)this->atlas_info.size.height();
         r /= (float)this->atlas_info.size.width();
         b /= (float)this->atlas_info.size.height();
      }
      v[0].uv = { x, y };
      v[1].uv = { r, y };
      v[2].uv = { r, b };
      v[3].uv = { x, b };
   }

   /*static*/ void fps::setup_shaders(surface_renderer& sr) {
      auto* s = sr.get_or_create_shader(shader_id);
      s->set_render_pass(sr.render_passes_by_name.ui);
      s->set_layout_info(
         {  // Descriptor set layouts
            sr.descriptor_set_layouts.fps.handle,
         }
      );
      //
      auto& dfn = s->definition;
      //
      shader_module* frag = nullptr;
      shader_module* vert = nullptr;
      {
         frag = new shader_module(sr.logical_device, QResource("shaders/overlay-fps.frag.spv").uncompressedData());
         vert = new shader_module(sr.logical_device, QResource("shaders/overlay-fps.vert.spv").uncompressedData());
         if (frag->empty()) {
            throw std::runtime_error("[vulkanDK::overlays::fps::setup_shaders] Failed to load fragment shader.");
         }
         if (vert->empty()) {
            throw std::runtime_error("[vulkanDK::overlays::fps::setup_shaders] Failed to load vertex shader.");
         }
         sr.shader_modules.push_back(frag);
         sr.shader_modules.push_back(vert);
      }
      dfn.stages = {
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
      dfn.color_blending.blends.emplace_back(material_definition::default_alpha_blend);
      {
         auto& vertex     = dfn.inputs.vertex;
         auto  attributes = _vertex::getAttributeDescriptions();
         vertex.bindings.push_back(_vertex::getBindingDescription());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      s->setup_pipeline_layout(sr);
   }
   void fps::initialize_descriptor_sets(surface_renderer& sr, swap_chain_image& sci) {
      auto sampler_info = VkDescriptorImageInfo{
         .sampler     = sr.raw_pixel_texture_sampler,
         .imageView   = VK_NULL_HANDLE,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      auto buffer_info = VkDescriptorBufferInfo{
         .buffer = this->shader_params.uniform.handle,
         .offset = 0,
         .range  = sizeof(_shader_state), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
      };
      //
      auto descriptor_writes = std::array{
         VkWriteDescriptorSet{ // uniform buffer object
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet           = sci.descriptor_sets.fps,
            .dstBinding       = 0, // this should match the binding value in the shader
            .dstArrayElement  = 0, // index of the first descriptor in the raray to update
            .descriptorCount  = 1, // you can update multiple descriptors at once if they're in an array
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &buffer_info,
            .pTexelBufferView = nullptr,
         },
         VkWriteDescriptorSet{ // texture sampler
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = sci.descriptor_sets.fps,
            .dstBinding      = 1, // this should match the binding value in the shader
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = &sampler_info,
         },
      };
      vkUpdateDescriptorSets(sr.logical_device, (uint32_t)descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
   }
   void fps::setup_shader_parameter_buffers(surface_renderer& sr) {
      constexpr VkDeviceSize buffer_size = sizeof(_shader_state);
      this->shader_params.uniform = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      //
      // Only need to update these when the viewport size changes, which is always accompanied by the swap 
      // chain images being set up; they set us up; ergo we only need to set this initially, during setup.
      //
      auto* data = (_shader_state*) this->shader_params.uniform.map_memory();
      data->view_w = sr.surface_extent.width;
      data->view_h = sr.surface_extent.height;
      this->shader_params.uniform.unmap_memory(data);
   }
   void fps::create_geometry(surface_renderer& sr) {
      auto& vib     = this->vertex_and_index_buffer;
      auto& staging = this->vi_staging_buffer;
      //
      constexpr VkDeviceSize buffer_size_v = sizeof(_vertex)  * vertex_count;
      constexpr VkDeviceSize buffer_size_i = sizeof(uint16_t) * index_count;
      constexpr VkDeviceSize buffer_size   = buffer_size_v + buffer_size_i;
      //
      staging = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data = staging.map_memory();
      memset(data, 0, buffer_size);
      {
         auto* indices = (uint16_t*)((std::intptr_t)data + buffer_size_v);
         for (size_t i = 0; i < quad_count; ++i) {
            //
            // Faces must have a counterclockwise vertex order to be considered "facing the camera."
            //
            indices[i * 6 + 0] = (i * vertices_per_quad);
            indices[i * 6 + 1] = (i * vertices_per_quad) + 3;
            indices[i * 6 + 2] = (i * vertices_per_quad) + 2;
            indices[i * 6 + 3] = (i * vertices_per_quad) + 2;
            indices[i * 6 + 4] = (i * vertices_per_quad) + 1;
            indices[i * 6 + 5] = (i * vertices_per_quad);
         }
      }
      staging.unmap_memory(data);
      //
      vib = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      vib.copy_from(staging);
      //
      if constexpr (persistent_staging_buffer) {
         //
         // Set up a persistent staging buffer:
         //
         staging = sr.create_buffer(_vib_indices_offset, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      } else {
         staging = buffer(); // destroy
      }
   }

   void fps::set_font(QFont f) {
      if (this->style.font == f)
         return;
      this->style.font = f;
      this->change_flags.set<change_flag::style>();
   }
   void fps::set_label(const QString& l) {
      if (this->style.label == l)
         return;
      this->style.label = l;
      this->change_flags.set<change_flag::style>();
   }
   void fps::set_value(value_type v) {
      if constexpr (show_history_average) {
         auto& h = this->history;
         if (h.count == 0) {
            h.average = v;
            h.count   = 1;
         } else if (h.count == std::numeric_limits<decltype(h.count)>::max() - 1) { // overflow imminent
            h.average = v;
            h.count   = 1;
         } else {
            using average_t = decltype(h.average);
            //
            constexpr bool alternate_method = true;
            if constexpr (alternate_method) {
               ++h.count;
               h.average += ((average_t)v - h.average) / h.count;
            } else {
               h.average = ((average_t)v + (average_t)h.count * h.average) / (h.count + 1);
               ++h.count;
            }
         }
      } else {
         if (this->value == v)
            return;
         this->value = v;
      }
      if constexpr (!assume_always_redraw) {
         this->change_flags.set<change_flag::value>();
      }
   }
   void fps::set_digit_spacing(float ds) {
      if (this->style.space_between_digits == ds)
         return;
      this->style.space_between_digits = ds;
      this->change_flags.set<change_flag::positions>();
   }

   bool fps::needs_atlas_update() const {
      return this->change_flags.test<change_flag::style>();
   }
   bool fps::needs_geometry_update() const {
      if constexpr (assume_always_redraw) {
         return true;
      }
      if (!this->change_flags.empty())
         return true;
      return false;
   }

   QImage fps::generate_atlas() {
      constexpr uint32_t gap_between_glyphs = 2;
      constexpr auto     glyph_text_flags   = Qt::AlignLeft | Qt::AlignAbsolute | Qt::AlignTop;
      //
      // Measure atlas:
      //
      {
         QRect    size;
         QImage   dummy   = QImage(1, 1, QImage::Format::Format_Mono);
         QPainter painter = QPainter(&dummy);
         //
         for (int i = 0; i < display_base; ++i) {
            painter.drawText(QRect(0, 0, ABSURDLY_LARGE_SIZE, ABSURDLY_LARGE_SIZE), glyph_text_flags, QString::number(i, display_base), &size);
            this->atlas_info.digits[i] = size;
         }
         {
            painter.drawText(QRect(0, 0, ABSURDLY_LARGE_SIZE, ABSURDLY_LARGE_SIZE), glyph_text_flags, this->style.label, &size);
            this->atlas_info.label = size;
         }
      }
      //
      // Generate atlas:
      //
      uint32_t w = this->atlas_info.label.width();
      uint32_t h = this->atlas_info.label.height();
      for (auto& digit : this->atlas_info.digits) {
         w += gap_between_glyphs;
         w += digit.width();
         h = (std::max)(h, (uint32_t)digit.height());
      }
      this->atlas_info.size = QSize(w, h);
      //
      auto image   = QImage(w, h, QImage::Format::Format_RGBA8888); // TODO: pick a better format
      auto painter = QPainter(&image);
      image.fill(Qt::GlobalColor::transparent);
      painter.setPen(QColor(255, 255, 255));
      painter.setBrush(QColor(255, 255, 255));
      painter.drawText(this->atlas_info.label, glyph_text_flags, this->style.label);
      //
      int x = this->atlas_info.label.width();
      for (int i = 0; i < display_base; ++i) {
         auto& glyph = this->atlas_info.digits[i];
         //
         x += gap_between_glyphs;
         glyph.moveLeft(x);
         x += glyph.width();
         //
         painter.drawText(glyph, glyph_text_flags, QString::number(i, display_base));
      }
      this->change_flags.reset<change_flag::style>();
      this->change_flags.set<change_flag::atlas>();
      return image;
   }
   void fps::generate_atlas(surface_renderer& sr, swap_chain_image& sci) {
      {
         QImage   texture = this->generate_atlas();
         uint32_t w = texture.width();
         uint32_t h = texture.height();
         //
         VkDeviceSize image_size = w * h * 4;
         //
         // We're gonna be setting up our image on a staging buffer, and then transferring that 
         // to the final (non-CPU-writeable) buffer.
         //
         auto  staging = sr.create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         void* data    = staging.map_memory();
         memcpy(data, texture.constBits(), image_size);
         staging.unmap_memory(data);
         //
         texture = QImage();
         //
         auto& content = this->atlas_info.image;
         content = concrete_image(sr);
         content.create_image(
            {
               .extent = {
                  .width  = w,
                  .height = h,
               },
               .format = VK_FORMAT_R8G8B8A8_SRGB,
               .usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            },
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
         );
         content.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
         content.copy_content_from_buffer(staging.handle);
         content.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
         content.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      }
      //
      // Update descriptor:
      //
      auto infos = std::array{
         VkDescriptorImageInfo{
            .sampler     = sr.raw_pixel_texture_sampler,
            .imageView   = this->atlas_info.image.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         },
      };
      auto writes = std::array{
         VkWriteDescriptorSet{
            .sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet     = sci.descriptor_sets.fps,
            .dstBinding = 1, // this should match the binding value in the shader
            .dstArrayElement = 0,
            .descriptorCount = (uint32_t)infos.size(),
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = infos.data(),
         },
      };
      vkUpdateDescriptorSets(sr.logical_device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
   }
   void fps::teardown_atlas() {
      this->atlas_info.image.teardown();
      //
      this->atlas_info.size  = QSize();
      this->atlas_info.label = QRect();
      for (auto& d : this->atlas_info.digits)
         d = QRect();
      //
      this->change_flags.set<change_flag::atlas>();
      this->change_flags.set<change_flag::style>();
   }

   void fps::update_geometry(surface_renderer& sr) {
      auto& vib     = this->vertex_and_index_buffer;
      auto& staging = this->vi_staging_buffer;
      //
      // We're only going to update the vertices, not the indices, so we'll just use a 
      // staging buffer with only enough room for the vertices.
      //
      if constexpr (!persistent_staging_buffer) {
         staging = sr.create_buffer(_vib_indices_offset, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
      void* data = staging.map_memory();
      this->update_geometry(data);
      staging.unmap_memory(data);
      //
      if constexpr (!persistent_staging_buffer) {
         vib.copy_from(staging);
         staging = buffer();
      } else {
         //
         // The buffer-copy is performed along with our draw call, saving us the overhead of 
         // creating and destroying temporary command buffers (and waiting on the queue submit).
         //
      }
   }
   void fps::update_geometry(void* mapped_vertex_memory) {
      auto* vertices = (_vertex*)mapped_vertex_memory;
      //
      // We're writing to a staging buffer, which is initially blank, so we have to set all 
      // vertices; we can't only update the ones that need updating. :(
      //
      {
         //
         // Initial setup for the "label" quad (i.e. "FPS: ").
         //
         const auto& glyph = this->atlas_info.label;
         _set_quad_pos({ 0, 0 }, vertices, glyph);
         _set_quad_uv(vertices, glyph);
      }
      //
      std::array<uint8_t, max_digits> digits;
      {
         value_type v;
         if constexpr (show_history_average) {
            v = this->history.average;
         } else {
            v = this->value;
         }
         if (v > max_visible_value) {
            //
            // If the FPS count exceeds the maximum number that can be displayed with the digit 
            // count we have, then the number will be truncated (e.g. "1234567" -> "34567" for 
            // a digit count of 5). This is undesired; it'd be cleaner to force all digits to 
            // the highest one (i.e. nines in base-10).
            //
            for (auto& n : digits)
               n = 9;
         } else {
            for (size_t i = 0; i < max_digits; ++i, (v /= display_base)) {
               digits[max_digits - i - 1] = v % display_base;
            }
         }
      }
      //
      // Vertices:
      //
      int    x = this->atlas_info.label.x() + this->atlas_info.label.width(); // this is not the same as QRect::right(), apparently; not sure what's up with that
      size_t i = 0;
      if (this->style.omit_leading_zeroes) {
         for (; i < max_digits - 1; ++i) {
            if (digits[i] != 0)
               break;
         }
         if (i) {
            memset(&vertices[vertices_per_quad], 0, sizeof(_vertex) * vertices_per_quad * i);
         }
      }
      for (; i < max_digits; ++i) {
         auto* digit_verts = &vertices[vertices_per_quad + vertices_per_quad * i];
         //
         const auto& glyph = this->atlas_info.digits[digits[i]];
         _set_quad_pos({ x, 0 }, digit_verts, glyph);
         _set_quad_uv(digit_verts, glyph);
         //
         x += glyph.width();
         x += this->style.space_between_digits;
      }
      //
      // And we're done!
      //
      this->change_flags.reset_all_of<change_flag::atlas, change_flag::positions, change_flag::value>();
   }

   void fps::commands_pre_pass(VkCommandBuffer command_buffer) {
      if constexpr (persistent_staging_buffer) {
         auto& vib     = this->vertex_and_index_buffer;
         auto& staging = this->vi_staging_buffer;
         //
         auto copy_region = VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = staging.size,
         };
         vkCmdCopyBuffer(command_buffer, staging.handle, vib.handle, 1, &copy_region);
      }
   }
   void fps::draw_call(VkCommandBuffer command_buffer) {
      auto& vib = this->vertex_and_index_buffer;
      //
      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(command_buffer, 0, 1, &vib.handle, &offset);
      vkCmdBindIndexBuffer  (command_buffer, vib.handle, _vib_indices_offset, VK_INDEX_TYPE_UINT16);
      vkCmdDrawIndexed(command_buffer, (uint32_t)index_count, 1, 0, 0, 0);
   }
}