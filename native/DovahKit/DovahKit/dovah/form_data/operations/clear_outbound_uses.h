#pragma once
#include <type_traits>
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "../helpers/is_visitable.h"
#include "../helpers/visit_members_recursively.h"
#include "lu/macros/FORCE_INLINE.define.h"

namespace dovah::form_data::operations {
   class clear_outbound_uses_visitor : public form_data::helpers::recursive_visitor_bases::ignore_primitives {
      public:
         constexpr clear_outbound_uses_visitor(form_stub& cf) : containing_form(cf) {}

      public:
         form_stub& containing_form;

         #pragma region Values to accept
            using form_data::helpers::recursive_visitor_bases::ignore_primitives::visit;

            // Form uses
            template<typename T>
               requires std::is_base_of_v<managed_form_use, T>
            LU_FORCE_INLINE void visit(T& data) {
               data.set(this->containing_form, nullptr);
            }
         
            // String uses.
            template<typename T>
               requires std::is_base_of_v<_base_managed_localized_string_use, T>
            LU_FORCE_INLINE void visit(T& data) {
               data.clear(this->containing_form);
            }
         
            // Vectors of form or string uses.
            template<typename T>
               requires (std::is_base_of_v<managed_form_use, T> || std::is_base_of_v< _base_managed_localized_string_use, T>)
            LU_FORCE_INLINE void visit(managed_vector<T>& data) {
               data.clear(this->containing_form);
            }
         #pragma endregion
   };

   template<typename T>
      requires (
         helpers::can_visit_recursively_with<T, clear_outbound_uses_visitor>
      || helpers::is_visitable<T>
      )
   void clear_outbound_uses(form_stub& containing_form, T& data) {
      clear_outbound_uses_visitor visitor{ containing_form };
      if constexpr (helpers::can_visit_recursively_with<T, clear_outbound_uses_visitor>) {
         visitor.visit(data);
      } else {
         helpers::visit_members_recursively(data, visitor);
      }
   }
}

#include "lu/macros/FORCE_INLINE.undef.h"