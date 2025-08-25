#pragma once
#include <cstdint>
#include <variant>
#include "dovah/data/packages/interrupt_override_target.h"
#include "dovah/data/packages/location_type.h"
#include "dovah/data/packages/object_type.h"
namespace dovah {
   namespace loaded_forms {
      namespace structs {
         struct package_location;
      }
      class Form;
   }
   class form_stub;
}

namespace ui::types::packages {
   struct package_location {
      public:
         using backend_type = dovah::loaded_forms::structs::package_location;

         using interrupt_override_target = dovah::packages::interrupt_override_target;
         using location_type = dovah::packages::location_type;
         using object_type   = dovah::packages::object_type;

         // Type indices correspond to `packages::target_type` values.
         using data_variant = std::variant<
            dovah::form_stub*, // reference -> REFR
            dovah::form_stub*, // cell -> CELL
            std::monostate,
            std::monostate,
            dovah::form_stub*, // object_id
            object_type,
            dovah::form_stub*, // near_linked_reference -> KYWD
            std::monostate,
            int32_t,
            int32_t,
            interrupt_override_target,
            uint8_t,
            std::monostate
         >;

      public:
         data_variant data;
         int32_t      radius = 0;

      public:
         constexpr location_type get_type() const {
            return (location_type)this->data.index();
         }
         constexpr void set_type(location_type t) {
            if (t == this->get_type())
               return;
            #pragma push_macro("CASE")
            #define CASE(v) case v: this->data.emplace<(size_t)v>(); break;
            switch (t) {
               CASE(location_type::reference);
               CASE(location_type::interior_cell);
               CASE(location_type::near_package_start_location);
               CASE(location_type::near_editor_location);
               CASE(location_type::object);
               CASE(location_type::object_type);
               CASE(location_type::linked_ref);
               CASE(location_type::at_package_location);
               CASE(location_type::reference_alias);
               CASE(location_type::location_alias);
               CASE(location_type::interrupt_override_target);
               CASE(location_type::package_data_target);
               CASE(location_type::self);
            }
            #undef CASE
            #pragma pop_macro("CASE")
         }

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

         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& owner) const;

         // Returns `true` if anything changes.
         bool sever_uses_of_form(dovah::form_stub&);
   };
}