#pragma once
#include <QColor>
#include "dovah/forms/structs/region/area.h"
#include "../game_file_path.h"
#include "./generable_content/audio.h"
#include "./generable_content/grass_collection.h"
#include "./generable_content/landscape.h"
#include "./generable_content/map.h"
#include "./generable_content/object_collection.h"
#include "./generable_content/weather_collection.h"

namespace ui::types::regions {
   class region {
      public:
         using area = dovah::loaded_forms::structs::region::area;

         struct region_data_header {
            bool    override = false;
            uint8_t priority = 50;
         };

         template<typename T>
         class region_data : public region_data_header, public T {};

      public:
         dovah::form_stub* stub = nullptr;
         //
         bool is_border_region = false;
         QColor map_color;
         struct {
            dovah::form_stub* worldspace = nullptr;
            std::vector<area> areas;
         } bounds;
         struct {
            std::optional<region_data<generable_content::audio>>              audio;
            std::optional<region_data<generable_content::grass_collection>>   grass;
            std::optional<region_data<generable_content::landscape>>          landscape;
            std::optional<region_data<generable_content::map>>                map;
            std::optional<region_data<generable_content::object_collection>>  objects;
            std::optional<region_data<generable_content::weather_collection>> weather;
         } generable_content;

      public:
         void import_data(const dovah::loaded_forms::Region&);
         void export_data(dovah::loaded_forms::Region&) const;

         void import_record_flags(uint32_t);
         void export_record_flags(uint32_t&);
   };
}