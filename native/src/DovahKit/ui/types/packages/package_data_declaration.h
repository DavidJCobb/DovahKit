#pragma once
#include <cstdint>
#include <QString>

namespace ui::types::packages {
   struct package_data_declaration {
      public:
         static constexpr const uint8_t no_unique_id = 0xFF;

      public:
         QString name;
         bool    is_public = true;
         uint8_t unique_id = no_unique_id;
   };
}