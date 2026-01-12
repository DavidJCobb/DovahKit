#include "./weather_collection.h"
#include "dovah/forms/Region.h"

namespace ui::types::regions::generable_content {
   void weather_collection::clear() {
      this->weathers.clear();
   }
   void weather_collection::import_data(const dovah::loaded_forms::Region& src_form) {
      this->clear();

      for (auto& src_coll : src_form.generable_content) {
         auto* src_cast = src_coll.as<backend_collection_type>();
         if (!src_cast)
            continue;
         this->import_data(*src_cast);
      }
   }
   void weather_collection::import_data(const backend_collection_type& src_coll) {
      auto& src_list = src_coll.weathers;
      this->weathers.reserve(this->weathers.size() + src_list.size());
      for (size_t i = 0; i < src_list.size(); ++i) {
         auto& src_item = src_list[i];
         auto* weather  = src_item.weather.get_form_stub();
         if (!weather || weather->form_type != dovah::form_type::weather)
            continue;
         auto* global = src_item.global.get_form_stub();
         if (global && global->form_type != dovah::form_type::global)
            global = nullptr;

         auto prior = this->index_of(*weather);
         if (prior != index_of_none) {
            auto& dst_item = this->weathers[prior];
            dst_item.chance_constant = src_item.chance;
            dst_item.chance_global   = global;
            continue;
         }
         auto& dst_item = this->weathers.emplace_back();
         dst_item.weather         = weather;
         dst_item.chance_constant = src_item.chance;
         dst_item.chance_global   = global;
      }
   }
   void weather_collection::export_data(dovah::loaded_forms::Region& dst_form, backend_collection_type& dst_coll) const {
      dst_coll.clear(dst_form);
      for (auto& src_item : this->weathers) {
         if (!src_item.weather)
            continue;
         auto& dst_item = dst_coll.weathers.emplace_back();
         dst_item.weather.set(dst_form, src_item.weather);
         dst_item.chance = src_item.chance_constant;
         dst_item.global.set(dst_form, src_item.chance_global);
      }
   }
}