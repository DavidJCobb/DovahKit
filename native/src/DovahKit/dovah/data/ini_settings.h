/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../utils/game_list.h"

namespace dovah {
   namespace game_ini {
      enum class setting_type {
         none = -1,
         boolean,
         float32,
         integer,
         integer_unsigned,
         string,
      };
      extern constexpr setting_type get_setting_type_from_name(const char* name);
      
      struct setting_value {
         union {
            bool     b;
            float    f;
            int32_t  i = 0;
            uint32_t u;
         };
         const char* const s = "";

         constexpr setting_value() {}
         constexpr explicit setting_value(bool v) : b(v) {}
         constexpr explicit setting_value(float v) : f(v) {}
         constexpr explicit setting_value(const char* v) : s(v) {}
         constexpr setting_value(int32_t v) : i(v) {}
      };

      class setting_definition {
         public:
            const char* const   name = "";
            const setting_value default_value;
            game_list games;

            constexpr setting_definition() {}

            template<typename T> constexpr setting_definition(const char* n, T value) : name(n), default_value(value), games(game_list::from_all()) {}
            template<typename T> constexpr setting_definition(game_list g, const char* n, T value) : name(n), default_value(value), games(g) {}
            
            inline constexpr bool exists_in_game(game g) const noexcept { return this->games.contains(g); }
            inline constexpr setting_type type() const noexcept { return get_setting_type_from_name(this->name); }
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
         extern const file_definition skyrim_prefs;
      }
   }
}