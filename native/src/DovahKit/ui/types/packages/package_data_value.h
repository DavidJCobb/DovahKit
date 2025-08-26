#pragma once
#include <memory>
#include <variant>
#include "dovah/data/packages/package_data_type.h"
#include "./package_location.h"
#include "./package_target.h"
#include "./package_topic.h"
namespace dovah {
   namespace loaded_forms {
      namespace structs::custom_packages {
         class package_data;
      }
      class Form;
   }
   class form_stub;
}

namespace ui::types::packages {
   // Variant type indices are matched to dovah::packages::package_data_type.
   class package_data_value : public std::variant<
      bool,
      float,
      int32_t,
      package_location,
      float,
      package_target,
      package_target,
      package_topic
   > {
      public:
         using backend_type = dovah::loaded_forms::structs::custom_packages::package_data;

      public:
         using variant::variant;

      public:
         template<dovah::packages::package_data_type Type>
         constexpr const auto& as() const {
            return std::get<(size_t)Type>(*this);
         }

         template<dovah::packages::package_data_type Type>
         constexpr auto& as() {
            return std::get<(size_t)Type>(*this);
         }

         template<dovah::packages::package_data_type Type>
         constexpr auto& emplace() {
            return variant::emplace<(size_t)Type>();
         }

         template<dovah::packages::package_data_type Type>
         constexpr bool is() const noexcept {
            return this->index() == (size_t)Type;
         }

         constexpr dovah::packages::package_data_type type() const noexcept {
            if (this->valueless_by_exception())
               return dovah::packages::package_data_type::invalid;
            return (dovah::packages::package_data_type)this->index();
         }

         void importData(const backend_type&);
         std::unique_ptr<backend_type> exportData(dovah::loaded_forms::Form& dst_owner) const;

         void convert_to(dovah::packages::package_data_type);

         // Returns `true` if anything changes.
         bool sever_uses_of_form(dovah::form_stub&);
   };
}