#include "file.h"
#include <intrin.h>
#include "helpers/glm/mat4_by_vec3_simd.h"
#include "helpers/cpuinfo.h"
//
#include "notice_code_list.h"
#include "reader.h"
//
#include "block.h"
#include "blocks/_factory.h"
#include "blocks/NiNode.h"
//
#include "blocks/BSLightingShaderProperty.h"
#include "blocks/BSShaderTextureSet.h"
#include "blocks/BSTriShape.h"
#include "blocks/NiGeometry.h"
#include "blocks/NiGeometryData.h"
//
#include "dovah/forms/components/model.h"
#include "dovah/forms/TextureSet.h"

namespace nifDK {
   /*static*/ file_version file_version::from_string(const std::string& s) {
      //
      // R"-(\S+ File Format \d+\.\d+\.\d+\.\d+)-"
      //
      bool   space = false;
      size_t size  = s.size();
      size_t i     = 0;
      for (; i < size; ++i) {
         auto c = s[i];
         if (c == ' ') {
            space = true;
            break;
         }
      }
      if (!space)
         return file_version(0); // failed: no space
      //
      if (s.size() - i <= 12)
         return file_version(0); // failed: not enough room for "File Format " and additional content, after the space
      if (s.compare(i, 12, "File Format ") != 0)
         return file_version(0); // failed: "File Format " not present after the space
      i += 12;
      //
      int      index  = 0;
      int      digits = 0;
      int      part   = 0;
      uint32_t out    = 0;
      for (; i < size; ++i) {
         auto c = s[i];
         if (c == '.') {
            if (part > 255)
               return file_version(0); // failed: a version part is too high (e.g. "256.0.0.0")
            out |= part << (0x08 * index);
            //
            ++index;
            part   = 0;
            digits = 0;
            if (index > 3)
               return file_version(0); // failed: too many '.' (e.g. "1.2.3.4.")
            continue;
         }
         if (c >= '0' && c <= '9') {
            ++digits;
            part = (part * 10) + (c - '0');
            continue;
         }
         return file_version(0); // failed: invalid character
      }
      if (index != 3)
         return file_version(0); // failed: wrong number of parts (e.g. "2.1.0")
      return file_version(out);
   }
   //
   void file_version::read(file_reader& reader) {
      reader.read<std::endian::little>(this->value);
   }
   void file_version::unchecked_read(file_reader& reader) {
      reader.unchecked_read<std::endian::little>(this->value);
   }

   file::~file() {
      this->root_node = nullptr;
      for (auto* b : this->all_blocks)
         if (b)
            delete b;
      this->all_blocks.clear();
   }

   void file::read(void* data, size_t size) {
      auto reader = file_reader(this, data, size);
      //
      uint32_t block_count;
      uint16_t block_type_count;
      std::vector<std::string> block_type_names;
      std::vector<uint16_t> block_type_indices;
      std::vector<uint32_t> block_sizes;
      uint32_t string_count;
      uint32_t max_string_length;
      std::vector<std::string> all_strings;
      uint32_t group_count;
      //
      try {
         reader.read_line_string(this->header.format_name);
         {
            auto version = file_version::from_string(this->header.format_name);
            if (version && version <= file_version::from_parts<3, 1, 0, 0>) {
               std::string copyright; // discard
               reader.read_line_string(copyright);
            }
         }
         reader.read(this->header.version);
         reader.read_endianness(file_reader::file_passkey());
         reader.read<std::endian::little>(this->header.user_versions.primary);
         reader.read<std::endian::little>(block_count);
         reader.read<std::endian::little>(this->header.user_versions.secondary);
         reader.read_prefixed_string<uint8_t>(this->header.export_data.creator);
         reader.read_prefixed_string<uint8_t>(this->header.export_data.info[0]);
         reader.read_prefixed_string<uint8_t>(this->header.export_data.info[1]);
         //
         reader.read(block_type_count);
         block_type_names.resize(block_type_count);
         for (uint16_t i = 0; i < block_type_count; ++i) {
            reader.read_prefixed_string<uint32_t>(block_type_names[i]);
         }
         //
         block_type_indices.resize(block_count);
         block_sizes.resize(block_count);
         for (auto& item : block_type_indices)
            reader.read(item);
         for (auto& item : block_sizes)
            reader.read(item);
         //
         reader.read_string_table(file_reader::file_passkey());
         //
         reader.read(group_count);
         if (group_count) {
            //
            // TODO
            //
         }
         //
         // End of header!
         //
         this->all_blocks.resize(block_count);
         for (size_t i = 0; i < block_count; ++i) {
            auto tni = block_type_indices[i];
            if (tni >= block_type_names.size()) {
               reader.throw_error(detailed_notice{
                  .code  = notice_code::bad_block_typename_index,
                  .flags = detailed_notice::flag::has_cause_block,
                  .cause = {
                     .block = {
                        .index = (int32_t)i,
                     },
                  },
               });
            }
            const auto& tn = block_type_names[tni];
            auto* b = this->all_blocks[i] = create_block_of_type(tn);
            if (!b) {
               //
               // This typename is an abstract NetImmerse class; we can't create it at run-time (we may have implemented 
               // it as an abstract class) and it shouldn't appear in files.
               //
               reader.throw_error(detailed_notice{
                  .code  = notice_code::block_is_abstract_typename,
                  .flags = detailed_notice::flag::has_cause_block,
                  .cause = {
                     .block = {
                        .index = (int32_t)i,
                     },
                     .block_type = tn,
                  },
               });
            }
            b->owner = this;
         }
         size_t base = reader.position();
         size_t offset = 0;
         for (size_t i = 0; i < block_count; ++i) {
            auto* b = this->all_blocks[i];
            assert(b);
            const auto& tn = block_type_names[block_type_indices[i]];
            auto guard = reader.enter_block(file_reader::file_passkey(), i, block_sizes[i], tn);
            b->parse(reader);
         }
         //
         // Validity checks for node trees:
         //
         {
            auto& list = this->all_blocks;
            auto  size = list.size();
            //
            block_types::NiNode* root = nullptr;
            for (size_t i = 0; i < size; ++i) {
               auto* node = dynamic_cast<block_types::NiNode*>(this->all_blocks[i]);
               if (!node)
                  continue;
               if (node->parent)
                  continue;
               if (root) {
                  reader.throw_error(detailed_notice{
                     .code  = notice_code::multiple_top_level_nodes,
                     .flags = detailed_notice::flag::has_cause_block,
                     .cause = {
                        .block = {
                           .index = (int32_t)i,
                           .name  = node->name,
                        },
                     },
                  });
               }
               root = node;
            }
            this->root_node = root;
            //
            // Got the root, if any, and we know there are no other node trees. Now let's make sure the tree isn't cyclical.
            //
            if (root) {
               std::vector<block_types::NiNode*> seen;
               root->for_self_and_subtree([this, &seen, &reader](block_types::NiNode* node) {
                  bool already_seen = (std::find(seen.begin(), seen.end(), node) != seen.end());
                  if (!already_seen)
                     return;
                  //
                  int32_t block_index = -1;
                  for (size_t i = 0; i < this->all_blocks.size(); ++i) {
                     if (this->all_blocks[i] == node) {
                        block_index = (int32_t)i;
                        break;
                     }
                  }
                  assert(block_index >= 0);
                  reader.throw_error(detailed_notice{
                     .code  = notice_code::cyclical_node_tree,
                     .flags = detailed_notice::flag::has_cause_block,
                     .cause = {
                        .block = {
                           .index = block_index,
                           .name  = node->name,
                        },
                     },
                  });
               });
            }
         }
      } catch (file_reader::read_error& e) {
         this->results.error = reader.error_details();
         //
         // TODO: report and handle the error
         //
         #if _DEBUG
            __debugbreak();
         #endif
      }
      //
      // I think we're done, at this point
      //
      if (this->results.error.empty()) {
         this->recalc_bounds();
      }
   }

   block_types::NiObjectNET* file::block_by_name(const std::string& name) const {
      for (auto* b : this->all_blocks) {
         auto* net = dynamic_cast<nifDK::block_types::NiObjectNET*>(b);
         if (!net)
            continue;
         if (net->name == name)
            return net;
      }
      return nullptr;
   }

   void file::apply_texture_swaps(const dovah::loaded_forms::components::model_ts& defs) {
      for (const auto& entry : defs.texture_swaps) {
         if (!entry.texture_set)
            continue;
         auto* block = this->block_by_name(entry.nif_block_name);
         if (!block)
            continue;
         //
         auto txst = entry.texture_set.get_form_stub()->load().ptr_cast<dovah::loaded_forms::TextureSet>();
         if (!txst)
            continue;
         //
         block_types::BSShaderProperty* shader = nullptr;
         if (auto* data = dynamic_cast<nifDK::block_types::NiGeometry*>(block)) {
            shader = data->properties.shader;
         } else if (auto* data = dynamic_cast<nifDK::block_types::BSTriShape*>(block)) {
            shader = data->properties.shader;
         }
         if (!shader)
            continue;
         //
         if (auto* bslp = dynamic_cast<nifDK::block_types::BSLightingShaderProperty*>(shader)) {
            if (auto* paths = bslp->texture.paths) {
               for (size_t i = 0; i < paths->textures.list.size(); ++i)
                  paths->textures.list[i] = std::string("textures\\") + txst->textures.list[i];
            }
            continue;
         }
      }
   }
   void file::recalc_bounds() {
      this->bounds = {};
      if (!this->root_node)
         return;
      //
      glm::mat4 transform = glm::mat4(1);
      //
      if (auto& cpuinfo = cobb::cpuinfo::get(); cpuinfo.extension_support.sse_1) {
         //
         // SIMD intrinsics to process entire vertices at once.
         //
         this->root_node->walk_tree(
            transform,
            [](nifDK::block_types::NiNode* node, glm::mat4& transform) {
               transform = transform * node->transform.to_matrix();
            },
            [this](nifDK::block_types::NiAVObject* object, glm::mat4 transform) {
               if (auto* geom = dynamic_cast<nifDK::block_types::NiGeometry*>(object)) {
                  auto* data = dynamic_cast<nifDK::block_types::NiGeometryData*>(geom->data);
                  if (!data)
                     return;
                  transform = transform * geom->transform.to_matrix();
                  //
                  __m128 bmin = _mm_load_ps(this->bounds.min.list.data());
                  __m128 bmax = _mm_load_ps(this->bounds.max.list.data());
                  auto& list = data->vertices;
                  auto  size = list.size();
                  //
                  size_t i = 0;
                  for (; i < size; ++i) {
                     auto v = cobb::glm::mat4_by_vec3_simd(transform, list[i]);
                     __m128 vert = _mm_load_ps(&v.x);
                     bmin = _mm_min_ps(bmin, vert);
                     bmax = _mm_max_ps(bmax, vert);
                  }
                  _mm_store_ps(this->bounds.min.list.data(), bmin);
                  _mm_store_ps(this->bounds.max.list.data(), bmax);
                  //
                  return;
               }
               if (auto* data = dynamic_cast<nifDK::block_types::BSTriShape*>(object)) {
                  transform = transform * data->transform.to_matrix();
                  //
                  __m128 bmin = _mm_load_ps(this->bounds.min.list.data());
                  __m128 bmax = _mm_load_ps(this->bounds.max.list.data());
                  auto& list = data->vertices;
                  auto  size = list.size();
                  //
                  size_t i = 0;
                  for (; i < size; ++i) {
                     auto v = cobb::glm::mat4_by_vec3_simd(transform, list[i].vertex);
                     __m128 vert = _mm_load_ps(&v.x);
                     bmin = _mm_min_ps(bmin, vert);
                     bmax = _mm_max_ps(bmax, vert);
                  }
                  _mm_store_ps(this->bounds.min.list.data(), bmin);
                  _mm_store_ps(this->bounds.max.list.data(), bmax);
                  //
                  return;
               }
            }
         );
      } else {
         this->root_node->walk_tree(
            transform,
            [](nifDK::block_types::NiNode* node, glm::mat4& transform) {
               transform = transform * node->transform.to_matrix();
            },
            [this](nifDK::block_types::NiAVObject* object, const glm::mat4& transform) {
               if (auto* geom = dynamic_cast<nifDK::block_types::NiGeometry*>(object)) {
                  auto* data = dynamic_cast<nifDK::block_types::NiGeometryData*>(geom->data);
                  if (!data)
                     return;
                  auto& bmin = this->bounds.min;
                  auto& bmax = this->bounds.max;
                  for (glm::fvec3 v : data->vertices) {
                     v = transform * glm::fvec4(v, 1);
                     bmin.x = std::min(bmin.x, v.x);
                     bmin.y = std::min(bmin.y, v.y);
                     bmin.z = std::min(bmin.z, v.z);
                     bmax.x = std::max(bmax.x, v.x);
                     bmax.y = std::max(bmax.y, v.y);
                     bmax.z = std::max(bmax.z, v.z);
                  }
               }
               if (auto* geom = dynamic_cast<nifDK::block_types::BSTriShape*>(object)) {
                  auto& bmin = this->bounds.min;
                  auto& bmax = this->bounds.max;
                  for (auto& vert : geom->vertices) {
                     auto v = transform * glm::fvec4(vert.vertex, 1);
                     bmin.x = std::min(bmin.x, v.x);
                     bmin.y = std::min(bmin.y, v.y);
                     bmin.z = std::min(bmin.z, v.z);
                     bmax.x = std::max(bmax.x, v.x);
                     bmax.y = std::max(bmax.y, v.y);
                     bmax.z = std::max(bmax.z, v.z);
                  }
               }
            }
         );
         // End of non-intrinsic branch.
      }
   }
   void file::sever_connection_to(vulkanDK::rendered_mesh_handle handle) {
      for (auto* b : this->all_blocks) {
         b->sever_connection_to_vulkan_mesh(handle);
      }
   }
}