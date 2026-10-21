#pragma once
#include <string>
#include <variant>
#include "./bare_localized_string_id.h"
namespace dovah::tes {
   class file;
}

namespace dovah {
   template<auto LStringType>
   class managed_localized_string_use {
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
         constexpr const bool is_localized() const noexcept {
            return std::holds_alternative<std::string>(this->_value);
         }
   };
}