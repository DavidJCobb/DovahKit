#include "color.h"
#include <array>
#include "../../../../lua.h"
#include "../../../../helpers/strings.h"
#include "../../../../helpers/qt/string_scanner.h"

struct lua_State;

namespace {
   void _pull_from_table(QColor& color, lua_State* L, int index, int top, uint8_t& found) {
      found = 0;
      //
      int isnum;
      int value;
      std::array<int, 4> values = { 0, 0, 0, 255 };
      //
      constexpr std::array names = { "r", "g", "b" };
      for (int i = 1; i <= names.size(); ++i) {
         lua_getfield(L, index, names[i - 1]);
         isnum;
         value = lua_tointegerx(L, top + 1, &isnum);
         lua_pop(L, 1);
         if (!isnum) {
            lua_geti(L, index, i);
            value = lua_tointegerx(L, top + 1, &isnum);
            lua_pop(L, 1);
         }
         if (value < 0 || value > 255) {
            luaL_error(L, "value %d is out of range for the %s color component", value, names[i - 1]);
         }
         values[i - 1] = value;
         if (isnum)
            found |= 1 << i;
      }
      color.setRgb(values[0], values[1], values[2]);
      //
      lua_getfield(L, index, "a");
      value = lua_tointegerx(L, top + 1, &isnum);
      lua_pop(L, 1);
      if (isnum) {
         values[3] = value;
      } else {
         lua_geti(L, index, 4);
         value = lua_tointegerx(L, top + 1, &isnum);
         lua_pop(L, 1);
         if (isnum)
            values[3] = value;
      }
      if (value < 0 || value > 255) {
         luaL_error(L, "value %d is out of range for the alpha color component", value);
      }
      color.setAlpha(values[3]);
   }
   void _pull_from_string(QColor& color, lua_State* L, int index) {
      std::array<int, 4> values = { 0, 0, 0, 255 };
      //
      auto raw = QString::fromUtf8(lua_tostring(L, index)).trimmed().toLower();
      //
      // Check for HTML hex colors.
      //
      if (raw[0] == '#') {
         //
         // Hex digits or octets (#ABC, #AABBCC, #ABCD, or #AABBCCDD).
         //
         int digits;
         switch (raw.size() - 1) { // don't forget to ditch the #
            case 3: // short RGB
            case 4: // short RGBA
               digits = 1;
               break;
            case 6: // long  RGB
            case 8: // long  RGBA
               digits = 2;
               break;
            default:
               luaL_error(L, "syntax for hex-color is incorrect: provide a `#` sign followed by either 3, 4, 6, or 8 hexadecimal digits");
         }
         bool isnum;
         for (int i = 0; i < 4; ++i) {
            QString octet;
            {
               int j = (i * digits) + 1;
               if (j + digits > raw.size())
                  break;
               octet = raw.mid(j, digits);
               if (digits == 1)
                  octet += octet[0];
            }
            values[i] = octet.toInt(&isnum, 16);
            if (!isnum)
               luaL_error(L, "syntax for hex-color is incorrect: provide a `#` sign followed by either 3, 4, 6, or 8 hexadecimal digits");
         }
         color.setRgb(values[0], values[1], values[2], values[3]);
         return;
      }
      //
      // Check for CSS function-style rgb(a) or hsl(a) colors.
      //
      bool css_rgb = raw.startsWith("rgb");
      bool css_hsl = raw.startsWith("hsl");
      if (css_rgb || css_hsl) {
         bool alpha   = false;
         bool comma   = false;
         bool percent = false;
         //
         // I wish I could use a loop to pull the color components. The problem is that for RGB(A) colors, 
         // each component is similar enough in syntax to tempt you into using a loop, but just different 
         // enough that you can't:
         // 
         //  - R differs from G and B in that it is the deciding factor in whether the components should 
         //    use percentage values or not.
         // 
         //  - B differs from R and G in that it might not be followed by a separator (e.g. ','); depending 
         //    on whether this is an RGB or RGBA color.
         // 
         //  - A differs from R, G, and B both in its range, [0, 1], and in its ability to use or not use 
         //    a percentage independently of R, G, and B.
         // 
         //  - Additionally, since we're using the same parsing for HSL, "R" doubles for "H" and so differs 
         //    in how it handles units of angle measurement.
         //
         cobb::qt::string_scanner scanner(raw);
         scanner.skip(3); // "rgb" or "hsl"
         if (scanner.extract_specific_char('a', true))
            alpha = true;
         if (!scanner.extract_specific_char('('))
            luaL_error(L, "syntax for function-color is incorrect: expected `(`");
         //
         double v;
         if (!scanner.extract_double(v))
            luaL_error(L, "syntax for function-color is incorrect: expected number");
         if (css_hsl) {
            if (scanner.extract_specific_substring("deg", true)) {
               ; // degrees are the default for HSL "H" values
            } else if (scanner.extract_specific_substring("rad", true)) {
               v /= 57.295779513082320876798154814105; // 180 / PI
            } else if (scanner.extract_specific_substring("grad", true)) {
               v *= 0.9;
            } else if (scanner.extract_specific_substring("turn", true)) {
               v *= 360.0;
            }
         } else {
            if (scanner.extract_specific_char('%', true)) {
               percent = true;
               v *= 2.55;
            }
         }
         values[0] = std::round(v);
         //
         if (scanner.extract_specific_char(','))
            comma = true;
         //
         if (!scanner.extract_double(v))
            luaL_error(L, "syntax for function-color is incorrect: expected number");
         if (percent || css_hsl) {
            if (!scanner.extract_specific_char('%', true)) {
               if (css_hsl)
                  luaL_error(L, "syntax for function-color is incorrect: HSL saturation values must be percentages");
               else
                  luaL_error(L, "syntax for function-color is incorrect: do not mix and match percentage and non-percentage RGB values");
            }
            v *= 2.55;
         } else {
            if (scanner.extract_specific_char('%', true))
               luaL_error(L, "syntax for function-color is incorrect: do not mix and match percentage and non-percentage RGB values");
         }
         values[1] = std::round(v);
         if (comma)
            if (!scanner.extract_specific_char(','))
               luaL_error(L, "syntax for function-color is incorrect: expected `,`");
         //
         if (!scanner.extract_double(v))
            luaL_error(L, "syntax for function-color is incorrect: expected number");
         if (percent || css_hsl) {
            if (!scanner.extract_specific_char('%', true)) {
               if (css_hsl)
                  luaL_error(L, "syntax for function-color is incorrect: HSL lightness values must be percentages");
               else
                  luaL_error(L, "syntax for function-color is incorrect: do not mix and match percentage and non-percentage RGB values");
            }
            v *= 2.55;
         } else {
            if (scanner.extract_specific_char('%', true))
               luaL_error(L, "syntax for function-color is incorrect: do not mix and match percentage and non-percentage RGB values");
         }
         values[2] = std::round(v);
         //
         if (alpha) {
            QChar desired = comma ? ',' : '/';
            if (!scanner.extract_specific_char(desired))
               luaL_error(L, "syntax for function-color is incorrect: expected `%s`", QString(desired).toUtf8());
            //
            if (!scanner.extract_double(v))
               luaL_error(L, "syntax for function-color is incorrect: expected number (alpha)");
            if (scanner.extract_specific_char('%', true)) // alpha can be a percentage even if the other values are not
               v *= 2.55;
            else
               v *= 255.0;
            values[3] = std::round(v);
         }
         //
         if (!scanner.extract_specific_char(')'))
            luaL_error(L, "syntax for function-color is incorrect: expected `)`");
         //
         if (!scanner.is_at_effective_end()) {
            luaL_error(L, "syntax for function-color is incorrect: unexpected content after the color");
         }
         //
         if (css_hsl) {
            values[0] %= 360;
            for (int i = 1; i < 4; ++i)
               values[i] = std::clamp(values[i], 0, 255);
            color.setHsl(values[0], values[1], values[2], values[3]);
            return;
         }
         for (int i = 0; i < 4; ++i)
            values[i] = std::clamp(values[i], 0, 255);
         color.setRgb(values[0], values[1], values[2], values[3]);
         return;
      }
      //
      // Named color.
      //
      color.setNamedColor(raw);
      if (!color.isValid())
         luaL_error(L, "the provided string is not a recognized color name");
   }

   int _pcallable_pull(lua_State* L) {
      assert(lua_islightuserdata(L, 2));
      auto* out = (QColor*) lua_touserdata(L, 2);
      *out = editor_script::util::ui::pull_color(L, 1);
      return 0;
   }
}

namespace editor_script::util::ui {
   extern void push_color(lua_State* L, const QColor& color) {
      lua_createtable(L, 4, 4);
      auto pos = lua_gettop(L);
      //
      std::array names = { "r", "g", "b", "a" };
      std::array parts = { color.red(), color.green(), color.blue(), color.alpha() };
      for (int i = 0; i < names.size(); ++i) {
         lua_pushinteger(L, parts[i]);
         lua_seti(L, pos, i + 1);
         lua_pushinteger(L, parts[i]);
         lua_setfield(L, pos, names[i]);
      }
   }
   extern [[nodiscard]] QColor pull_color(lua_State* L, int index) {
      QColor color;
      //
      index = lua_absindex(L, index);
      auto top = lua_gettop(L);
      switch (lua_type(L, index)) {
         case LUA_TTABLE:
            [[fallthrough]];
         case LUA_TUSERDATA:
            {
               uint8_t found = 0;
               _pull_from_table(color, L, index, top, found);
               //
               found >>= 1;
               if ((found & 0b1111) == 0) {
                  //
                  // No RGBA components were supplied. Check for __tostring.
                  //
                  int type = luaL_getmetafield(L, index, "__tostring");
                  lua_pop(L, 1);
                  if (type == LUA_TFUNCTION) {
                     luaL_tolstring(L, index, nullptr);
                     _pull_from_string(color, L, lua_gettop(L));
                     lua_pop(L, 1);
                  }
               } else if ((found & 0b111) != 0b111) {
                  //
                  // TODO: Can we warn that a color component is missing? Or should we error on that 
                  // instead? Not sure how we'd report warnings properly for functions that take a 
                  // color verbatim, versus MOPHs (particularly those that accept multiple types, 
                  // including colors, with fallbacks).
                  //
               }
            }
            break;
            //
         case LUA_TSTRING:
            _pull_from_string(color, L, index);
            break;
         case LUA_TNUMBER:
         case LUA_TFUNCTION:
         case LUA_TLIGHTUSERDATA:
         case LUA_TBOOLEAN:
            [[fallthrough]];
         case LUA_TNONE:
         case LUA_TNIL:
            luaL_error(L, "color (string, table, or nil) expected");
            break;
      }
      //
      return color;
   }

   extern [[nodiscard]] QColor protected_pull_color(lua_State* L, int index, std::string& error) {
      auto top = lua_gettop(L);
      //
      error.clear();
      index = lua_absindex(L, index);
      //
      QColor result;
      lua_pushvalue(L, index);
      lua_pushlightuserdata(L, &result);
      lua_pushcfunction(L, &_pcallable_pull);
      auto   status = lua_pcall(L, 1, 1, 0);
      if (status == LUA_OK) {
         lua_settop(L, top);
         return result;
      }
      error = lua_tostring(L, -1);
      lua_settop(L, top);
      return QColor();
   }
}