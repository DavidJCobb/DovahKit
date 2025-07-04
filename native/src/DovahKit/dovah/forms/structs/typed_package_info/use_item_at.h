#pragma once
#include <optional>
#include "./base.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   class use_item_at final : public base {
      public:
         static constexpr const legacy_type type = legacy_type::use_item_at;
         static constexpr const uint32_t    header_subrecord = 'PUID';

      public:
         package_location use_at_location; // Location 1
         std::optional<package_location> search_location; // Location 2
         package_target item_to_use; // Target 1

      public:
         virtual package_location* get_location_1() override { return &this->use_at_location; };
         virtual package_location* get_location_2() override { auto& o = this->search_location; return o.has_value() ? &o.value() : nullptr; };
         virtual package_target* get_target_1() override { return &this->item_to_use; };
         virtual package_target* get_target_2() override { return nullptr; };
         //
         virtual void set_location_1(const package_location& v) override { this->use_at_location = v; }
         virtual void set_location_2(const package_location& v) override { this->search_location = v; }
         virtual void set_target_1(const package_target& v) override { this->item_to_use = v; }
         virtual void set_target_2(const package_target& v) override {}
         //
         virtual bool is_of_legacy_type(legacy_type t) const noexcept override { return t == type; };

         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         static void generate_header_use_info(tes_record_reader&, form_stub_use_info_builder&) {};
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept override;
         virtual void clear(loaded_forms::Form& my_containing_form) override;
   };
}