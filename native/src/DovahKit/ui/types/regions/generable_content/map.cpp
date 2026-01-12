#include "./map.h"
#include "dovah/forms/Region.h"
#include "editor/core.h"

namespace ui::types::regions::generable_content {
   void map::clear() {
      this->name.clear();
   }
   void map::import_data(const dovah::loaded_forms::Region& src_form) {
      this->clear();

      auto& editor = DovahKitCore::get();
      for (auto& src_coll : src_form.generable_content) {
         auto* src_cast = src_coll.as<backend_collection_type>();
         if (!src_cast)
            continue;
         this->name = editor.convert_localized_string(src_cast->name);
      }
   }
   void map::import_data(const backend_collection_type& src_coll) {
      this->name = DovahKitCore::get().convert_localized_string(src_coll.name);
   }
   void map::export_data(dovah::loaded_forms::Region& dst_form, backend_collection_type& dst_coll) const {
      auto& editor = DovahKitCore::get();
      editor.assign_localized_string(dst_coll.name, this->name);
   }
}