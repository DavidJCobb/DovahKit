#pragma once
#include "../package_data.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data_object_list : public package_data {
      public:
         static constexpr const char* const serialized_typename = "ObjectList";

      public:
         // I've confirmed that this is a float, though of course I don't know *why* it's a float.
         float value = 0;

      public:
         virtual package_data_type get_type() const noexcept { return package_data_type::object_list; }
         //
         virtual void load_value(tes_record_reader&, load_order_interfaces::form_load&, const load_context&) override;
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         virtual void save_value(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual package_data* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept override;
         virtual void clear(loaded_forms::Form& my_containing_form) override;
   };
}