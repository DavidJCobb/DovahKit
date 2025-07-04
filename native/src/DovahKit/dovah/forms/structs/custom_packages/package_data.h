#pragma once
#include <memory>
#include <string>
#include "../../_common.h"
#include "dovah/data/packages/package_data_type.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data {
      public:
         static constexpr const uint32_t subrecord_typename  = 'ANAM';
         static constexpr const uint32_t subrecord_var_name  = 'BNAM';
         static constexpr const uint32_t subrecord_access    = 'PNAM';
         static constexpr const uint32_t subrecord_unique_id = 'UNAM';

         static constexpr const uint8_t no_unique_id = 0xFF;

         using package_data_type = ::dovah::packages::package_data_type;

         // for error reporting
         struct load_context {
            size_t which; // we are the N-th package data in the package
         };

      public:
         uint8_t     unique_id = no_unique_id; // UNAM
         std::string name; // BNAM
         bool        is_public = false; // PNAM != 0

      public:
         virtual ~package_data() {}

         static std::unique_ptr<package_data> load_content(tes_record_reader&, load_order_interfaces::form_load&, const load_context&); // ANAM+[BNAM+PNAM]+<value>
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);


         // caller must handle ANAM
         // call load_value
         // caller must handle UNAM
         void load_metadata(tes_record_reader&, load_order_interfaces::form_load&); // BNAM+PNAM

         void save_typename(tes_record_writer&, load_order_interfaces::form_save&); // ANAM
         // call save_value
         void save_unique_id(tes_subrecord_writer&, load_order_interfaces::form_save&); // UNAM
         void save_metadata(tes_record_writer&, load_order_interfaces::form_save&); // BNAM+PNAM

      public:
         virtual package_data_type get_type() const noexcept = 0;
         //
         virtual void load_value(tes_record_reader&, load_order_interfaces::form_load&, const load_context&) = 0;
         //static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {};
         virtual void save_value(tes_record_writer&, load_order_interfaces::form_save&) = 0;
         virtual package_data* clone(loaded_forms::Form& owner_of_clone) const noexcept = 0;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept = 0;
         virtual void clear(loaded_forms::Form& my_containing_form) = 0;

   };
}