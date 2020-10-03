#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class navmesh_door_portal : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XNDP';
         //
         form_id_t navmesh;
         int16_t   triangle;
         uint16_t  pad06;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::navmesh_door_portal; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}