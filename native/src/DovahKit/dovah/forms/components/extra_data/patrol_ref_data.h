#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class patrol_ref_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature_time = 'XPRD';
         //
         float idle_time; // XPRD
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::patrol_ref_data; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
   };
}