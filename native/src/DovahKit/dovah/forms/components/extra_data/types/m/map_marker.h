#pragma once
#include <cstdint>
#include "../../extra_data.h"
#include "../../../../../localized_strings.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class map_marker : public extra_data {
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

      public:
         map_marker() : extra_data(all_extra_data_types::index_of_type<map_marker>) {}

      public:
         flags_t          flags = 0;
         localized_string name;
         uint16_t         type  = 0;

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