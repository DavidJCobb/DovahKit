#pragma once
#include <cstdint>
#include "../../base_list_size_too_large_to_serialize_error.h"

#include "dovah/forms/components/papyrus/attached_script.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_component::papyrus {
   class too_many_properties_on_script : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_properties_on_script(form_stub& subject, size_t size, const std::string& scriptname)
         :
            base_list_size_too_large_to_serialize_error(subject, size, dovah::loaded_forms::components::papyrus::attached_script::max_property_count),
            scriptname(scriptname)
         {}

         std::string scriptname;
   };
}
#include "../../../_util.undef.h"