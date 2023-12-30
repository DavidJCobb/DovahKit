#pragma once
#include <vector>
#include "./scriptobject.h"

namespace dovah::loaded_forms {
   namespace components::papyrus {
      class attachment_data;
   }
   class Form;
}

namespace dovah::papyrus {
   class scriptobject_readwrite_passkey {
      friend class scriptobject_list;
      friend class loaded_forms::Form;
      private:
         constexpr scriptobject_readwrite_passkey() {}
   };

   class scriptobject_list {
      protected:
         std::vector<scriptobject> objects;

      public:
         constexpr scriptobject_list() {}
         scriptobject_list(scriptobject_readwrite_passkey, const loaded_forms::components::papyrus::attachment_data& target);
         scriptobject_list(scriptobject_readwrite_passkey, const loaded_forms::components::papyrus::attachment_data& target, const loaded_forms::components::papyrus::attachment_data& base);

         constexpr const scriptobject* get_script_by_name(const std::string_view&) const;
         constexpr scriptobject* get_script_by_name(const std::string_view&);

         // If the script is added, returns the new ScriptObject; else nullptr. Adding can fail 
         // if a script with this name is already present.
         constexpr scriptobject* add_script(const std::string_view& scriptname);

         constexpr void remove_script(const std::string_view& scriptname);

         constexpr size_t get_script_count() const;
         //
         constexpr const scriptobject* get_nth_script(size_t) const;
         constexpr scriptobject* get_nth_script(size_t);

         // These assume that the scripts on `target` were cleared first.
         void _overwrite(
            scriptobject_readwrite_passkey,
            loaded_forms::Form& target_form,
            loaded_forms::components::papyrus::attachment_data& target
         ) const;
         void _overwrite(
            scriptobject_readwrite_passkey,
            loaded_forms::Form& target_form,
            loaded_forms::components::papyrus::attachment_data& target,
            const loaded_forms::components::papyrus::attachment_data& base
         ) const;
   };
}

#include "./scriptobject_list.inl"