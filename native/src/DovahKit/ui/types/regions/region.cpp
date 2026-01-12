#include "./region.h"
#include "dovah/forms/Region.h"
#include "editor/core.h"

// Given DO(name, enum_name, ...):
#define FOR_EACH_GENERABLE_TYPE(DO) \
   DO(audio,     sound) \
   DO(grass,     grass) \
   DO(landscape, landscape) \
   DO(map,       map) \
   DO(objects,   objects) \
   DO(weather,   weather)

namespace ui::types::regions {
   void region::import_data(const dovah::loaded_forms::Region& loaded) {
      this->stub = &loaded.stub;

      this->bounds = {};
      this->generable_content = {};

      this->map_color = QColor(loaded.map_color.r, loaded.map_color.g, loaded.map_color.b);

      if (auto* stub = loaded.parent_world.get_form_stub()) {
         if (stub->form_type == dovah::form_type::worldspace) {
            this->bounds.worldspace = stub;
            this->bounds.areas      = loaded.areas;
         }
      }

      struct {
         #define DO(name, ...) std::optional<region_data_header> name;
         FOR_EACH_GENERABLE_TYPE(DO)
         #undef DO
      } headers;

      auto& editor = DovahKitCore::get();
      for (auto& item : loaded.generable_content) {
         auto type = item.type();
         if (!type.has_value())
            continue;
         region_data_header header;
         header.override = item.override;
         header.priority = item.priority;
         switch (type.value()) {
            #define DO(name, enum_name, ...) case dovah::region_data_type::enum_name: headers.name.emplace() = header; break;
            FOR_EACH_GENERABLE_TYPE(DO);
            #undef DO
         }
      }

      #define DO(name, ...) \
         if (headers.name.has_value()) { \
            auto& data = this->generable_content.name.emplace(); \
            data.region_data_header::operator=(headers.name.value()); \
            data.import_data(loaded); \
         }
      FOR_EACH_GENERABLE_TYPE(DO);
      #undef DO
   }
   void region::export_data(dovah::loaded_forms::Region& loaded) const {
      auto& editor = DovahKitCore::get();
      if (!loaded.is_working_copy) {
         emit editor.formModificationImminent(&loaded.stub);
      }

      loaded.map_color = {
         .r = (uint8_t)this->map_color.red(),
         .g = (uint8_t)this->map_color.green(),
         .b = (uint8_t)this->map_color.blue(),
      };

      loaded.parent_world.set(loaded, this->bounds.worldspace);
      loaded.areas = this->bounds.areas;

      for (auto& item : loaded.generable_content)
         item.clear(loaded);
      loaded.generable_content.clear();
      
      {
         auto& src_opt  = this->generable_content.objects;
         using src_type = std::decay_t<decltype(src_opt)>::value_type;
         if (src_opt.has_value()) {
            auto& src_coll = src_opt.value();
            auto& dst_item = loaded.generable_content.emplace_back();
            auto& dst_coll = dst_item.get_or_emplace<src_type::backend_collection_type>(loaded);
            dst_item.override = src_coll.override;
            dst_item.priority = src_coll.priority;
            src_coll.export_data(loaded, dst_coll);
         }
      }
      {
         auto& src_opt  = this->generable_content.weather;
         using src_type = std::decay_t<decltype(src_opt)>::value_type;
         if (src_opt.has_value()) {
            auto& src_coll = src_opt.value();
            auto& dst_item = loaded.generable_content.emplace_back();
            auto& dst_coll = dst_item.get_or_emplace<src_type::backend_collection_type>(loaded);
            dst_item.override = src_coll.override;
            dst_item.priority = src_coll.priority;
            src_coll.export_data(loaded, dst_coll);
         }
      }
      {
         auto& src_opt  = this->generable_content.map;
         using src_type = std::decay_t<decltype(src_opt)>::value_type;
         if (src_opt.has_value()) {
            auto& src_coll = src_opt.value();
            auto& dst_item = loaded.generable_content.emplace_back();
            auto& dst_coll = dst_item.get_or_emplace<src_type::backend_collection_type>(loaded);
            dst_item.override = src_coll.override;
            dst_item.priority = src_coll.priority;
            src_coll.export_data(loaded, dst_coll);
         }
      }
      {
         auto& src_opt  = this->generable_content.landscape;
         if (src_opt.has_value()) {
            auto& src_coll = src_opt.value();
            auto& dst_item = loaded.generable_content.emplace_back();
            auto& dst_coll = dst_item.get_or_emplace<dovah::loaded_forms::structs::region::generable_content::landscape>(loaded);
            dst_item.override = src_coll.override;
            dst_item.priority = src_coll.priority;
            src_coll.export_data(loaded, dst_coll);
         }
      }
      {
         auto& src_opt  = this->generable_content.grass;
         using src_type = std::decay_t<decltype(src_opt)>::value_type;
         if (src_opt.has_value()) {
            auto& src_coll = src_opt.value();
            auto& dst_item = loaded.generable_content.emplace_back();
            auto& dst_coll = dst_item.get_or_emplace<src_type::backend_collection_type>(loaded);
            dst_item.override = src_coll.override;
            dst_item.priority = src_coll.priority;
            src_coll.export_data(loaded, dst_coll);
         }
      }
      {
         auto& src_opt  = this->generable_content.audio;
         using src_type = std::decay_t<decltype(src_opt)>::value_type;
         if (src_opt.has_value()) {
            auto& src_coll = src_opt.value();
            auto& dst_item = loaded.generable_content.emplace_back();
            auto& dst_coll = dst_item.get_or_emplace<src_type::backend_collection_type>(loaded);
            dst_item.override = src_coll.override;
            dst_item.priority = src_coll.priority;
            src_coll.export_data(loaded, dst_coll);
         }
      }

      if (!loaded.is_working_copy) {
         loaded.stub.set_edited(true);
         emit editor.formModified(&loaded.stub);
      }
   }

   void region::import_record_flags(uint32_t flags) {
      this->is_border_region = flags & dovah::loaded_forms::Region::form_flag::border_region;
   }
   void region::export_record_flags(uint32_t& flags) {
      cobb::edit_bit(flags, dovah::loaded_forms::Region::form_flag::border_region, this->is_border_region);
   }
}

#undef FOR_EACH_GENERABLE_TYPE