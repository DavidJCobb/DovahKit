#pragma once
#pragma once
#include <cstdint>
#include "helpers/vector3.h"
#include "../../extra_data.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class occlusion_plane : public extra_data {
      public:
         static constexpr uint32_t signature = 'XOCP';

      public:
         occlusion_plane() : extra_data(all_extra_data_types::index_of_type<occlusion_plane>) {}

      public:
         float width  = 0;
         float height = 0;
         cobb::vector3<float> position;
         struct {
            float a = 0;
            float b = 0;
            float c = 0;
            float d = 0;
         } rotation;

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