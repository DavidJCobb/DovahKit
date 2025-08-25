#include "./package_topic.h"
#include "dovah/forms/structs/package_topic.h"

namespace ui::types::packages {
   void package_topic::importData(const backend_type& src) {
      if (std::holds_alternative<dovah::form_reference_t>(src.data)) {
         this->data = std::get<dovah::form_reference_t>(src.data).get_form_stub();
      } else if (std::holds_alternative<uint32_t>(src.data)) {
         this->data = std::get<uint32_t>(src.data);
      }
   }
   void package_topic::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      if (std::holds_alternative<dovah::form_reference_t>(dst.data)) {
         std::get<dovah::form_reference_t>(dst.data).set(dst_owner, nullptr);
      }

      if (std::holds_alternative<dovah::form_stub*>(this->data)) {
         dst.data.emplace<dovah::form_reference_t>().set(dst_owner, std::get<dovah::form_stub*>(this->data));
      } else if (std::holds_alternative<uint32_t>(this->data)) {
         dst.data = std::get<uint32_t>(this->data);
      }
   }

   bool package_topic::sever_uses_of_form(dovah::form_stub& stub) {
      if (std::holds_alternative<dovah::form_stub*>(this->data)) {
         auto& item = std::get<dovah::form_stub*>(this->data);
         if (item == &stub) {
            item = nullptr;
            return true;
         }
      }
      return false;
   }
}