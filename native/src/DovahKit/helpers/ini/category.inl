#pragma once
#include "./category.h"
#include "../string/strieq_ascii.h"
#include "./setting.h"

namespace cobb::ini {
   constexpr setting* category::setting_by_name(std::string_view name) const {
      for (auto* s : this->_settings)
         if (::cobb::strieq_ascii(name, s->name))
            return s;
      return nullptr;
   }

   constexpr void category::add_setting_change_callback(setting& s, setting_change_callback f) {
      this->_change_callbacks.push_back({ f, &s });
   }
   constexpr void category::remove_setting_change_callback(setting& s, setting_change_callback f) {
      _stored_change_callback desired = { f, &s };

      auto& list  = this->_change_callbacks;
      auto  first = std::find(list.begin(), list.end(), desired);
      if (first != list.end())
         list.erase(first);
   }

   constexpr void category::_on_setting_instantiated(::cobb::passkey<category, setting>, setting& s) {
      this->_settings.push_back(&s);
   }

   constexpr void category::_on_setting_changed(::cobb::passkey<category, setting>, setting& s, value_union old_value, value_union new_value) {
      for (const auto& entry : this->_change_callbacks)
         if (entry.target == &s)
            (entry.callback)(s, old_value, new_value);
      this->_forward_change_notif(s, old_value, new_value);
   }
}
