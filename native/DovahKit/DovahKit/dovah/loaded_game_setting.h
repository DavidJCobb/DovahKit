#pragma once
#include <string>
#include <variant>
#include <vector>
#include "lu/passkey.h"
#include "./localization/category.h"
#include "./types/bare_form_id.h"
#include "./types/game_settings/value_variant.h"
namespace dovah {
   namespace facts::game_settings {
      class definition;
   }
   namespace localization {
      class use_node;
   }
   namespace tes {
      class file;
   }
   class game_setting_collection;

   template<localization::category LStringType>
   class localized_string_id;
}

namespace dovah {
   class loaded_game_setting {
      friend game_setting_collection;
      protected:
         using value_variant = types::game_settings::value_variant;

      public:
         struct source_file_info {
            const tes::file* source_file   = nullptr;
            uint32_t         record_offset = 0;
            bare_form_id     form_id       = 0; // may be zero if the GMST's record ID did not resolve to a valid form ID
         };

      protected:
         game_setting_collection*                _owner      = nullptr;
         const facts::game_settings::definition* _definition = nullptr;
         struct {
            std::string loaded;    // needed separately from the definition so we can track GMSTs with invalid/no names
            std::string lowercase; // for faster lookups from maps, etc.
         } _name;
         std::vector<source_file_info> _source_files;
         struct {
            value_variant           variant;
            localization::use_node* string_use_info = nullptr;
         } _value;

      public:
         constexpr const facts::game_settings::definition* definition() const noexcept;
         constexpr const std::string& name() const noexcept;
         constexpr const std::string& name_in_lowercase() const noexcept;
         constexpr const std::vector<source_file_info>& source_file_list() const noexcept;

         constexpr const value_variant& get_value_variant() const noexcept;

         bare_form_id form_id() const noexcept;

         // These assert that the typing is correct and that there's an active file.
         // These may throw, if the GMST doesn't already have a form ID and no form stub can be created.
         void set_value(bool);
         void set_value(float);
         void set_value(int32_t);
         // Above preconditions, but also assert on writing localized strings to a non-localized active 
         // file or vice versa.
         void set_value(localized_string_id<localization::category::general>);
         void set_value(std::string&&);

      protected:
         // If the GMST was already defined in this source file, with a valid form ID, then this returns 
         // that form ID. The GMST will be moved from that form ID, so any corresponding form stub for 
         // that form ID will need to be cleaned up.
         bare_form_id _on_record_loaded(
            lu::passkey<game_setting_collection>,
            const tes::file&   source_file,
            uint32_t           record_offset,
            bare_form_id       form_id,
            const std::string& name,
            const std::string& name_lowercase,
            value_variant&&    value
         );

         void _on_value_edited();
   };
}

#include "./loaded_game_setting.inl"