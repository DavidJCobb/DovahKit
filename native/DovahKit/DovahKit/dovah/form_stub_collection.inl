#pragma once
#include "./form_stub_collection.h"

namespace dovah {
   inline size_t form_stub_collection::count_all_forms() const noexcept {
      return this->forms.forms.size();
   }
   inline size_t form_stub_collection::count_forms_of_type(form_type ft) const noexcept {
      if ((size_t)ft >= this->forms_by_type.size())
         return 0;
      return this->forms_by_type[(size_t)ft].forms.size();
   }

   template<typename Functor>
      requires std::is_invocable_v<Functor, const form_stub&>
   void form_stub_collection::for_each_form(Functor&& functor) const {
      for (const auto& pair : this->forms) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Functor, const form_stub&>) {
            auto result = functor(*pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            functor(*pair.second);
         }
      }
   }
   //
   template<typename Functor>
      requires std::is_invocable_v<Functor, form_stub&>
   void form_stub_collection::for_each_form(Functor&& functor) {
      for (const auto& pair : this->forms) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Functor, form_stub&>) {
            auto result = functor(*pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            functor(*pair.second);
         }
      }
   }
         
   template<typename Functor>
      requires std::is_invocable_v<Functor, const form_stub&>
   void form_stub_collection::for_each_form_of_type(form_type ft, Functor&& functor) const {
      if ((size_t)ft >= this->forms_by_type.size())
         return;
      auto& map = this->forms_by_type[(size_t)ft];
      for (const auto& pair : map) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Functor, const form_stub&>) {
            auto result = functor(*pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            functor(*pair.second);
         }
      }
   }
   //
   template<typename Functor>
      requires std::is_invocable_v<Functor, form_stub&>
   void form_stub_collection::for_each_form_of_type(form_type ft, Functor&& functor) {
      if ((size_t)ft >= this->forms_by_type.size())
         return;
      auto& map = this->forms_by_type[(size_t)ft];
      for (const auto& pair : map) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Functor, form_stub&>) {
            auto result = functor(*pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            functor(*pair.second);
         }
      }
   }
   
   inline const form_stub* form_stub_collection::get_form_by_id(bare_form_id form_id, bool ignore_none_stubs) const {
      auto& map = this->forms.forms;
      auto  it  = map.find(form_id);
      if (it == map.end())
         return nullptr;
      if (ignore_none_stubs)
         if (it->second->is_none_stub())
            return nullptr;
      return it->second;
   }
   inline form_stub* form_stub_collection::get_form_by_id(bare_form_id form_id, bool ignore_none_stubs) {
      return const_cast<form_stub*>(std::as_const(*this).get_form_by_id(form_id, ignore_none_stubs));
   }
   inline const form_stub* form_stub_collection::get_form_by_id(bare_form_id form_id, form_type ft, bool ignore_none_stubs) const {
      if ((size_t)ft >= this->forms_by_type.size())
         return nullptr;
      auto& map = this->forms_by_type[(size_t)ft].forms;
      auto  it  = map.find(form_id);
      if (it == map.end())
         return nullptr;
      if (ignore_none_stubs)
         if (it->second->is_none_stub())
            return nullptr;
      return it->second;
   }
   inline form_stub* form_stub_collection::get_form_by_id(bare_form_id form_id, form_type ft, bool ignore_none_stubs) {
      return const_cast<form_stub*>(std::as_const(*this).get_form_by_id(form_id, ft, ignore_none_stubs));
   }
}