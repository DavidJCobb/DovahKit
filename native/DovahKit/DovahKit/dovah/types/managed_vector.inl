#pragma once
#include "./managed_vector.h"
#include "dovah/form_data/operations/assign.h"
#include "dovah/form_data/operations/clear_outbound_uses.h"
#include "dovah/form_stub.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T>
#define CLASS_NAME managed_vector<T>

namespace dovah {
   #pragma region Copy
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::assign(form_stub& containing_form, const managed_vector& src) {
         const size_type size_prior = this->size();
         const size_type size_after = src.size();
         if (size_prior < size_after) {
            this->_wrapped.resize(size_after);
         }
         for (size_t i = 0; i < size_after; ++i) {
            form_data::operations::assign(containing_form, this->_wrapped[i], src._wrapped[i]);
         }
         if (size_prior > size_after) {
            for (size_t i = size_after; i < size_prior; ++i) {
               form_data::operations::clear_outbound_uses(containing_form, this->_wrapped[i]);
            }
            this->_wrapped.resize(size_after);
         }
      }
   #pragma endregion
   #pragma region Modifiers
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::clear(form_stub& containing_form) {
         for (auto& item : this->_wrapped) {
            form_data::operations::clear_outbound_uses(containing_form, item);
         }
         this->_wrapped.clear();
      }
      TEMPLATE_PARAMS
      typename CLASS_NAME::value_type& CLASS_NAME::emplace_back() {
         return this->_wrapped.emplace_back();
      }
      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::iterator CLASS_NAME::erase(form_stub& containing_form, iterator it) {
         form_data::operations::clear_outbound_uses(containing_form, *it);
         this->_wrapped.erase(it);
      }
      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::iterator CLASS_NAME::erase(form_stub& containing_form, iterator first, iterator last) {
         for(auto it = first; it <= last; ++it)
            form_data::operations::clear_outbound_uses(containing_form, *it);
         this->_wrapped.erase(first, last);
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::pop_back(form_stub& containing_form) {
         this->erase(this->rbegin());
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::resize(form_stub& containing_form, size_type size_after) {
         const auto size_prior = this->size();
         if (size_prior > size_after) {
            for (size_type i = size_after; i < size_prior; ++i) {
               form_data::operations::clear_outbound_uses(containing_form, this->_wrapped[i]);
            }
         }
         this->_wrapped.resize(size_after);
      }
   #pragma endregion
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")
