#pragma once
#include <cstdint>
#include "helpers/vector3.h"
#include "../../extra_data.h"
#include "../../../../../form_reference_t.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class water_current_zone_data : public extra_data {
      public:
         static constexpr uint32_t signature_vel_linear     = 'XCVL';
         static constexpr uint32_t signature_vel_rotational = 'XCVR';
         static constexpr uint32_t signature_zone_cell      = 'XCZC';
         static constexpr uint32_t signature_zone_action    = 'XCZA';
         static constexpr uint32_t signature_zone_ref       = 'XCZR';

      public:
         water_current_zone_data() : extra_data(all_extra_data_types::index_of_type<water_current_zone_data>) {}

         struct ref_entry {
            form_reference_t ref;
            int32_t action = -1;
         };

      public:
         int32_t action = -1;
         form_reference_t cell;
         std::vector<ref_entry> refs; // this would more properly be an unordered_map of refs to actions, but form_reference_t makes that... risky.
         struct {
            cobb::vector3<float> linear;
            cobb::vector3<float> angular;
         } velocity;
      protected:
         struct {
            bool expecting_ref = false;
         } _load_state;

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