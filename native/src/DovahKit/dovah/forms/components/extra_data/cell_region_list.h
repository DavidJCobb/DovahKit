#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class cell_region_list : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XCLR';
         //
         std::vector<form_reference_t> regions; // REGN
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::cell_region_list; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}