#pragma once
#include <optional>
#include "./base.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   // Any legacy package type that lacks its own TESPackageData subclass in-engine.
   class legacy_general : public base {
      public:
         static constexpr const package_type  type = package_type::dialogue;
         static constexpr const info_metadata metadata = {
            .accepts_locations = { true, false },
            .accepts_targets   = { true, false },
         };

      public:
         std::optional<package_location> location;    // Location 1
         std::optional<package_location> target; // Target 1

      public:
         virtual void accept_primary_location(const package_location& v) override { this->location = v; }
         virtual void accept_secondary_location(const package_location& v) override {}

         virtual void accept_primary_target(const package_location& v) override { this->target = v; }
         virtual void accept_secondary_target(const package_location& v) override {}

         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         //static std::vector<form_id_t> generate_header_use_info(tes_record_reader&);
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) const override;
         virtual base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept override;
         virtual void clear(loaded_forms::Form& my_containing_form) override;
   };
}