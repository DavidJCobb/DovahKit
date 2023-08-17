#pragma once
#include <ostream>
#include <string_view>
#include <vector>
#include "../passkey.h"
#include "./types.h"

namespace cobb::ini {
   class file;
   class setting;
}

namespace cobb::ini {
   // Instances should use the `constinit` specifier.
   class category {
      public:
         using setting_change_callback = void(*)(setting&, value_union old_value, value_union new_value);

      protected:
         struct _stored_change_callback {
            setting_change_callback callback = nullptr;
            const setting* target = nullptr;
         };

      public:
         template<bool Dummy = true>
         constexpr category(file& f, const char* n) : owner(f), name(n) {
            f._on_category_instantiated({}, *this);
         }

      public:
         file& owner;
         const char* const name;
      protected:
         std::vector<setting*> _settings; // unowned pointers (the settings should be `constinit` or otherwise have static storage duration)
         std::vector<_stored_change_callback> _change_callbacks;

      public:
         constexpr const std::vector<setting*>& settings() const noexcept {
            return this->_settings;
         }

         // This primarily exists for use during serialization and parsing of INI files. You are 
         // strongly encouraged to define your settings as `constinit` values that can be accessed 
         // directly, to avoid run-time name lookups.
         constexpr setting* setting_by_name(std::string_view name) const;

         constexpr void _on_setting_instantiated(::cobb::passkey<category, setting>, setting&);
         constexpr void _on_setting_changed(::cobb::passkey<category, setting>, setting&, value_union old_value, value_union new_value);
   };
}

#include "./category.inl"