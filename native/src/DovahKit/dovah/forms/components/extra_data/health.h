#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class health : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XHLT';
         //
         uint32_t value = 0;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::health; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
   };
}