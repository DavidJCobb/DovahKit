#pragma once
#include <string>
#include <vector>
#include "./helpers/name_equals.h"
#include "./inheritance_status.h"
#include "./property.h"

namespace dovah::papyrus {
   class scriptobject {
      public:
         std::string           scriptname;
         inheritance_status    inheritance;
         std::vector<property> properties;

         constexpr bool name_matches(const std::string_view& v) const {
            return helpers::name_equals(v, this->scriptname);
         }
         constexpr bool name_matches(const std::string& v) const {
            return helpers::name_equals(v, this->scriptname);
         }

         constexpr const property* lookup_property(const std::string_view&) const;
         constexpr property* lookup_property(const std::string_view&);
   };
}

#include "./scriptobject.inl"