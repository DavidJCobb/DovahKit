#pragma once
#include <cstdint>
#include <variant>
namespace dovah {
   namespace loaded_forms {
      namespace structs {
         struct package_topic;
      }
      class Form;
   }
   class form_stub;
}

namespace ui::types::packages {
   struct package_topic {
      public:
         using backend_type = dovah::loaded_forms::structs::package_topic;

      public:
         std::variant<
            dovah::form_stub*,
            uint32_t
         > data;

      public:
         constexpr dovah::form_stub* get_topic() const noexcept {
            if (auto* maybe = std::get_if<dovah::form_stub*>(&this->data))
               return *maybe;
            return nullptr;
         }
         constexpr uint32_t get_subtype_signature() const noexcept {
            if (auto* maybe = std::get_if<uint32_t>(&this->data))
               return *maybe;
            return 0;
         }

         constexpr void set_topic(dovah::form_stub* v) noexcept {
            this->data = v;
         }
         constexpr void set_subtype_signature(uint32_t v) noexcept {
            this->data = v;
         }

         void importData(const backend_type&);
         void exportData(backend_type&, dovah::loaded_forms::Form& owner) const;

         // Returns true if anything changes.
         bool sever_uses_of_form(dovah::form_stub&);
   };
}