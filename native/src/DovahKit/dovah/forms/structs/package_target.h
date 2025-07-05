#pragma once
#include <cstdint>
#include <variant>
#include "../_common.h"
#include "../../data/packages/interrupt_override_target.h"
#include "../../data/packages/object_type.h"
#include "../../data/packages/target_type.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovah::loaded_forms::structs {
   struct package_target {
      public:
         static constexpr const uint32_t subrecord_legacy        = 'PTDT'; // PACK/PTDT: Package Target DaTa
         static constexpr const uint32_t subrecord_modern        = 'PTDA'; // PACK/PTDA: Package Target DatA
         static constexpr const uint32_t subrecord_modern_second = 'PT2A';

         using interrupt_override_target = packages::interrupt_override_target;
         using object_type = packages::object_type;
         using target_type = packages::target_type;

         // Type indices correspond to `packages::target_type` values.
         using data_variant = std::variant<
            form_reference_t, // REFR // reference
            form_reference_t, // FORM // object
            object_type,      //      // object type
            form_reference_t, // KYWD // linked ref with keyword
            int32_t,          //      // reference alias ID
            interrupt_override_target,
            std::monostate    //      // self
         >;

      public:
         data_variant data;
         union {
            int32_t count;
            int32_t distance = 0;
         };

      public:
         target_type get_type() const;
         void set_type(Form& my_owner, target_type);

         template<target_type PLT>
         std::variant_alternative_t<(size_t)PLT, data_variant>* as_type() {
            if (this->data.index() != (size_t)PLT)
               return nullptr;
            return &std::get<(size_t)PLT>(this->data);
         };
         //
         template<target_type PLT>
         const std::variant_alternative_t<(size_t)PLT, data_variant>* as_type() const {
            if (this->data.index() != (size_t)PLT)
               return nullptr;
            return &std::get<(size_t)PLT>(this->data);
         };

      protected:
         void _clear_data(Form& my_owner);
         void _emplace_data_for_type(target_type);

         template<target_type Type>
         auto& _get_or_emplace_data() {
            if (this->get_type() != Type)
               this->_emplace_data_for_type(Type);
            return *this->as_type<Type>();
         }

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, Form& my_owner);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&) = delete; // use (package_target::use_info_state)
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const package_target& src, Form& my_owner) noexcept;
         void unmanaged_clone_from(const package_target& src, load_order_interfaces::form_load&) noexcept; // HACK: used during PACK form load
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