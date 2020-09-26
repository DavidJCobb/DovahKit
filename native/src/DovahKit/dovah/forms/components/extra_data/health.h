#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class health : public basic_extra_data {
      //
      // Apparently an Oblivion leftover; still read. I wonder how much of Skyrim is still 
      // compatible with this, e.g. whether you could slap it onto a destructible object 
      // REFR to spawn it partially destroyed.
      //
      public:
         static constexpr uint32_t signature = 'XHLT';
         //
         uint32_t value = 0;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::health; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*) {}
   };
}