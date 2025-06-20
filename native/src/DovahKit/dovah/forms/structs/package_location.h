#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include "../_common.h"
#include "../../data/package_location_type.h"

namespace dovah::loaded_forms::structs {
   struct package_location {
      public:
         static constexpr const uint32_t subrecord_package_data = 'PLDT'; // PACK/PLDT: Package Location DaTa
         static constexpr const uint32_t subrecord_vendor_data  = 'PLVD'; // FACT/PLVD: Package Location Vendor Data

         // Type indices correspond to `package_location_type` values.
         using data_variant = std::variant<
            form_reference_t, // reference -> REFR
            form_reference_t, // cell -> CELL
            std::monostate,
            std::monostate,
            form_reference_t, // object_id
            package_location_object_type_filter,
            form_reference_t, // near_linked_reference -> KYWD
            std::monostate,
            int32_t,
            int32_t,
            std::monostate,
            std::monostate,
            std::monostate
         >;

      public:
         data_variant data;
         int32_t      radius = 0;

      public:
         package_location_type get_type() const;
         void set_type(Form& my_owner, package_location_type);

         template<package_location_type PLT>
         auto* as_type() {
            if (this->data.index() != (size_t)PLT)
               return nullptr;
            return &std::get<(size_t)PLT>(this->data);
         };
         //
         template<package_location_type PLT>
         const auto* as_type() const {
            if (this->data.index() != (size_t)PLT)
               return nullptr;
            return &std::get<(size_t)PLT>(this->data);
         };

      protected:
         void _clear_data(Form& my_owner);
         void _emplace_data_for_type(package_location_type);

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&) = delete; // use (package_location::use_info_state)
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const package_location& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct use_info_state {
            form_id_t form;
            //
            void generate_use_info(tes_subrecord_reader&);
            void clear();
            void commit_to(form_stub_use_info_builder&);
         };
   };
}