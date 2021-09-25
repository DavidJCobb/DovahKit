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
#include <array>
#include <cstdint>
#include "../core.h"
#include "../localized_strings.h"
#include "../utils/game_list.h"

namespace dovah {
   enum class game_setting_type {
      none = -1,
      boolean,
      float32,
      integer,
      string,
   };

   extern constexpr game_setting_type get_game_setting_type_from_name(const char* name);

   struct game_setting_value {
      union {
         bool    b;
         float   f;
         int32_t i = 0;
      };
      localized_string s;
   };

   class game_setting_definition {
      public:
         const char*        name = "";
         game_setting_value default_value;
         game_list games;
         
         template<typename T> constexpr game_setting_definition(const char* n, T value) : name(n), default_value(value), games(game_list::from_all()) {}
         template<typename T> constexpr game_setting_definition(game_list g, const char* n, T value) : name(n), default_value(value), games(g) {}

         static const game_setting_definition* lookup(const char* name) noexcept;

         inline constexpr bool exists_in_game(game g) const noexcept { return this->games.contains(g); }
         inline constexpr game_setting_type type() const noexcept { return get_game_setting_type_from_name(this->name); }
   };

   extern const std::array<game_setting_definition, 3608> game_settings;
}