#pragma once
#include <cstdint>
#include "../core.h"

namespace dovah {
   struct default_object {
      uint32_t    signature = 0;
      form_type_t type      = form_type::none;

      default_object(uint32_t s, form_type_t f) : signature(s), type(f) {}
   };
   extern const std::array<default_object, 0x15A> default_objects;

   extern const default_object* get_default_object_definition(uint32_t signature);
}