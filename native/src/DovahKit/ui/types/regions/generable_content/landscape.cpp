#include "./landscape.h"
#include "dovah/forms/Region.h"

namespace ui::types::regions::generable_content {
   void landscape::clear() {
      this->texture = {};
   }
   void landscape::import_data(const dovah::loaded_forms::Region& src_form) {
      this->clear();

      for (auto& src_coll : src_form.generable_content) {
         auto* src_cast = src_coll.as<backend_collection_type>();
         if (!src_cast)
            continue;
         this->import_data(*src_cast);
      }
   }
   void landscape::import_data(const backend_collection_type& src_coll) {
      this->texture = ui::types::game_file_path(src_coll.texture.c_str());
   }
   void landscape::export_data(dovah::loaded_forms::Region& dst_form, backend_collection_type& dst_coll) const {
      dst_coll.texture = this->texture.to_string().toStdString();
   }
}