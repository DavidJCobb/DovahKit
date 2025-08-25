#include "./package_target.h"
#include "dovah/forms/structs/package_target.h"

#pragma push_macro("SWITCH")
//
// Define the CASE and CASE_FORM macros before using the SWITCH macro.
// The case macros should take a single parameter: the `location_type` 
// value to act on.
//
#undef SWITCH
#define SWITCH(of) \
   switch ((of).get_type()) { \
      CASE_FORM(target_type::reference); \
      CASE_FORM(target_type::object); \
      CASE(target_type::object_type); \
      CASE_FORM(target_type::linked_ref); \
      CASE(target_type::reference_alias); \
      CASE(target_type::interrupt_override_target); \
      CASE(target_type::self); \
   }

namespace ui::types::packages {
   void package_target::importData(const backend_type& src) {
      this->distance = src.distance;
      
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
   void package_target::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      dst.distance = this->distance;

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

   bool package_target::sever_uses_of_form(dovah::form_stub& stub) {
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