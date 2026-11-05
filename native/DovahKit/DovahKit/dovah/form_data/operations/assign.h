#pragma once
#include <cassert>
#include <type_traits>
#include "lu/concepts/instantiation_of/std_array.h"
#include "lu/concepts/instantiation_of/std_vector.h"
#include "lu/concepts/map_like_type.h"
#include "../helpers/is_visitable.h"
#include "dovah/form_stub.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_localized_string_use.h"
#include "./clear_outbound_uses.h"
#include "lu/macros/FORCE_INLINE.define.h"

namespace dovah::form_data_operations {
   template<typename Dst, typename Src>
      requires (!std::is_pointer_v<Dst> && !std::is_pointer_v<Src> && !lu::concepts::map_like_type<Dst> && !lu::concepts::map_like_type<Src>)
   LU_FORCE_INLINE void assign(dovah::form_stub& dst_containing_form, Dst& dst_data, const Src& src_data) {
      dst_data = src_data;
   }

   template<typename Dst, typename Src>
      requires (
         (std::is_base_of_v<dovah::managed_form_use, Dst> || std::is_same_v<dovah::form_stub*, Dst>)
      && (std::is_base_of_v<dovah::managed_form_use, Src> || std::is_same_v<dovah::form_stub*, Src>)
      )
   LU_FORCE_INLINE void assign<Dst, Src>(dovah::form_stub& dst_containing_form, Dst& dst_data, const Src& src_data) {
      dovah::form_stub* v;
      if constexpr (std::is_base_of_v<dovah::managed_form_use, Src>) {
         v = src_data.get();
      } else {
         v = src_data;
      }
      if constexpr (std::is_base_of_v<dovah::managed_form_use, Dst>) {
         dst_data.set(dst_containing_form, v);
      } else {
         dst_data = v;
      }
   }

   template<typename Dst, typename Src>
      requires (
         std::is_base_of_v<dovah::_base_managed_localized_string_use, Dst>
      && std::is_base_of_v<dovah::_base_managed_localized_string_use, Src>
      )
   void assign<Dst, Src>(dovah::form_stub& dst_containing_form, Dst& dst_data, const Src& src_data) {
      const bool dst_is_localized = dst_data.is_localized();
      assert(src_data.is_localized() == dst_is_localized);
      if (dst_is_localized) {
         const auto id = src_data.get_localized_id();
         if (id) {
            assert(src_data.is_active_file_string_id());
         }
         dst_data.set_localized_id(dst_containing_form, id);
      } else {
         dst_data.set_non_localized_text(dst_containing_form, src_data.get_non_localized_string());
      }
   }

   #pragma region Containers
      template<typename T>
         requires (lu::concepts::instantiation_of_std_array<T>)
      LU_FORCE_INLINE void assign<T>(dovah::form_stub& dst_containing_form, T& dst_data, const T& src_data) {
         const size_t size = src_data.size();
         for (size_t i = 0; i < size; ++i)
            assign(dst_containing_form, dst_data[i], src_data[i]);
      }

      template<typename T>
         requires (lu::concepts::instantiation_of_std_vector<T>)
      void assign<T>(dovah::form_stub& dst_containing_form, T& dst_data, const T& src_data) {
         const size_t size_prior = dst_data.size();
         const size_t size_after = src_data.size();
         if (size_prior < size_after) {
            dst_data.resize(size_after);
         }
         for (size_t i = 0; i < size_after; ++i) {
            assign(dst_containing_form, dst_data[i], src_data[i]);
         }
         for (size_t i = size_after; i < size_prior; ++i) {
            clear_outbound_uses(dst_containing_form, dst_data[i]);
         }
      }
   #pragma endregion

   template<dovah::form_data_helpers::is_visitable Dst, dovah::form_data_helpers::is_visitable Src>
   void assign<Dst, Src>(form_stub& dst_containing_form, Dst& dst_data, const Src& src_data) {
      Dst::visit_members_in_tandem(dst_data, src_data, [&dst_containing_form](auto& dst_field, const auto& src_field) {
         assign(dst_containing_form, dst_field, src_field);
      });
   }
}

#include "lu/macros/FORCE_INLINE.undef.h"