#pragma once
#include <string>
#include <string_view>
#include <variant>
#include "./bare_localized_string_id.h"
namespace dovah::tes {
   class file;
}

namespace dovah {
   template<auto LStringType>
   class unmanaged_localized_string_use {
      protected:
         struct _localized_id {
            const tes::file*         file = nullptr; // TES file from which this string use is loaded
            bare_localized_string_id id   = 0;       // ID within the corresponding string file
         };

      protected:
         std::variant<
            std::string,
            _localized_id
         > _value;

      public:
         constexpr bare_localized_string_id get_localized_id() const noexcept {
            if (std::holds_alternative<_localized_id>(this->_value))
               return std::get<_localized_id>(this->_value).id;
            return 0;
         }
         constexpr std::string_view get_non_localized_text() const noexcept {
            if (std::holds_alternative<std::string>(this->_value))
               return std::get<std::string>(this->_value).id;
            return {};
         }
         constexpr const bool is_localized() const noexcept {
            return std::holds_alternative<std::string>(this->_value);
         }
         void set_localized_id(const tes::file& tes_file, bare_localized_string_id id) {
            this->_value = _localized_id{ &tes_file, id };
         }
         void set_non_localized_string(std::string_view v) {
            this->_value = std::string(v);
         }
         void set_non_localized_string(std::string&& v) {
            this->_value = std::move(v);
         }
   };
}