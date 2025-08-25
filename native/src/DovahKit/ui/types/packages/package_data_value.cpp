#include "./package_data_value.h"
#include "dovah/forms/structs/custom_packages/package_data/_all.h"

namespace {
   namespace custom_packages {
      using namespace dovah::loaded_forms::structs::custom_packages;
   }
   using package_data_type = dovah::packages::package_data_type;
}

namespace ui::types::packages {
   void package_data_value::importData(const backend_type& src) {
      if (auto* casted = dynamic_cast<const custom_packages::package_data_bool*>(&src)) {
         this->emplace<package_data_type::boolean>() = casted->value;
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_float*>(&src)) {
         this->emplace<package_data_type::float32>() = casted->value;
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_int*>(&src)) {
         this->emplace<package_data_type::integer>() = casted->value;
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_location*>(&src)) {
         this->emplace<package_data_type::location>().importData(casted->data);
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_object_list*>(&src)) {
         this->emplace<package_data_type::object_list>() = casted->value;
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_single_ref*>(&src)) {
         this->emplace<package_data_type::single_ref>().importData(casted->data);
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_target_selector*>(&src)) {
         this->emplace<package_data_type::target_selector>().importData(casted->data);
         return;
      }
      if (auto* casted = dynamic_cast<const custom_packages::package_data_topic*>(&src)) {
         this->emplace<package_data_type::topic>().importData(casted->data);
         return;
      }
      //
      // Safety fallback for unknown-type data loaded from a malformed or future-format file:
      //
      this->emplace<package_data_type::boolean>() = false;
   }
   std::unique_ptr<package_data_value::backend_type> package_data_value::exportData(dovah::loaded_forms::Form& dst_owner) const {
      if (this->is<package_data_type::boolean>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_bool>();
         dst_ptr->value = this->as<package_data_type::boolean>();
         return dst_ptr;
      }
      if (this->is<package_data_type::float32>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_float>();
         dst_ptr->value = this->as<package_data_type::float32>();
         return dst_ptr;
      }
      if (this->is<package_data_type::integer>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_int>();
         dst_ptr->value = this->as<package_data_type::integer>();
         return dst_ptr;
      }
      if (this->is<package_data_type::location>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_location>();
         this->as<package_data_type::location>().exportData(dst_ptr->data, dst_owner);
         return dst_ptr;
      }
      if (this->is<package_data_type::integer>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_object_list>();
         dst_ptr->value = this->as<package_data_type::object_list>();
         return dst_ptr;
      }
      if (this->is<package_data_type::location>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_single_ref>();
         this->as<package_data_type::single_ref>().exportData(dst_ptr->data, dst_owner);
         return dst_ptr;
      }
      if (this->is<package_data_type::location>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_target_selector>();
         this->as<package_data_type::target_selector>().exportData(dst_ptr->data, dst_owner);
         return dst_ptr;
      }
      if (this->is<package_data_type::location>()) {
         auto dst_ptr = std::make_unique<custom_packages::package_data_topic>();
         this->as<package_data_type::topic>().exportData(dst_ptr->data, dst_owner);
         return dst_ptr;
      }
      assert(false && "unhandled variant permutation in ui::types::packages::package_data_value!");
   }

   bool package_data_value::sever_uses_of_form(dovah::form_stub& stub) {
      switch (this->type()) {
         case dovah::packages::package_data_type::location:
            return this->as<dovah::packages::package_data_type::location>().sever_uses_of_form(stub);
            break;
         case dovah::packages::package_data_type::single_ref:
            return this->as<dovah::packages::package_data_type::single_ref>().sever_uses_of_form(stub);
            break;
         case dovah::packages::package_data_type::target_selector:
            return this->as<dovah::packages::package_data_type::single_ref>().sever_uses_of_form(stub);
            break;
         case dovah::packages::package_data_type::topic:
            return this->as<dovah::packages::package_data_type::topic>().sever_uses_of_form(stub);
            break;
      }
      return false;
   }
}