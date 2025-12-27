#pragma once
#include <cstdint>
#include "../../extra_data.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class leveled_creature_modifier : public extra_data {
      public:
         static constexpr uint32_t signature = 'XLCM';

         enum class difficulty {
            easy,
            normal,
            hard,
            very_hard,
            none,
         };

      public:
         leveled_creature_modifier() : extra_data(all_extra_data_types::index_of_type<leveled_creature_modifier>) {}

      public:
         difficulty value = difficulty::normal;

      public:
         virtual subrecord_load_result load(tes_file_reading::subrecord&, load_interface_t&) override;
         virtual record_load_result load(tes_file_reading::record&, load_interface_t&) override;
         virtual void save(tes_file_writing::record&, save_interface_t&) override;
         
         static void generate_use_info(tes_file_reading::record&, form_stub_use_info_builder&, extra_data_use_info_state&);
         virtual void clear_contained_formIDs(loaded_forms::Form& my_owner) override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
         
         virtual extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
   };
}