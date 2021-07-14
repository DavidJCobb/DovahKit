#include "ini_settings.h"
#include <cassert>

namespace dovah::game_ini {
   #pragma region Class internals
   extern setting_type get_setting_type_from_name(const char* name) {
      if (!name)
         return setting_type::none;
      switch (name[0]) {
         case 'b':
         case 'B':
            return setting_type::boolean;
         case 'f':
         case 'F':
            return setting_type::float32;
         case 'i':
         case 'I':
            return setting_type::integer;
         case 's':
         case 'S':
            return setting_type::string;
      }
      return setting_type::none;
   }

   setting_definition::setting_definition(const char* n, bool value) : name(n), type(setting_type::boolean) {
      this->default_value.b = value;
      this->current_value.b = value;
   }
   setting_definition::setting_definition(const char* n, float value) : name(n), type(setting_type::float32) {
      this->default_value.f = value;
      this->current_value.f = value;
   }
   setting_definition::setting_definition(const char* n, int32_t value) : name(n), type(setting_type::integer) {
      this->default_value.i = value;
      this->current_value.i = value;
   }
   setting_definition::setting_definition(const char* n, const char* value) : name(n), type(setting_type::string) {
      this->default_value.s = value;
      this->current_value.s = value;
   }

   void setting_definition::_set_games(std::initializer_list<game>& g) {
      this->games.skyrim_classic = false;
      this->games.skyrim_special = false;
      for (auto v : g) {
         switch (v) {
            case game::skyrim_classic: this->games.skyrim_classic = true; break;
            case game::skyrim_special: this->games.skyrim_special = true; break;
            default:
               assert(false && "dovah::game_ini::setting_definition: Unrecognized game passed to constructor!");
               //
               // Add a member to (setting_definition::games), and then reset it to (false) at the start of 
               // this function and add a case for it to this switch. Be sure to update the (exists_in_game) 
               // member function, too!
               //
               __assume(0); // MSVC: unreachable
         }
      }
   }

   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, bool value) : setting_definition(n, value) {
      this->_set_games(g);
   }
   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, float value) : setting_definition(n, value) {
      this->_set_games(g);
   }
   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, int32_t value) : setting_definition(n, value) {
      this->_set_games(g);
   }
   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, const char* value) : setting_definition(n, value) {
      this->_set_games(g);
   }

   bool setting_definition::exists_in_game(game g) const noexcept {
      switch (g) {
         case game::skyrim_classic: return this->games.skyrim_classic; break;
         case game::skyrim_special: return this->games.skyrim_special; break;
      }
      return false;
   }

   section_definition::section_definition(const char* name, std::initializer_list<setting_definition> settings) : name(name), settings(settings) {
   }

   file_definition::file_definition(const char* name, std::initializer_list<section_definition> sections) : filename(name), sections(sections) {
   }
   const setting_definition* file_definition::lookup(const char* section, const char* setting) const noexcept {
      for (auto& a : this->sections) {
         if (stricmp(a.name.c_str(), section) != 0)
            continue;
         for (auto& b : a.settings) {
            if (stricmp(b.name, setting) == 0)
               return &b;
         }
         return nullptr;
      }
      return nullptr;
   }
   #pragma endregion

   namespace files {
      extern const file_definition skyrim = file_definition("Skyrim.ini", {
         section_definition("Landscape", {
            { "sDefaultLandDiffuseTexture", "Dirt02.dds" },
            { "sDefaultLandNormalTexture",  "Dirt02_N.dds" },
         }),
      });
      //
      // TODO: Dump all INI settings from the executable, along with their program-level defaults.
      //
   }
}