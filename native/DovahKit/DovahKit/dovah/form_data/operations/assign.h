#pragma once
#include <cassert>
#include <type_traits>
#include <vector>
#include "lu/concepts/instantiation_of/std_vector.h"
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "../helpers/is_or_contains_managed_use.h"
#include "../helpers/is_visitable.h"
#include "../helpers/visit_members_in_tandem_recursively.h"
#include "./clear_outbound_uses.h"
#include "lu/macros/FORCE_INLINE.define.h"

namespace dovah::form_data_operations {
   class assign_visitor {
      public:
         constexpr assign_visitor(form_stub& s) : dst_containing_form(s) {}

      public:
         form_stub& dst_containing_form;

         #pragma region Values to accept
            template<typename Dst, typename Src>
               requires (!form_data_helpers::is_or_contains_managed_use<Dst> && !form_data_helpers::is_or_contains_managed_use<Src>)
            LU_FORCE_INLINE void visit(Dst& dst, const Src& src) {
               dst = src;
            }
            
            // Form uses.
            template<typename Dst, typename Src>
               requires (
                  (std::is_base_of_v<managed_form_use, Dst> || std::is_same_v<form_stub*, Dst>)
               && (std::is_base_of_v<managed_form_use, Src> || std::is_same_v<form_stub*, Src>)
               && (form_data_helpers::is_or_contains_managed_use<Dst> || form_data_helpers::is_or_contains_managed_use<Src>)
               )
            LU_FORCE_INLINE void visit(Dst& dst_data, const Src& src_data) {
               form_stub* v;
               if constexpr (std::is_base_of_v<managed_form_use, Src>) {
                  v = src_data.get();
               } else {
                  v = src_data;
               }
               if constexpr (std::is_base_of_v<managed_form_use, Dst>) {
                  dst_data.set(this->dst_containing_form, v);
               } else {
                  dst_data = v;
               }
            }
            
            // String uses.
            template<typename Dst, typename Src>
               requires (
                  std::is_base_of_v<_base_managed_localized_string_use, Dst>
               && std::is_base_of_v<_base_managed_localized_string_use, Src>
               && (form_data_helpers::is_or_contains_managed_use<Dst> || form_data_helpers::is_or_contains_managed_use<Src>)
               )
            void assign(Dst& dst_data, const Src& src_data) {
               const bool dst_is_localized = dst_data.is_localized();
               assert(src_data.is_localized() == dst_is_localized);
               if (dst_is_localized) {
                  const auto id = src_data.get_localized_id();
                  if (id) {
                     assert(src_data.is_active_file_string_id());
                  }
                  dst_data.set_localized_id(this->dst_containing_form, id);
               } else {
                  dst_data.set_non_localized_string(this->dst_containing_form, src_data.get_non_localized_string());
               }
            }
            
            // Vectors.
            template<typename Dst, typename Src>
               requires (
                  lu::concepts::instantiation_of_std_vector<Dst>
               && lu::concepts::instantiation_of_std_vector<Src>
               && (form_data_helpers::is_or_contains_managed_use<Dst> || form_data_helpers::is_or_contains_managed_use<Src>)
               )
            void visit(Dst& dst_data, const Src& src_data) {
               const size_t size_prior = dst_data.size();
               const size_t size_after = src_data.size();
               if (size_prior < size_after) {
                  dst_data.resize(size_after);
               }
               for (size_t i = 0; i < size_after; ++i) {
                  assign(this->dst_containing_form, dst_data[i], src_data[i]);
               }
               if (size_prior > size_after) {
                  for (size_t i = size_after; i < size_prior; ++i) {
                     clear_outbound_uses(this->dst_containing_form, dst_data[i]);
                  }
                  dst_data.resize(size_after);
               }
            }
         #pragma endregion
   };

   template<form_data_helpers::is_visitable Dst, form_data_helpers::is_visitable Src>
   void assign(form_stub& dst_containing_form, Dst& dst_data, const Src& src_data) {
      assign_visitor visitor{ dst_containing_form };
      form_data_helpers::visit_members_in_tandem_recursively(dst_data, src_data, visitor);
   }
}

#include "lu/macros/FORCE_INLINE.undef.h"