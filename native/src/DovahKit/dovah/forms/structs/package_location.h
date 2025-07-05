#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include "../_common.h"
#include "../../data/packages/interrupt_override_target.h"
#include "../../data/packages/location_type.h"
#include "../../data/packages/object_type.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovah::loaded_forms::structs {
   struct package_location {
      public:
         static constexpr const uint32_t subrecord_package_data  = 'PLDT'; // PACK/PLDT: Package Location DaTa
         static constexpr const uint32_t subrecord_vendor_data   = 'PLVD'; // FACT/PLVD: Package Location Vendor Data
         static constexpr const uint32_t subrecord_legacy_second = 'PLD2';

         using interrupt_override_target = packages::interrupt_override_target;
         using object_type = packages::object_type;
         using location_type = packages::location_type;

         // Type indices correspond to `package_location_type` values.
         using data_variant = std::variant<
            form_reference_t, // reference -> REFR
            form_reference_t, // cell -> CELL
            std::monostate,
            std::monostate,
            form_reference_t, // object_id
            object_type,
            form_reference_t, // near_linked_reference -> KYWD
            std::monostate,
            int32_t,
            int32_t,
            interrupt_override_target,
            std::monostate,
            std::monostate
         >;

      public:
         data_variant data;
         int32_t      radius = 0;

      public:
         location_type get_type() const;
         void set_type(Form& my_owner, location_type);

         template<location_type PLT>
         std::variant_alternative_t<(size_t)PLT, data_variant>* as_type() {
            if (this->data.index() != (size_t)PLT)
               return nullptr;
            return &std::get<(size_t)PLT>(this->data);
         };
         //
         template<location_type PLT>
         const std::variant_alternative_t<(size_t)PLT, data_variant>* as_type() const {
            if (this->data.index() != (size_t)PLT)
               return nullptr;
            return &std::get<(size_t)PLT>(this->data);
         };

      protected:
         void _clear_data(Form& my_owner);
         void _emplace_data_for_type(location_type);

         template<location_type Type>
         auto& _as_type() {
            return std::get<(size_t)Type>(this->data);
         }

         template<location_type Type>
         auto& _get_or_emplace_data() {
            if (this->get_type() != Type)
               this->_emplace_data_for_type(Type);
            return *this->as_type<Type>();
         }

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, Form& my_owner);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&) = delete; // use (package_location::use_info_state)
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const package_location& src, Form& my_owner) noexcept;
         void unmanaged_clone_from(const package_location& src, load_order_interfaces::form_load&) noexcept; // HACK: used during PACK form load
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