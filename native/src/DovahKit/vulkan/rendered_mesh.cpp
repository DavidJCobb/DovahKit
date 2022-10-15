#include "rendered_mesh.h"
#include "helpers/math.h"
#include "helpers/offset_into.h"
#include "nif/file.h"
#include "./command_buffer.h"
#include "./raycast.h"
#include "./surface_renderer.h"
#include "./scene_entities/owned_gpu_resource_upload_operation.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>

namespace {
   bool ray_intersects_obb(
      const glm::vec3& ray_origin,
      const glm::vec3& ray_direction,
      const glm::vec3& local_aabb_min,
      const glm::vec3& local_aabb_max,
      const glm::mat4& transform,
      float& distance
   ) {
      // http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
      glm::vec3 world_pos = transform[3];
      glm::vec3 delta     = world_pos - ray_origin;
      //
      float greatest_min = 0.0F;
      float smallest_max  = std::numeric_limits<float>::max();
      for (int i = 0; i < 3; ++i) {
         glm::vec3 axis = transform[i];
         float e = glm::dot(axis, delta);
         float f = glm::dot(ray_direction, axis);
         if (fabs(f) > std::numeric_limits<float>::epsilon()) {
            //
            // For the current axis, compute both ray/plane intersections; e.g. for the X-axis, 
            // compute the intersections with the YZ plane.
            //
            float behind = (e + local_aabb_min[i]) / f;
            float ahead  = (e + local_aabb_max[i]) / f;
            if (behind > ahead){
               std::swap(behind, ahead);
            }
            //
            if (smallest_max > ahead)
               smallest_max = ahead;
            if (greatest_min < behind)
               greatest_min = behind;
            //
            // If the nearest "far" intersection is ever closer than the nearest "near" 
            // intersection, then there is no intersection.
            //
            if (smallest_max < greatest_min)
               return false;
         } else {
            //
            // Ray is parallel to the AABB.
            //
            if (-e + local_aabb_min[i] > 0.0f || -e + local_aabb_max[i] < 0.0f)
               return false;
         }
      }
      distance = greatest_min;
      return true;
   }
}

namespace vulkanDK {
   rendered_mesh::~rendered_mesh() {
      this->reset();
   }

   rendered_mesh::rendered_mesh(rendered_mesh&& o) noexcept {
      *this = std::move(o);
   }
   rendered_mesh& rendered_mesh::operator=(rendered_mesh&& o) noexcept {
      base::operator=(std::move(o));
      {
         auto& tm = this->mesh_data;
         auto& om = o.mesh_data;
         std::swap(tm.vertices, om.vertices);
         std::swap(tm.indices,  om.indices);
         tm.bounding_sphere = om.bounding_sphere;
      }
      this->mesh_flags = o.mesh_flags;
      //
      this->owned_gpu_resources = std::move(o.owned_gpu_resources);
      this->push_params        = o.push_params;
      this->frame_drawing_data = o.frame_drawing_data;
      this->texture_indices    = o.texture_indices;
      std::swap(this->anim_state, o.anim_state);
      std::swap(this->owning_nif, o.owning_nif);
      //
      return *this;
   }

   void rendered_mesh::mark_for_delete() {
      base::_mark_for_delete<rendered_mesh>();
      this->owning_nif = nullptr;
   }
   void rendered_mesh::reset() {
      base::_reset<rendered_mesh>();
      //
      this->owning_nif = nullptr;
      if (auto*& p = this->anim_state) {
         delete p;
         p = nullptr;
      }
      this->mesh_flags = mesh_flag::all_default_flags;
      this->mesh_data.vertices.clear();
      this->mesh_data.indices.clear();
      //
      this->push_params = {};
      this->texture_indices = decltype(texture_indices)();
   }

   rendered_mesh::frame_culling_data_type rendered_mesh::calculate_frame_culling_data() const {
      cull_flags_t cull_flags = 0;
      if (this->mesh_flags & mesh_flag::culled_by_application)
         cull_flags |= cull_flag::culled_by_application;
      
      return {
         .transform = this->frame_drawing_data.transform,
         .bounding_sphere_center = this->mesh_data.bounding_sphere.center,
         .bounding_sphere_radius = sqrtf(this->mesh_data.bounding_sphere.radius_sq),
         .flags = cull_flags,
      };
   }
   

   void rendered_mesh::set_transform(const glm::mat4& in) {
      this->frame_drawing_data.transform = in;
      this->on_frame_drawing_data_changed();
   }

   // Setup functions:
   void rendered_mesh::recalc_bounding_sphere() {
      if (this->mesh_data.vertices.empty()) {
         this->mesh_data.bounding_sphere = {
            .center    = { 0, 0, 0 },
            .radius_sq = 0,
         };
         return;
      }
      //
      struct range {
         float min = std::numeric_limits<float>::max();
         float max = std::numeric_limits<float>::min();
         //
         void consider(float v) {
            if (v < min)
               min = v;
            if (v > max)
               max = v;
         }
         float center() const noexcept {
            return (max + min) / 2;
         }
      };
      range x;
      range y;
      range z;
      for (auto& v : this->mesh_data.vertices) {
         x.consider(v.pos.x);
         y.consider(v.pos.y);
         z.consider(v.pos.z);
      }
      this->mesh_data.bounding_box = {
         .min = { x.min, y.min, z.min },
         .max = { x.max, y.max, z.max },
      };
      this->mesh_data.bounding_sphere = {
         .center    = { x.center(), y.center(), z.center() },
         .radius_sq = 0.0,
      };
      //
      auto& bs = this->mesh_data.bounding_sphere;
      for (auto& v : this->mesh_data.vertices) {
         float radius_sq = glm::distance2(bs.center, v.pos);
         bs.radius_sq = std::max(bs.radius_sq, radius_sq);
      }
      this->frame_drawing_data.bounding_sphere_center = bs.center;
      this->frame_drawing_data.bounding_sphere_radius = sqrt(bs.radius_sq);
   }
   //
   size_t rendered_mesh::total_size_for_setup() const {
      return (sizeof(vertex) * this->mesh_data.vertices.size()) + this->mesh_data.indices.size_in_bytes();
   }
   void rendered_mesh::sizes_for_setup(VkDeviceSize& v, VkDeviceSize& i, VkDeviceSize& total) const {
      v = this->mesh_data.vertices.size() * sizeof(vertex);
      i = this->mesh_data.indices.size_in_bytes();
      total = v + i;
   }
   void rendered_mesh::setup_vib_data_at(void* dest) const {
      auto& vl = this->mesh_data.vertices;
      auto& il = this->mesh_data.indices;
      //
      auto vs = vl.size() * sizeof(vertex);
      //
      memcpy((void*)((std::intptr_t)dest),      vl.data(), vs);
      memcpy((void*)((std::intptr_t)dest + vs), il.data(), il.size_in_bytes());
   }

   #pragma region Member functions for owned GPU resources (esp. for uploading)
   VkDeviceSize rendered_mesh::owned_gpu_resources_size() const noexcept {
      return (sizeof(vertex) * this->mesh_data.vertices.size()) + this->mesh_data.indices.size_in_bytes();
   }
   void rendered_mesh::upload_owned_gpu_resources(scene_entities::owned_gpu_resource_upload_operation& upload) {
      const auto&  list_v = this->mesh_data.vertices;
      const auto&  list_i = this->mesh_data.indices;
      VkDeviceSize size_v = this->mesh_data.vertices.size() * sizeof(vertex);
      VkDeviceSize size_i = this->mesh_data.indices.size_in_bytes();
      VkDeviceSize buffer_size = size_v + size_i;

      upload.stage_data(list_v.data(), size_v);
      upload.stage_data(list_i.data(), size_i);
      
      auto& vib = this->vib();
      vib.wide_indices = this->mesh_data.indices.type() == vertex_index_list::value_type::wide;
      vib.indices_at   = this->mesh_data.vertices.size() * sizeof(vertex);
      vib.index_count  = this->mesh_data.indices.size();
      vib.buffer       = upload.create_buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      upload.queue_upload_to_buffer(vib.buffer);
   }
   #pragma endregion

   void rendered_mesh::draw_call(VkCommandBuffer command_buffer) {
      VkDeviceSize offset = 0;
      //
      auto& vib = this->owned_gpu_resources.current;
      //
      if (!this->active() || this->pending_gpu_upload())
         return;
      //
      vkCmdBindVertexBuffers(command_buffer, 0, 1, &vib.buffer.handle, &offset);
      if (vib.wide_indices) {
         vkCmdBindIndexBuffer(command_buffer, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT32);
      } else {
         vkCmdBindIndexBuffer(command_buffer, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT16);
      }
      vkCmdDrawIndexed(command_buffer, (uint32_t)vib.index_count, 1, 0, 0, 0);
   }
   VkDrawIndexedIndirectCommand rendered_mesh::make_indirect_draw_command() const {
      return VkDrawIndexedIndirectCommand{
         .indexCount    = this->owned_gpu_resources.current.index_count,
         .instanceCount = 1,
         .firstIndex    = 0,
         .vertexOffset  = 0,
         .firstInstance = 0,
      };
   }

   // geometry
   bool rendered_mesh::ray_intersects_bounding_sphere(const cobb::vector3<float>& ray_origin, cobb::vector3<float> ray_direction) const {
      constexpr float epsilon = 0.0001;
      auto& bound  = this->mesh_data.bounding_sphere;
      auto  center = glm::vec3(this->transform() * glm::vec4(bound.center, 1.0F));
      auto  scale  = glm::length(this->transform()[0]); // X-scale; Y-scale would be the length of column 1; Z-scale, column 2
      //
      // Line/sphere intersection check:
      //
      ray_direction.normalize();
      //
      auto gap   = ray_origin - center;
      auto delta = std::pow(ray_direction.dot(gap), 2) - (gap.length_sq() - (bound.radius_sq * scale));
      //
      // Cases:
      // 
      //  - delta < -epsilon
      //     - No intersection
      //  - delta >= -epsilon && delta <= epsilon
      //     - One intersection (ray scrapes sphere's surface)
      //  - delta > epsilon
      //     - Two intersections (ray penetrates and exits sphere)
      //
      return (delta >= -epsilon);
   }
   bool rendered_mesh::ray_intersects_shape(const glm::vec3& ray_origin, glm::vec3 ray_direction, float& hit_distance) const {
      ray_direction = glm::normalize(ray_direction);
      glm::vec2 bary_position;
      //
      auto& list = this->mesh_data.indices;
      auto& vert = this->mesh_data.vertices;
      bool  hits = false;
      hit_distance = std::numeric_limits<float>::max();
      for (size_t i = 0; i + 2 < list.size(); i += 3) {
         std::array<uint32_t, 3> indices = list.triangle_from(i);
         glm::vec3 a = this->transform() * glm::vec4(vert[indices[0]].pos, 1.0F);
         glm::vec3 b = this->transform() * glm::vec4(vert[indices[1]].pos, 1.0F);
         glm::vec3 c = this->transform() * glm::vec4(vert[indices[2]].pos, 1.0F);
         //
         float distance;
         bool  result = glm::intersectRayTriangle(
            ray_origin,
            ray_direction,
            a,
            b,
            c,
            bary_position,
            distance
         );
         if (result) {
            if (distance < 0) {
               //
               // So GLM didn't  manage to  implement  a ray/triangle  intersection check 
               // properly. That's fun.
               // 
               // Specifically, it can return a true result but with a negative distance, 
               // which means that the surface is actually BEHIND the ray, which means it 
               // isn't the hit position and indeed, there wasn't a hit.
               //
               continue;
            }
            hits         = result;
            hit_distance = std::min(distance, hit_distance);
         }
      }
      return hits;
   }
   bool rendered_mesh::ray_intersects(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& hit_distance) const {
      if (this->empty())
         return false;
      if (!this->ray_intersects_bounding_sphere(ray_origin, ray_direction))
         return false;
      if (this->mesh_data.bounding_sphere.radius_sq > (10000 * 10000)) {
         //
         // Massive triangles can cause ray/triangle intersection checks to behave 
         // erratically and produce both false positives and false negatives. Let's 
         // be a little more certain before we resort to trying them.
         //
         if (!ray_intersects_obb(ray_origin, ray_direction, this->mesh_data.bounding_box.min, this->mesh_data.bounding_box.max, this->transform(), hit_distance))
            return false;
      }
      return this->ray_intersects_shape(ray_origin, ray_direction, hit_distance);
   }

   raycast_hit_data rendered_mesh::do_raycast(const raycast& rc) const {
      if (!this->active() || this->pending_gpu_upload())
         return {};

      if (!this->ray_intersects_bounding_sphere(rc.origin, rc.direction))
         return {};
      if (this->mesh_data.bounding_sphere.radius_sq > (10000 * 10000)) {
         //
         // Massive triangles can cause ray/triangle intersection checks to behave 
         // erratically and produce both false positives and false negatives. Let's 
         // be a little more certain before we resort to trying them.
         //
         float hit_distance;
         if (!ray_intersects_obb(rc.origin, rc.direction, this->mesh_data.bounding_box.min, this->mesh_data.bounding_box.max, this->transform(), hit_distance))
            return {};
      }

      raycast_hit_data hit = {};

      auto ray_direction = glm::normalize(rc.direction);

      auto& list = this->mesh_data.indices;
      auto& vert = this->mesh_data.vertices;
      for (size_t i = 0; i + 2 < list.size(); i += 3) {
         std::array<uint32_t, 3> indices = list.triangle_from(i);
         glm::vec3 a = this->transform() * glm::vec4(vert[indices[0]].pos, 1.0F);
         glm::vec3 b = this->transform() * glm::vec4(vert[indices[1]].pos, 1.0F);
         glm::vec3 c = this->transform() * glm::vec4(vert[indices[2]].pos, 1.0F);
         //
         glm::vec2 bary_position;
         float     distance;
         //
         bool result = glm::intersectRayTriangle(
            rc.origin,
            ray_direction,
            a,
            b,
            c,
            bary_position,
            distance
         );
         if (!result || distance < 0)
            continue;
         if (distance >= hit.distance)
            continue;
         hit.bary_position  = bary_position;
         hit.distance       = distance;
         hit.position       = bary_position.x * a + bary_position.y * b + (1.0F - bary_position.x - bary_position.y) * c;
         hit.surface_normal = glm::triangleNormal(a, b, c);
      }

      return hit;
   }
}