#pragma once
#include <cstdint>
#include <variant>
#include "dovah/data/packages/interrupt_override_target.h"
#include "dovah/data/packages/object_type.h"
#include "dovah/data/packages/target_type.h"
namespace dovah {
   namespace loaded_forms {
      namespace structs {
         struct package_target;
      }
      class Form;
   }
   class form_stub;
}

namespace ui::types::packages {
   struct package_target {
      public:
         using backend_type = dovah::loaded_forms::structs::package_target;

         using interrupt_override_target = dovah::packages::interrupt_override_target;
         using object_type = dovah::packages::object_type;
         using target_type = dovah::packages::target_type;

         // Type indices correspond to `packages::target_type` values.
         using data_variant = std::variant<
            dovah::form_stub*, // REFR // reference
            dovah::form_stub*, // FORM // object
            object_type,       //      // object type
            dovah::form_stub*, // KYWD // linked ref with keyword
            int32_t,           //      // reference alias ID
            interrupt_override_target,
            std::monostate     //      // self
         >;

      public:
         data_variant data;
         union {
            int32_t count;
            int32_t distance = 0;
         };

      public:
         constexpr target_type get_type() const {
            return (target_type)this->data.index();
         }
         constexpr void set_type(target_type t) {
            if (t == this->get_type())
               return;
            #pragma push_macro("CASE")
            #define CASE(v) case v: this->data.emplace<(size_t)v>(); break;
            switch (t) {
               CASE(target_type::reference);
               CASE(target_type::object);
               CASE(target_type::object_type);
               CASE(target_type::linked_ref);
               CASE(target_type::reference_alias);
               CASE(target_type::interrupt_override_target);
               CASE(target_type::self);
            }
            #undef CASE
            #pragma pop_macro("CASE")
         }

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

         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& owner) const;

         // Returns `true` if anything changes.
         bool sever_uses_of_form(dovah::form_stub&);
   };
}