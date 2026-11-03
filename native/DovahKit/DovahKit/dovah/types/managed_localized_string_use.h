#pragma once
#include <string>
#include <variant>
#include "./bare_localized_string_id.h"
#include "./localized_string_type.h"
namespace dovah {
   namespace tes {
      class file;
      class subrecord_view;
   }
   class form_stub;
   class loaded_game_setting;
}

namespace dovah {
   class _base_managed_localized_string_use {
      protected:
         struct _localized_id {
            const tes::file*         file = nullptr; // TES file from which this string use is loaded
            bare_localized_string_id id   = 0;       // ID within the corresponding string file
         };

      protected:
         std::variant<
            std::monostate, // edge-case: the string never appeared in the winning record; nothing was loaded
            std::string,
            _localized_id
         > _value;

      private:
         void _discard_localized_id(form_stub& containing_form, localized_string_type);

      protected:
         void _set_localized_id(form_stub& containing_form, localized_string_type, bare_localized_string_id);
         void _set_non_localized_string(form_stub& containing_form, localized_string_type, std::string_view);
         void _set_non_localized_string(form_stub& containing_form, localized_string_type, std::string&&);

      public:
         constexpr bool empty() const noexcept {
            if (std::holds_alternative<std::monostate>(this->_value))
               return true;
            if (std::holds_alternative<std::string>(this->_value))
               return std::get<std::string>(this->_value).empty();
            if (std::holds_alternative<_localized_id>(this->_value)) {
               auto& info = std::get<_localized_id>(this->_value);
               return (!info.file || info.id == 0);
            }
            std::unreachable();
         }
         constexpr bare_localized_string_id get_localized_id() const noexcept {
            if (std::holds_alternative<_localized_id>(this->_value))
               return std::get<_localized_id>(this->_value).id;
            return 0;
         }
         bool is_active_file_string_id() const noexcept;
         constexpr bool is_localized() const noexcept {
            return std::holds_alternative<_localized_id>(this->_value);
         }

         bool stream(tes::subrecord_view& subrecord);
   };

   template<auto LStringType>
   class managed_localized_string_use : public _base_managed_localized_string_use {
      public:
         void set_localized_id(form_stub& containing_form, bare_localized_string_id id) {
            _base_managed_localized_string_use::_set_localized_id(containing_form, LStringType, id);
         }
         void set_non_localized_string(form_stub& containing_form, std::string_view v) {
            _base_managed_localized_string_use::_set_non_localized_string(containing_form, LStringType, v);
         }
         void set_non_localized_string(form_stub& containing_form, std::string&& v) {
            _base_managed_localized_string_use::_set_non_localized_string(containing_form, LStringType, std::move(v));
         }
   };
}