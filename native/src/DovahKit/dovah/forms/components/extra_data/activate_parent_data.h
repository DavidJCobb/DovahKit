#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class activate_parent_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature_flags  = 'XAPD';
         static constexpr uint32_t signature_parent = 'XAPR';
         struct flag {
            flag() = delete;
            enum : uint8_t {
               parent_activate_only = 0x01,
            };
         };
         struct parent {
            form_reference_t ref;
            float delay;
         };
         //
         uint8_t flags = 0;
         std::vector<parent> parents;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::activate_parent_data; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}