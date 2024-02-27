#pragma once
#include <concepts>
#include "dovah/forms/_all.h"

namespace dovahkit::subsystems::form_info_cache::cacheable_traits {
   namespace impl::attached_scripts {
      template<typename LoadedForm>
      concept form_data_has_papyrus = requires (LoadedForm& f) {
         { f.script_data } -> std::same_as<dovah::loaded_forms::components::papyrus_attachment_data&>;
      };
   }

   // This is a struct definition solely because it's impossible to build a list of 
   // namespaces for use in compile-time computation.
   struct attached_scripts {
      attached_scripts() = delete;

      template<typename LoadedFormClass>
      static constexpr const bool form_class_is_of_interest = impl::attached_scripts::form_data_has_papyrus<LoadedFormClass>;

      static constexpr const auto form_types_of_interest = []() {
         constexpr const size_t size = []() -> size_t {
            size_t out = 0;
            dovah::all_loaded_form_types::for_each([&out]<typename Current>() {
               if constexpr (form_class_is_of_interest<Current>)
                  ++out;
            });
            return out;
         }();

         std::array<dovah::form_type::type, size> types = {};
         size_t i = 0;
         dovah::all_loaded_form_types::for_each([&types, &i]<typename Current>() {
            if constexpr (form_class_is_of_interest<Current>)
               types[i++] = (dovah::form_type::type)Current::form_type;
         });
         return types;
      }();

      static constexpr const bool form_type_is_of_interest(dovah::form_type_t ft) {
         for (auto v : form_types_of_interest)
            if (v == ft)
               return true;
         return false;
      }
   };

}