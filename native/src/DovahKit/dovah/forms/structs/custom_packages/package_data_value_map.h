#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "../../_common.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data;
}

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data_value_map { // counterpart to BGSPackageDataList
      public:
         static constexpr const uint32_t subrecord_unique_id   = 'UNAM';
         static constexpr const uint32_t subrecord_typename    = 'ANAM';
         static constexpr const uint32_t subrecord_decl_name   = 'BNAM';
         static constexpr const uint32_t subrecord_decl_access = 'PNAM';
         static constexpr const uint32_t subrecord_next_uid    = 'XNAM';

         static constexpr const size_t max_serializable_count = std::numeric_limits<uint32_t>::max();

         struct entry {
            uint8_t unique_id = 0xFF; // "none"
            std::unique_ptr<package_data> value = nullptr;
         };

      public:
         std::vector<entry> entries;
         uint8_t next_unique_id = 0;

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load&, size_t count);
         static void generate_use_info(tes_record_reader&, size_t count, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         void clone_from(const package_data_value_map& src, Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);
   };
}