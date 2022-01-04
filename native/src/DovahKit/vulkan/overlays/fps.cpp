#include "fps.h"
#include <QPainter>
#include <QResource>
#include "../surface_renderer.h"

namespace {
   static constexpr int ABSURDLY_LARGE_SIZE = 9999;
}

namespace vulkanDK::overlays {
   /*static*/ std::array<VkVertexInputAttributeDescription, 2> fps::_vertex::getAttributeDescriptions() {
      return std::array{
         VkVertexInputAttributeDescription{
            .location = 0, // should match the location value in the shader's code
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT, // vec3
            .offset   = offsetof(vertex, pos),
         },
         VkVertexInputAttributeDescription{ // UVs
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = offsetof(vertex, texCoord),
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
   }

   void fps::_set_quad_x(qreal x, _vertex* v, const QRect& glyph) {
      v[0].pos.x = x;
      v[1].pos.x = x + glyph.width();
      v[2].pos.x = v[1].pos.x;
      v[3].pos.x = x;
   }
   void fps::_set_quad_pos(QPoint pos, _vertex* v, const QRect& glyph) {
      _set_quad_x(pos.x(), v, glyph);
      v[0].pos.y = pos.y();
      v[1].pos.y = pos.y();
      v[2].pos.y = pos.y() + glyph.height();
      v[3].pos.y = v[1].pos.y;
   }
   void fps::_set_quad_uv(_vertex* v, const QRect& glyph) {
      auto x = glyph.x() / this->atlas_info.size.width();
      auto y = glyph.y() / this->atlas_info.size.height();
      auto r = glyph.right()  / this->atlas_info.size.width();
      auto b = glyph.bottom() / this->atlas_info.size.height();
      //
      v[0].uv = { x, y };
      v[1].uv = { r, y };
      v[2].uv = { r, b };
      v[3].uv = { x, b };
   }

   /*static*/ void fps::create_material_definitions(surface_renderer& sr) {
      auto& dfn = sr.material_definitions.emplace_back();
      auto vert_binding    = vertex::getBindingDescription();
      auto vert_attributes = vertex::getAttributeDescriptions();
      //
      shader_module* frag = nullptr;
      shader_module* vert = nullptr;
      {
         frag = new shader_module(sr.logical_device, QResource("shaders/overlay-fps.frag.spv").uncompressedData());
         vert = new shader_module(sr.logical_device, QResource("shaders/overlay-fps.vert.spv").uncompressedData());
         sr.shader_modules.push_back(frag);
         sr.shader_modules.push_back(vert);
      }
      if (frag->empty()) {
         throw std::runtime_error("[vulkanDK::overlays::fps::create_material_definitions::create_material_definitions] Failed to load fragment shader.");
      }
      if (vert->empty()) {
         throw std::runtime_error("[vulkanDK::overlays::fps::create_material_definitions::create_material_definitions] Failed to load vertex shader.");
      }
      //
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
      dfn.color_blending.blends.emplace_back(material_definition::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
      {
         auto& vertex     = dfn.inputs.vertex;
         auto  attributes = _vertex::getAttributeDescriptions();
         vertex.bindings.push_back(_vertex::getBindingDescription());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
   }
   void fps::setup_texture_sampler() {
   }
   void fps::initialize_descriptor_sets(surface_renderer& sr, swap_chain_image& sci) {
      auto sampler_info = VkDescriptorImageInfo{
         .sampler     = sr.texture_sampler,
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
            .dstSet           = sci.descriptor_sets[descriptor_set_index],
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
            .dstSet          = sci.descriptor_sets[descriptor_set_index],
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
      auto& vib = this->vertex_and_index_buffer;
      //
      constexpr VkDeviceSize buffer_size_v = sizeof(_vertex)  * vertex_count;
      constexpr VkDeviceSize buffer_size_i = sizeof(uint16_t) * index_count;
      constexpr VkDeviceSize buffer_size   = buffer_size_v + buffer_size_i;
      //
      auto  staging = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data    = staging.map_memory();
      memset(data, 0, buffer_size_v);
      {
         auto* indices = (uint16_t*)((std::intptr_t)data + buffer_size_v);
         for (size_t i = 0; i < quad_count; ++i) {
            indices[i * 6 + 0] = i;
            indices[i * 6 + 1] = i + 1;
            indices[i * 6 + 2] = i + 2;
            indices[i * 6 + 3] = i + 2;
            indices[i * 6 + 4] = i + 3;
            indices[i * 6 + 5] = i;
         }
      }
      staging.unmap_memory(data);
      //
      vib = sr.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      vib.copy_from(staging);
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
      if (this->value == v)
         return;
      this->value = v;
      this->change_flags.set<change_flag::value>();
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
      if (!this->change_flags.empty())
         return true;
      return this->value != this->last_value;
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
      //
      auto image   = QImage(w, h, QImage::Format::Format_ARGB32); // TODO: pick a better format
      auto painter = QPainter(&image);
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
         w, h,
         VK_FORMAT_R8G8B8A8_SRGB,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      content.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      content.copy_content_from_buffer(staging.handle);
      content.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      content.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      //
      // Update descriptor:
      //
      auto infos = std::array{
         VkDescriptorImageInfo{
            .sampler     = sr.texture_sampler,
            .imageView   = this->atlas_info.image.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         },
      };
      auto writes = std::array{
         VkWriteDescriptorSet{
            .sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet     = sci.descriptor_sets[descriptor_set_index],
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
      auto& vib = this->vertex_and_index_buffer;
      //
      auto  staging = sr.create_buffer(vib.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data = staging.map_memory();
      this->update_geometry(data);
      staging.unmap_memory(data);
      //
      vib.copy_from(staging);
   }
   void fps::update_geometry(void* mapped_vertex_memory) {
      auto* vertices = (_vertex*)mapped_vertex_memory;
      //
      if (this->change_flags.test<change_flag::atlas>()) {
         //
         // Initial setup for the "label" quad (i.e. "FPS: ").
         //
         const auto& glyph = this->atlas_info.label;
         _set_quad_pos({ 0, 0 }, vertices, glyph);
         _set_quad_uv(vertices, glyph);
      }
      //
      std::array<uint8_t, max_digits> prior;
      std::array<uint8_t, max_digits> after;
      {
         auto p = this->last_value;
         auto v = this->value;
         for (size_t i = 0; i < max_digits; ++i, v /= display_base, p /= display_base) {
            prior[max_digits - i - 1] = p % display_base;
            after[max_digits - i - 1] = v % display_base;
         }
         //
         // If the FPS count exceeds the maximum number that can be displayed with the digit 
         // count we have, then the number will be truncated (e.g. "1234567" -> "34567" for 
         // a digit count of 5). This is undesired; it'd be cleaner to force all digits to 
         // the highest one (i.e. nines in base-10).
         //
         if (p > max_visible_value)
            for (auto& n : prior)
               n = 9;
         if (v > max_visible_value)
            for (auto& n : after)
               n = 9;
      }
      //
      // Update all digits needing updates. Let's start with the vertex buffer.
      //
      int  x         = this->atlas_info.label.right();
      bool displaced = this->change_flags.test(change_flag::positions); // if a digit changes, and the new glyph has a different width, then it will displace all subsequent glyphs
      for (size_t i = 0; i < max_digits; ++i) {
         auto* digit_verts = &vertices[vertices_per_quad + vertices_per_quad * i];
         //
         const auto& glyph = this->atlas_info.digits[after[i]];
         if (this->change_flags.test<change_flag::atlas>() || prior[i] != after[i]) {
            if (!displaced) {
               const auto& old_glyph = this->atlas_info.digits[prior[i]];
               if (old_glyph.width() != glyph.width()) {
                  displaced = true;
               }
            }
            //
            // Update quad:
            //
            _set_quad_pos({ x, 0 }, digit_verts, glyph);
            _set_quad_uv(digit_verts, glyph);
         } else if (displaced) {
            //
            // This digit has not changed, but one to its left has, and in such a way 
            // as to require position updates for all subsequent digits.
            //
            _set_quad_x(x, digit_verts, glyph);
         }
         x += glyph.width();
         x += this->style.space_between_digits;
      }
      if (this->change_flags.test<change_flag::atlas>()) {
         //
         // Initial setup for index buffers.
         //
         auto* indices = (uint16_t*)((std::intptr_t)mapped_vertex_memory + _vib_indices_offset);
         for (size_t i = 0; i < quad_count; ++i) {
            indices[i * 6 + 0] = i;
            indices[i * 6 + 1] = i + 1;
            indices[i * 6 + 2] = i + 2;
            indices[i * 6 + 3] = i + 2;
            indices[i * 6 + 4] = i + 3;
            indices[i * 6 + 5] = i;
         }
      }
      //
      this->change_flags.reset_all_of<change_flag::atlas, change_flag::positions, change_flag::value>();
      this->last_value = this->value;
   }
   void fps::draw_call(VkCommandBuffer command_buffer) {
      VkDeviceSize offset = 0;
      //
      auto& vib = this->vertex_and_index_buffer;
      vkCmdBindVertexBuffers(command_buffer, 0, 1, &vib.handle, &offset);
      vkCmdBindIndexBuffer  (command_buffer, vib.handle, _vib_indices_offset, VK_INDEX_TYPE_UINT16);
      vkCmdDrawIndexed(command_buffer, (uint32_t)index_count, 1, 0, 0, 0);
   }
}