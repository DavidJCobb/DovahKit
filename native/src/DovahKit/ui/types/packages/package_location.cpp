#include "./package_location.h"
#include "dovah/forms/structs/package_location.h"

#pragma push_macro("SWITCH")
//
// Define the CASE and CASE_FORM macros before using the SWITCH macro.
// The case macros should take a single parameter: the `location_type` 
// value to act on.
//
#undef SWITCH
#define SWITCH(of) \
   switch ((of).get_type()) { \
      CASE_FORM(location_type::reference); \
      CASE_FORM(location_type::interior_cell); \
      CASE(location_type::near_package_start_location); \
      CASE(location_type::near_editor_location); \
      CASE_FORM(location_type::object); \
      CASE(location_type::object_type); \
      CASE_FORM(location_type::linked_ref); \
      CASE(location_type::at_package_location); \
      CASE(location_type::reference_alias); \
      CASE(location_type::location_alias); \
      CASE(location_type::interrupt_override_target); \
      CASE(location_type::package_data_target); \
      CASE(location_type::self); \
   }

namespace ui::types::packages {
   void package_location::importData(const backend_type& src) {
      this->radius = src.radius;
      
      #pragma push_macro("CASE")
      #pragma push_macro("CASE_FORM")
      #define CASE(v)      case v: this->data.emplace<(size_t)v>() = std::get<(size_t)v>(src.data); break;
      #define CASE_FORM(v) case v: this->data.emplace<(size_t)v>() = std::get<(size_t)v>(src.data).get_form_stub(); break;
      SWITCH(src);
      #undef CASE
      #undef CASE_FORM
      #pragma pop_macro("CASE")
      #pragma pop_macro("CASE_FORM")
   }
   void package_location::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      dst.radius = this->radius;

      //
      // Clear forms out of the destination.
      // 
      // (Since `form_reference_t` occurs multiple times in the variant, we can't just 
      // `std::get` it by type.)
      //
      #pragma push_macro("CASE")
      #pragma push_macro("CASE_FORM")
      #define CASE(v)      case v: break;
      #define CASE_FORM(v) case v: std::get<(size_t)v>(dst.data).set(dst_owner, nullptr); break;
      SWITCH(dst);
      #undef CASE
      #undef CASE_FORM
      #pragma pop_macro("CASE")
      #pragma pop_macro("CASE_FORM")

      //
      // Now we can more easily set the value.
      //
      #pragma push_macro("CASE")
      #pragma push_macro("CASE_FORM")
      #define CASE(v)      case v: dst.data.emplace<(size_t)v>() = std::get<(size_t)v>(this->data); break;
      #define CASE_FORM(v) case v: dst.data.emplace<(size_t)v>().set(dst_owner, std::get<(size_t)v>(this->data)); break;
      SWITCH(*this);
      #undef CASE
      #undef CASE_FORM
      #pragma pop_macro("CASE")
      #pragma pop_macro("CASE_FORM")
   }

   bool package_location::sever_uses_of_form(dovah::form_stub& stub) {
      #pragma push_macro("CASE")
      #pragma push_macro("CASE_FORM")
      #define CASE(v)      case v: break;
      #define CASE_FORM(v)\
         case v: \
            { \
               auto& ptr = std::get<(size_t)v>(this->data); \
               if (ptr == &stub) { \
                  ptr = nullptr; \
                  return true; \
               } \
            } \
            break;
      SWITCH(*this);
      #undef CASE
      #undef CASE_FORM
      #pragma pop_macro("CASE")
      #pragma pop_macro("CASE_FORM")
      return false;
   }
}

#undef SWITCH
#pragma pop_macro("SWITCH")