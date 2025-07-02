#pragma once
#include <memory>
#include <vector>
#include "./base.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data;
   class procedure_node;
}

namespace dovah::loaded_forms::structs::typed_package_info {
   class custom final : public base {
      public:
         static constexpr const legacy_type type = legacy_type::custom;
         static constexpr const uint32_t    header_subrecord = 'PKCU';

         using package_data   = custom_packages::package_data;
         using procedure_node = custom_packages::procedure_node;

         static constexpr const size_t max_serializable_package_data = std::numeric_limits<uint32_t>::max();
         static constexpr const size_t max_valid_package_data = std::numeric_limits<uint8_t>::max() - 1; // minus one to account for the `no_unique_id` sentinel value

      public:
         std::vector<package_data*> data;
         std::unique_ptr<procedure_node> procedure_tree = nullptr;
         form_reference_t template_package; // PKCU+0x04 -> PACK
         uint32_t         revision = 0; // PKCU+0x08
         uint32_t         next_unique_id = 0; // XNAM

      public:
         virtual package_location* get_location_1() override { return nullptr; };
         virtual package_location* get_location_2() override { return nullptr; };
         virtual package_target* get_target_1() override { return nullptr; };
         virtual package_target* get_target_2() override { return nullptr; };
         //
         virtual void set_location_1(const package_location& v) override {}
         virtual void set_location_2(const package_location& v) override {}
         virtual void set_target_1(const package_target& v) override {}
         virtual void set_target_2(const package_target& v) override {}
         //
         virtual bool is_of_legacy_type(legacy_type t) const noexcept override { return t == legacy_type::custom || t == legacy_type::custom_template; };

         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         static void generate_header_use_info(tes_record_reader&, std::vector<form_id_t>& out);
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept override;
         virtual void clear(loaded_forms::Form& my_containing_form) override;
   };
}