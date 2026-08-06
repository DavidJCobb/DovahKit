#include "./object_bounds_from_nif.h"
#include <limits>
#include <glm/glm.hpp>
#include "nif/blocks/BSBound.h"
#include "nif/blocks/BSTriShape.h"
#include "nif/blocks/NiTriShape.h"
#include "nif/blocks/NiTriShapeData.h"
#include "nif/blocks/NiNode.h"

namespace {
   using object_bounds_float = dovah::utils::object_bounds_float;
}

static object_bounds_float _from_geometry(const nifDK::block_types::NiTriShape& shape) {
   object_bounds_float result;
   if (shape.data) {
      for (const auto& vert : shape.data->vertices) {
         result.min.x = std::min(result.min.x, vert.x);
         result.min.y = std::min(result.min.y, vert.y);
         result.min.z = std::min(result.min.z, vert.z);
         result.max.x = std::max(result.max.x, vert.x);
         result.max.y = std::max(result.max.y, vert.y);
         result.max.z = std::max(result.max.z, vert.z);
      }
   }
   return result;
}
static object_bounds_float _from_geometry(const nifDK::block_types::BSTriShape& shape) {
   object_bounds_float result;
   for (const auto& vert : shape.vertices) {
      result.min.x = std::min(result.min.x, vert.vertex.x);
      result.min.y = std::min(result.min.y, vert.vertex.y);
      result.min.z = std::min(result.min.z, vert.vertex.z);
      result.max.x = std::max(result.max.x, vert.vertex.x);
      result.max.y = std::max(result.max.y, vert.vertex.y);
      result.max.z = std::max(result.max.z, vert.vertex.z);
   }
   return result;
}

static void _from_object(object_bounds_float& dst, const nifDK::block& block) {
   if (auto* avo = dynamic_cast<const nifDK::block_types::NiAVObject*>(&block)) {
      auto this_transform = avo->transform.to_matrix();

      object_bounds_float bounds;
      if (auto* casted = dynamic_cast<const nifDK::block_types::NiTriShape*>(&block)) {
         bounds = _from_geometry(*casted);
      } else if (auto* casted = dynamic_cast<const nifDK::block_types::BSTriShape*>(&block)) {
         bounds = _from_geometry(*casted);
      } else if (auto* casted = dynamic_cast<const nifDK::block_types::NiNode*>(&block)) {
         for (auto* child : casted->children) {
            _from_object(bounds, *child);
         }
      }
      if (!bounds.is_undefined()) {
         bounds.min = this_transform * glm::fvec4(bounds.min, 1.0F);
         bounds.max = this_transform * glm::fvec4(bounds.max, 1.0F);

         dst.min.x = std::min(dst.min.x, bounds.min.x);
         dst.min.y = std::min(dst.min.y, bounds.min.y);
         dst.min.z = std::min(dst.min.z, bounds.min.z);
         dst.max.x = std::max(dst.max.x, bounds.max.x);
         dst.max.y = std::max(dst.max.y, bounds.max.y);
         dst.max.z = std::max(dst.max.z, bounds.max.z);
      }
   }
}

namespace dovah::utils {
   extern object_bounds_float object_bounds_from_nif(const nifDK::block& block) {
      object_bounds_float bounds;
      if (auto* avo = dynamic_cast<const nifDK::block_types::NiAVObject*>(&block)) {
         if (auto* bbx = dynamic_cast<const nifDK::block_types::BSBound*>(avo->get_extra_data("BBX"))) {
            bounds.min = bbx->center - bbx->halfwidths;
            bounds.max = bbx->center + bbx->halfwidths;
            return bounds;
         }
      }
      _from_object(bounds, block);
      //
      // TODO: If the NIF doesn't contain a NiGeometry with at least one vertex, but does 
      //       contain a BSDynamicTriShape, then set the bounds to min/max i.e. -32768 min 
      //       and 32767 max.
      //
      return bounds;
   }
}