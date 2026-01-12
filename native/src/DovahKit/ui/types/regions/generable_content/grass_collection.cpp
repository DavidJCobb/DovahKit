#include "./grass_collection.h"
#include "dovah/forms/Region.h"

namespace ui::types::regions::generable_content {
   void grass_collection::clear() {
      this->grasses.clear();
   }
   void grass_collection::import_data(const dovah::loaded_forms::Region& src_form) {
      this->clear();

      for (auto& src_coll : src_form.generable_content) {
         auto* src_cast = src_coll.as<backend_collection_type>();
         if (!src_cast)
            continue;
         this->import_data(*src_cast);
      }
   }
   void grass_collection::import_data(const backend_collection_type& src_coll) {
      auto& src_list = src_coll.entries;
      this->grasses.reserve(this->grasses.size() + src_list.size());
      for (size_t i = 0; i < src_list.size(); ++i) {
         auto& src_item     = src_list[i];
         auto* grass        = src_item.grass.get_form_stub();
         auto* land_texture = src_item.land_texture.get_form_stub();
         if (!grass || grass->form_type != dovah::form_type::grass)
            continue;
         if (land_texture && land_texture->form_type != dovah::form_type::land_texture)
            continue;

         bool duplicate = false;
         for (auto& prior_item : this->grasses) {
            if (prior_item.grass == grass && prior_item.land_texture == land_texture) {
               duplicate = true;
               break;
            }
         }
         if (duplicate)
            continue;

         auto& dst_item = this->grasses.emplace_back();
         dst_item.grass        = grass;
         dst_item.land_texture = land_texture;
      }
   }
   void grass_collection::export_data(dovah::loaded_forms::Region& dst_form, backend_collection_type& dst_coll) const {
      dst_coll.clear(dst_form);
      for (auto& src_item : this->grasses) {
         if (!src_item.grass || !src_item.land_texture)
            continue;
         auto& dst_item = dst_coll.entries.emplace_back();
         dst_item.grass.set(dst_form, src_item.grass);
         dst_item.land_texture.set(dst_form, src_item.land_texture);
      }
   }
}