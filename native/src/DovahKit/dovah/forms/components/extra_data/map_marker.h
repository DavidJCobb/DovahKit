#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class map_marker : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XMRK';
         struct flag {
            flag() = delete;
            enum type : uint8_t {
               visible         = 0x01,
               can_travel_to   = 0x02,
               show_all_hidden = 0x04, // prevents ShowAllMapMarkers and friends from affecting this marker unless the scripter specifically decides otherwise
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;
         //
         flags_t          flags = 0;
         localized_string name;
         uint16_t         type  = 0;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::map_marker; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual bool        load(tes_record_reader&) override;
         virtual void        save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}