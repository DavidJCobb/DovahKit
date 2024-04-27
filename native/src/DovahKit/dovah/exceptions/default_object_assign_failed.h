#pragma once
#include <cstdint>
#include <stdexcept>

namespace dovah {
   struct default_object; // definition for known default objects

   class form_stub;
}

namespace dovah::exceptions {
   class default_object_assign_failed : public std::runtime_error {
      public:
         default_object_assign_failed() : std::runtime_error("Failed to set a default object.") {}
         
         uint32_t              dobj_signature   = 0;
         const default_object* known_definition = nullptr;
         form_stub*            used_form_stub   = nullptr;
   };
}