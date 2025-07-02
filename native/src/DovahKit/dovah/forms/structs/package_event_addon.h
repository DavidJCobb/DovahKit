#pragma once
#include <cstdint>
#include "../_common.h"
#include "./package_data_topic.h"

namespace dovah::loaded_forms::structs {
   struct package_event_addon {
      public:
         form_reference_t   idle;
         package_data_topic topic;

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) = delete; // use (package_location::use_info_state)
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const package_event_addon& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct use_info_state {
            form_id_t idle;
            form_id_t topic;
            //
            void generate_use_info(tes_record_reader&);
            void commit_to(form_stub_use_info_builder&);
         };
   };
}