#include "rendered_mesh.h"
#include "../helpers/math.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/norm.hpp>

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
      {
         auto& tm = this->data;
         auto& om = o.data;
         std::swap(tm.vertices, om.vertices);
         std::swap(tm.indices,  om.indices);
         tm.bounding_sphere = om.bounding_sphere;
      }
      {
         auto& tm = this->vertex_and_index_buffer;
         auto& om = o.vertex_and_index_buffer;
         std::swap(tm.buffer,       om.buffer);
         std::swap(tm.indices_at,   om.indices_at);
         std::swap(tm.index_count,  om.index_count);
         std::swap(tm.wide_indices, om.wide_indices);
      }
      this->mesh_flags      = o.mesh_flags;
      this->push_params     = o.push_params;
      this->shader_params   = o.shader_params;
      this->texture_indices = o.texture_indices;
      this->handled_frames  = o.handled_frames;
      std::swap(this->life_state, o.life_state);
      std::swap(this->anim_state, o.anim_state);
      //
      return *this;
   }

   void rendered_mesh::_on_shader_parameter_change() {
      if (this->pending_delete())
         return;
      this->handled_frames.set_all_out_of_date();
   }
   //
   void rendered_mesh::set_transform(const glm::mat4& in) {
      this->shader_params.transform = in;
      this->_on_shader_parameter_change();
   }

   // Setup functions:
   void rendered_mesh::recalc_bounding_sphere() {
      if (this->data.vertices.empty()) {
         this->data.bounding_sphere = {
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
      for (auto& v : this->data.vertices) {
         x.consider(v.pos.x);
         y.consider(v.pos.y);
         z.consider(v.pos.z);
      }
      this->data.bounding_box = {
         .min = { x.min, y.min, z.min },
         .max = { x.max, y.max, z.max },
      };
      this->data.bounding_sphere = {
         .center    = { x.center(), y.center(), z.center() },
         .radius_sq = 0.0,
      };
      //
      auto& bs = this->data.bounding_sphere;
      for (auto& v : this->data.vertices) {
         float radius_sq = glm::distance2(bs.center, v.pos);
         bs.radius_sq = std::max(bs.radius_sq, radius_sq);
      }
      this->shader_params.bounding_sphere_center = bs.center;
      this->shader_params.bounding_sphere_radius = sqrt(bs.radius_sq);
   }
   //
   size_t rendered_mesh::total_size_for_setup() const {
      return (sizeof(vertex) * this->data.vertices.size()) + this->data.indices.size_in_bytes();
   }
   void rendered_mesh::sizes_for_setup(VkDeviceSize& v, VkDeviceSize& i, VkDeviceSize& total) const {
      v = this->data.vertices.size() * sizeof(vertex);
      i = this->data.indices.size_in_bytes();
      total = v + i;
   }
   void rendered_mesh::setup_vib_data_at(void* dest) const {
      auto& vl = this->data.vertices;
      auto& il = this->data.indices;
      //
      auto vs = vl.size() * sizeof(vertex);
      //
      memcpy((void*)((std::intptr_t)dest),      vl.data(), vs);
      memcpy((void*)((std::intptr_t)dest + vs), il.data(), il.size_in_bytes());
   }

   void rendered_mesh::draw_call(VkCommandBuffer command_buffer) {
      VkDeviceSize offset = 0;
      //
      auto& vib = this->vertex_and_index_buffer;
      //
      if (!this->active())
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
         .indexCount    = this->vertex_and_index_buffer.index_count,
         .instanceCount = 1,
         .firstIndex    = 0,
         .vertexOffset  = 0,
         .firstInstance = 0,
      };
   }

   void rendered_mesh::mark_for_delete() {
      this->life_state = scene_frame_item_state::pending_delete;
      this->handled_frames.set_all_out_of_date();
   }
   void rendered_mesh::reset() {
      auto& vib = this->vertex_and_index_buffer;
      vib.buffer       = buffer();
      vib.index_count  = 0;
      vib.indices_at   = 0;
      vib.wide_indices = false;
      //
      if (auto*& p = this->anim_state) {
         delete p;
         p = nullptr;
      }
      this->push_params   = {};
      this->shader_params = {};
      this->texture_indices = decltype(texture_indices)();
      this->handled_frames = frame_dirty_state();
      this->life_state = scene_frame_item_state::empty;
      //
      this->data.vertices.clear();
      this->data.indices.clear();
   }

   // geometry
   bool rendered_mesh::ray_intersects_bounding_sphere(const cobb::vector3<float>& ray_origin, cobb::vector3<float> ray_direction) const {
      constexpr float epsilon = 0.0001;
      auto& bound  = this->data.bounding_sphere;
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
      auto& list = this->data.indices;
      auto& vert = this->data.vertices;
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
      if (this->data.bounding_sphere.radius_sq > (10000 * 10000)) {
         //
         // Massive triangles can cause ray/triangle intersection checks to behave 
         // erratically and produce both false positives and false negatives. Let's 
         // be a little more certain before we resort to trying them.
         //
         if (!ray_intersects_obb(ray_origin, ray_direction, this->data.bounding_box.min, this->data.bounding_box.max, this->transform(), hit_distance))
            return false;
      }
      return this->ray_intersects_shape(ray_origin, ray_direction, hit_distance);
   }
}