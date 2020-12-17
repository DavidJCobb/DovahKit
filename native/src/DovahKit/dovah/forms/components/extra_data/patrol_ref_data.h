#pragma once
#include "../extra_data.h"
#include "../package_event_dialogue.h"

namespace dovah::loaded_forms::components::extra {
   class patrol_ref_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature_time  = 'XPRD';
         static constexpr uint32_t signature_event = 'XPPA'; // "patrol perform action?"
         //
         float idle_time; // XPRD
         package_event_dialogue event;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::patrol_ref_data; };
         virtual load_result load(tes_subrecord_reader&, load_interface_t&) override;
         virtual bool        load(tes_record_reader&,    load_interface_t&) override;
         virtual void        save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}