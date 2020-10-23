#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class leveled_creature_modifier : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XLCM';
         enum class difficulty {
            easy,
            normal,
            hard,
            very_hard,
         };
         //
         difficulty value = difficulty::normal;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::leveled_creature_modifier; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}