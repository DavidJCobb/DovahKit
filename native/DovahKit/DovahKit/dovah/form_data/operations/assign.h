#pragma once
#include <cassert>
#include <type_traits>
#include <vector>
#include "lu/concepts/instantiation_of/std_vector.h"
#include "dovah/form_data/helpers/is_or_contains_managed_use.h"
#include "dovah/form_data/helpers/is_managed_vector_type.h"
#include "dovah/form_data/helpers/is_subobject_vector_type.h"
#include "dovah/form_data/helpers/is_visitable.h"
#include "dovah/form_data/helpers/visit_members_in_tandem_recursively.h"
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "lu/macros/FORCE_INLINE.define.h"

namespace dovah::form_data::operations {
   class assign_visitor {
      public:
         constexpr assign_visitor(form_stub& s) : dst_containing_form(s) {}

      public:
         form_stub& dst_containing_form;

         #pragma region Values to accept
            template<typename Dst, typename Src>
               requires (!(helpers::is_or_contains_managed_use<Dst> || helpers::is_or_contains_managed_use<Src>))
            LU_FORCE_INLINE void visit(Dst& dst, const Src& src) {
               dst = src;
            }
            
            // Form uses.
            template<typename Dst, typename Src>
               requires (
                  (std::is_base_of_v<managed_form_use, Dst> || std::is_same_v<form_stub*, Dst>)
               && (std::is_base_of_v<managed_form_use, Src> || std::is_same_v<form_stub*, Src>)
               && (helpers::is_or_contains_managed_use<Dst> || helpers::is_or_contains_managed_use<Src>)
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
               && (helpers::is_or_contains_managed_use<Dst> || helpers::is_or_contains_managed_use<Src>)
               )
            void visit(Dst& dst_data, const Src& src_data) {
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

            //
            // A note on the vector-related handlers below: We act on `managed_vector` types, but 
            // we can't *include* the `managed_vector` header here, because that will lead to a 
            // circular include. (The `managed_vector` class itself depends on the functionality 
            // defined in this file, you see.) Fortunately, we don't need to. Since we're dealing 
            // with templates, the compiler basically doesn't care exactly what's inside of these 
            // functions at parse time (beyond needing to be *able* to parse them); it only cares 
            // once the function templates are instantiated (i.e. at the call site). If these 
            // functions are ever called, it will be because someone passed in a data structure 
            // that is or contains a `managed_vector`, and so the definition of `managed_vector` 
            // will be visible at that point.
            //
            
            // Vectors (assign managed to unmanaged).
            template<lu::concepts::instantiation_of_std_vector Dst, helpers::is_managed_vector_type Src>
            void visit(Dst& dst_data, const Src& src_data) {
               const size_t size = src_data.size();
               dst_data.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  assign(this->dst_containing_form, dst_data[i], src_data[i]);
               }
            }
            
            // Vectors (assign unmanaged to managed).
            template<helpers::is_managed_vector_type Dst, lu::concepts::instantiation_of_std_vector Src>
            void visit(Dst& dst_data, const Src& src_data) {
               const size_t size = src_data.size();
               dst_data.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  assign(this->dst_containing_form, dst_data[i], src_data[i]);
               }
            }
            
            // Vectors (assign managed to managed).
            template<helpers::is_managed_vector_type Dst, helpers::is_managed_vector_type Src>
            LU_FORCE_INLINE void visit(Dst& dst_data, const Src& src_data) {
               dst_data.assign(this->dst_containing_form, src_data);
            }
         #pragma endregion
   };
   
   template<typename Dst, typename Src>
      requires (
         helpers::can_visit_recursively_in_tandem_with<Dst, Src, assign_visitor>
      || (helpers::is_visitable<Dst> && helpers::is_visitable<Src>)
      )
   void assign(form_stub& dst_containing_form, Dst& dst_data, const Src& src_data) {
      assign_visitor visitor{ dst_containing_form };
      if constexpr (helpers::can_visit_recursively_in_tandem_with<Dst, Src, assign_visitor>) {
         visitor.visit(dst_data, src_data);
      } else {
         helpers::visit_members_in_tandem_recursively(dst_data, src_data, visitor);
      }
   }
}

#include "lu/macros/FORCE_INLINE.undef.h"