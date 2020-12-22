#pragma once
#include "wrapper.h"

namespace editor_script {
   extern bool part_type_uses_name_key(part_type_t);

   extern const char* wrap_form(wrapper& out, dovah::form_stub*); // returns the appropriate metatable name to use
}