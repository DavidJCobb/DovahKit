#pragma once
#include <string_view>
#include "helpers/type_traits/is_std_array.h"
#include "dovah/form_reference_t.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"
#include "../common_lambdas.h"
#include "../property_definition.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<typename AccessFunc>
   struct localized_string_property_definition : public property_definition<
      AccessFunc,
      decltype(&checks::string),
      decltype(&pull::string),
      void (*)(lua_State* L, const dovah::localized_string&)
   > {
      //protected:
         static void _push(lua_State* L, const dovah::localized_string& v) {
            lua_pushstring(L, v.c_str());
         }

      public:
         constexpr localized_string_property_definition(std::string_view name, AccessFunc a) {
            this->name   = name;
            this->access = a;
            this->check  = &checks::string;
            this->pull   = &pull::string;
            this->push   = &_push;
         }
   };

   template<typename AccessFunc>
   localized_string_property_definition(std::string_view, AccessFunc) -> localized_string_property_definition<AccessFunc>;

   consteval auto define_localized_string_property(std::string_view name, auto accessor) {
      return localized_string_property_definition(name, accessor);
   }
}