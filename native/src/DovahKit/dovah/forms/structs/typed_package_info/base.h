#pragma once
#include <vector>
#include "../../../data/packages/legacy_type.h"
#include "../../_common.h"
#include "../package_location.h"
#include "../package_target.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   class base {
      public:
         using legacy_type = packages::legacy_type;

      public:
         virtual ~base() {}

         // Note for overrides:
         // Loading of Location 2 and Target 2 is handled by code in Package. However, saving of 
         // Location 2 and Target 2 must be handled by the subclass's own save function.
         //
         virtual package_location* get_location_1() = 0;
         virtual package_location* get_location_2() = 0;
         virtual package_target* get_target_1() = 0;
         virtual package_target* get_target_2() = 0;
         //
         virtual void set_location_1(const package_location&) = 0;
         virtual void set_location_2(const package_location&) = 0;
         virtual void set_target_1(const package_target&) = 0;
         virtual void set_target_2(const package_target&) = 0;
         //
         virtual bool is_of_legacy_type(legacy_type) const noexcept = 0;

         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) = 0;
         //static void generate_header_use_info(tes_record_reader&, std::vector<form_id_t>& out);
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) = 0;
         virtual base* clone(loaded_forms::Form& owner_of_clone) const noexcept = 0;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept = 0;
         virtual void clear(loaded_forms::Form& my_containing_form) = 0;
   };
}