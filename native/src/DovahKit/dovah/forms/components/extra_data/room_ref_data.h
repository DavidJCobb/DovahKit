#pragma once
#include <cstdint>
#include <limits>
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class room_ref_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XRMR';
         struct flag {
            flag() = delete;
            enum : uint8_t {
               has_imagespace        = 0x04,
               has_lighting_template = 0x08,
            };
         };

         static constexpr int max_linked_room_count = std::numeric_limits<uint8_t>::max();
         
      public:
         uint8_t flags = 0;
         form_reference_t lighting_template;
         form_reference_t imagespace;
         std::vector<form_reference_t> linked_rooms;
         
      public:
         virtual extra_data_type get_type() const noexcept { return extra_data_type::room_ref_data; };
         virtual load_result load(tes_subrecord_reader&, load_interface_t&) override;
         virtual bool        load(tes_record_reader&,    load_interface_t&) override;
         virtual void        save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&, extra_data_use_info_state&);
         //
         virtual basic_extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
   };
}