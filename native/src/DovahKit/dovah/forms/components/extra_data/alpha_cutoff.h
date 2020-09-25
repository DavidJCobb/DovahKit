#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class alpha_cutoff : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XALP';
         //
         uint8_t cutoff;
         uint8_t base;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::alpha_cutoff; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
   };
}