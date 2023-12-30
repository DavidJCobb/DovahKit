#pragma once
#include <string>
#include "./inheritance_status.h"
#include "./property_value.h"

namespace dovah::papyrus {
   class property {
      friend class scriptobject_list;
      friend class scriptobject;
      protected:
         std::string        _name;
         inheritance_status inheritance;
         //
      public:
         property_value value;

      public:
         constexpr const std::string_view name() const { return this->_name; }

         constexpr bool is_inherited() const { return this->inheritance.present_on_base; }
         constexpr bool is_inherited_and_removed() const { return this->inheritance.removed_on_target; }
         constexpr bool is_locally_defined() const { return this->inheritance.present_on_target && !this->inheritance.removed_on_target; }
   };
}