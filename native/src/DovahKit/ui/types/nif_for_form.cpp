#include "./nif_for_form.h"
#include "dovah/forms/components/model.h"

namespace ui::types {
   void nif_for_form::initializeFrom(const dovah::loaded_forms::components::model& src) {
      this->model_path             = src.model_path;
      this->supports_texture_swaps = src.supports_texture_swaps;
      if (src.supports_texture_swaps) {
         const auto& ts = static_cast<const dovah::loaded_forms::components::model_ts&>(src);
         for (auto& item : ts.texture_swaps) {
            auto& dst = this->texture_swaps.emplace_back();
            dst.block_name  = item.nif_block_name;
            dst.leaf_index  = item.nif_leaf_index;
            dst.texture_set = item.texture_set.get_form_stub();
         }
      }
   }
   void nif_for_form::commitTo(dovah::loaded_forms::components::model& dst, dovah::loaded_forms::Form& dst_owner) {
      dst.model_path     = this->model_path;
      dst.precached_info = this->precached_nif_info;
      if (dst.supports_texture_swaps) {
         auto& ts = static_cast<dovah::loaded_forms::components::model_ts&>(dst);

         for (auto& item : ts.texture_swaps) {
            item.texture_set.set(dst_owner, nullptr);
         }
         ts.texture_swaps.clear();

         for (auto& src : this->texture_swaps) {
            if (!src.texture_set)
               continue;
            auto& dst = ts.texture_swaps.emplace_back();
            dst.nif_block_name = src.block_name;
            dst.nif_leaf_index = src.leaf_index;
            dst.texture_set.set(dst_owner, src.texture_set);
         }
      }
   }
}