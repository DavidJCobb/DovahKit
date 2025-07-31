#pragma once
#include <string>
#include <vector>
#include "../../_common.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data_declaration_map { // counterpart to BGSPackageDataNameMap
      public:
         static constexpr const uint32_t subrecord_unique_id = 'UNAM';
         static constexpr const uint32_t subrecord_name      = 'BNAM';
         static constexpr const uint32_t subrecord_access    = 'PNAM';

         struct entry {
            uint8_t     unique_id = 0xFF; // "none"
            std::string name;
            bool        is_public = false;
         };

      public:
         std::vector<entry> entries;

      public:
         // these functions should be called just after opening the first subrecord relevant to this 
         // struct. when these functions exit, the subrecord after this struct's data will have just 
         // been opened.
         void load(tes_record_reader&, load_order_interfaces::form_load&);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

         void save(tes_record_writer&, load_order_interfaces::form_save&);
   };
}