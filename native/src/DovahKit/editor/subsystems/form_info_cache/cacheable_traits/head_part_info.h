#pragma once
#include <array>
#include "dovah/form_types.h"

namespace dovahkit::subsystems::form_info_cache::cacheable_traits {
   // This is a struct definition solely because it's impossible to build a list of 
   // namespaces for use in compile-time computation.
   struct head_part_info {
      head_part_info() = delete;

      static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::head_part };
      static constexpr const auto form_types_we_refer_to = std::array{ dovah::form_type::formlist };

      static constexpr const bool form_type_is_of_interest(dovah::form_type ft) {
         for (auto v : form_types_of_interest)
            if (v == ft)
               return true;
         return false;
      }
      static constexpr const bool form_type_is_referred_to(dovah::form_type ft) {
         for (auto v : form_types_we_refer_to)
            if (v == ft)
               return true;
         return false;
      }

      template<typename LoadedFormClass>
      static constexpr const bool form_class_is_of_interest = form_type_is_of_interest(LoadedFormClass::form_type);
   };

}