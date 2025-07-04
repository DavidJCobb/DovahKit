#pragma once
#include "../base.h"

namespace dovah::loaded_forms::structs::typed_package_info::generic {
   class with_target final : public base {
      public:
         with_target(legacy_type t) : type(t) {}

      public:
         const legacy_type type;
         package_target    target; // Target 1

      public:
         virtual package_location* get_location_1() override { return nullptr; };
         virtual package_location* get_location_2() override { return nullptr; };
         virtual package_target* get_target_1() override { return &this->target; };
         virtual package_target* get_target_2() override { return nullptr; };
         //
         virtual void set_location_1(const package_location& v) override {}
         virtual void set_location_2(const package_location& v) override {}
         virtual void set_target_1(const package_target& v) override { this->target = v; }
         virtual void set_target_2(const package_target& v) override {}
         //
         virtual bool is_of_legacy_type(legacy_type t) const noexcept override { return t == this->type; };

         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         static void generate_header_use_info(tes_record_reader&, form_stub_use_info_builder&);
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept override;
         virtual void clear(loaded_forms::Form& my_containing_form) override;
   };
}