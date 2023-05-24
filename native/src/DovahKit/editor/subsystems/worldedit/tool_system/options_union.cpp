#include "./options_union.h"

namespace dovahkit::subsystems::worldedit::tools {
   void options_union::_destroy_data() {
      if (this->tag == id_of_none)
         return;
      for (const auto& entry : _type_table) {
         if (entry.id == this->tag) {
            entry.destruct(*this);
            break;
         }
      }
   }

   /*static*/ options_union options_union::construct_for_type(tool_id id) {
      if (id == id_of_none)
         return {};
      for (const auto& entry : _type_table) {
         if (entry.id == id) {
            options_union out;
            out.tag = id;
            entry.construct(out);
            return out;
         }
      }
      return {};
   }

   options_union* options_union::clone() const {
      auto* copy = new options_union;
      copy->tag = this->tag;
      if (this->tag != id_of_none) {
         for (const auto& entry : _type_table) {
            if (entry.id != this->tag)
               continue;
            entry.construct_copy(*this, *copy);
            break;
         }
      }
      return copy;
   }
}