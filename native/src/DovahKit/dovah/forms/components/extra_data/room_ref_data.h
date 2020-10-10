#pragma once
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
         //
         uint8_t   linked_room_count = 0; // this is read as a dword
         uint8_t   flags             = 0; // 
         uint16_t  pad02;                 // 
         form_id_t lighting_template;
         form_id_t imagespace;
         std::vector<form_id_t> linked_rooms;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::room_ref_data; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual bool        load(tes_record_reader&) override;
         virtual void        save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
         virtual void get_outbound_formIDs(std::vector<form_id_t*>&) const noexcept override;
         virtual void on_after_delete() noexcept override;
         //
         void collapse();
   };
}