#include "./options_union.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ options_union options_union::construct_for_type(tool_id id) {
      if (id == id_of_none)
         return {};
      options_union out;
      all_tools_with_options::for_each_until_true([id, &out]<typename Tool>() {
         if constexpr (!tool_with_options_member_type<Tool>)
            return false;
         if (id_of<Tool> == id) {
            out.emplace<typename Tool::options>();
            return true;
         }
         return false;
      });
      return out;
   }

   options_union* options_union::clone() const {
      auto* copy = new options_union;
      *copy = *this;
      return copy;
   }
}