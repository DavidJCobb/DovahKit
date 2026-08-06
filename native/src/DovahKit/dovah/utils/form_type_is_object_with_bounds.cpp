#include "./form_type_is_object_with_bounds.h"
#include <array>
#include "../forms/components/bounds.h"
#include "../forms/_all.h"

namespace {
   template<typename Loaded>
   concept has_object_bounds = requires (const Loaded& loaded) {
      { loaded.bounds } -> std::same_as<const dovah::loaded_forms::components::object_bounds&>;
   };

   constexpr const size_t relevant_form_type_count = []() {
      size_t i = 0;
      dovah::all_loaded_form_types::for_each([&i]<typename Loaded>() {
         if constexpr (has_object_bounds<Loaded>)
            ++i;
      });
      return i;
   }();

   constexpr const std::array<dovah::form_type, relevant_form_type_count> relevant_form_types = []() {
      std::array<dovah::form_type, relevant_form_type_count> out = {};

      size_t i = 0;
      dovah::all_loaded_form_types::for_each([&out, &i]<typename Loaded>() {
         if constexpr (has_object_bounds<Loaded>) {
            out[i] = Loaded::form_type;
            ++i;
         }
      });

      return out;
   }();
}

namespace dovah {
   extern bool form_type_is_object_with_bounds(form_type ft) {
      for (auto owb : relevant_form_types)
         if (ft == owb)
            return true;
      return false;
   }
}
