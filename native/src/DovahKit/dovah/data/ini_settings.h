#pragma once
#include <cstdint>
#include <string>
#include "../core.h"

namespace dovah {
   namespace game_ini {
      enum class setting_type {
         none = -1,
         boolean,
         float32,
         integer,
         string,
      };
      extern setting_type get_setting_type_from_name(const char* name);
      
      struct setting_value {
         union {
            bool    b;
            float   f;
            int32_t i = 0;
         };
         std::string s;
      };

      class setting_definition {
         public:
            const char* const  name = "";
            const setting_type type = setting_type::none;
            setting_value default_value;
            setting_value current_value;
            struct {
               bool skyrim_classic = true;
               bool skyrim_special = true;
            } games;
            
            setting_definition(setting_type t) : type(t) {}
            setting_definition(const char* n, bool value);
            setting_definition(const char* n, float value);
            setting_definition(const char* n, int32_t value);
            setting_definition(const char* n, const char* value);
            setting_definition(std::initializer_list<game>, const char* n, bool value);
            setting_definition(std::initializer_list<game>, const char* n, float value);
            setting_definition(std::initializer_list<game>, const char* n, int32_t value);
            setting_definition(std::initializer_list<game>, const char* n, const char* value);
            //
            inline bool is_none() const noexcept { return this->type == setting_type::none; }
            bool exists_in_game(game) const noexcept;
            
         protected:
            void _set_games(std::initializer_list<game>&);
      };

      struct section_definition {
         std::string name;
         std::vector<setting_definition> settings;

         section_definition(const char* name, std::initializer_list<setting_definition>);
      };

      struct file_definition {
         std::string filename;
         std::vector<section_definition> sections;

         file_definition(const char* name, std::initializer_list<section_definition>);

         const setting_definition* lookup(const char* section, const char* setting) const noexcept;
      };

      namespace files {
         extern const file_definition skyrim;
      }
   }
}