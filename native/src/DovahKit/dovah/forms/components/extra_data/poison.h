#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class poison : public basic_extra_data {
      public:
         static constexpr uint32_t signature_type = 'XPSN';
         static constexpr uint32_t signature_dose = 'XPSC';
         //
         form_id_t type;      // XPSN subrecord; value is ALCH
         uint32_t  doses = 1; // XPSC subrecord
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::poison; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
   };
}