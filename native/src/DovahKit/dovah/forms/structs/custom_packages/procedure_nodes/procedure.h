#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include "../../../_common.h"
#include "../../../../data/packages/procedure_type.h"
#include "../package_flag_overrides.h"

namespace dovah::loaded_forms::structs::custom_packages::procedure_node_data {
   class procedure {
      public:
         static constexpr const uint32_t subrecord_procedure_type  = 'PNAM';
         static constexpr const uint32_t subrecord_flags           = 'FNAM';
         static constexpr const uint32_t subrecord_param_id_legacy = 'PKCU';
         static constexpr const uint32_t subrecord_param_id_modern = 'PKC2';

         using procedure_type = packages::procedure_type;

         struct flag {
            enum type : uint32_t {
               success_completes_package = 1 << 0,
            };
         };

      public:
         procedure_type type = procedure_type::invalid;
         uint32_t flags = 0; // FNAM
         std::optional<package_flag_overrides> flag_overrides;
         std::vector<uint8_t> parameter_unique_ids; // PKC2[]
         
         void load(tes_record_reader&, load_order_interfaces::form_load&);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
   };
}