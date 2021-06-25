#include "font.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include "../../../ui/util/lua_item_model.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "../../../../../helpers/lua/qt_variant.h"
#include "../../../../../helpers/qt/font.h"
#include "../../../../../helpers/function_traits.h"
#include "../../../../../helpers/strings.h"
#include "../../../../../helpers/type_traits.h"

#include "../../../widgets/objects/CanvasWidgetLayerDataLuaText.h"

#include <type_traits>

//
// QFont is a bit messy, not least because it has member functions that are neither deprecated nor 
// documented -- and that are highly relevant to what it does.
// 
// QFont consists of a set of optional font properties, as well as an internal flags mask denoting 
// which properties are actually set. This means that an individual QFont can be used to selectively 
// override specific font properties in some context, while leaving other font properties (i.e. ones 
// inherited from some broader context) untouched. In simpler terms: you can have a QFont that says, 
// "make this text bold, but don't change anything else" just by creating a new QFont and making sure 
// to only call QFont::setWeight on it.
// 
// When Qt needs to actually determine the final font properties for some object, it does so by using 
// {QFont QFont::resolve(const QFont&) const} to essentially "stack" all relevant QFonts together. 
// When that function is called on some QFont instance A and given some QFont instance B, it will 
// create and return a new QFont consisting of all properties that were set on A or B; if a property 
// is set on both A and B, then the value from A takes priority and is used by the result.
// 
// Given that QFont works this way, you would probably expect to be able to do the following:
// 
//  - Query which properties have been set on a given QFont
//  - Clear a property that was previously set on a given QFont
// 
// In reality, it's not nearly that simple. There are no documented functions which can be used for 
// these tasks. There are, however, undocumented public functions in files that are not marked as 
// being internal-only, which can perform these tasks. This is not entirely unusual; the "detach" 
// function on classes like QImage is also undocumented in most cases. For QFont, the functions we 
// want are these:
// 
//  - uint QFont::resolve() const;
//    Return the internal bitmask of "resolved" properties.
// 
//  - void QFont::resolve(uint);
//    Wholly overwrite the internal bitmask of "resolved" properties.
//

namespace {
   QFont _get_font(const editor_script::wrapper& w) {
      switch (w.type) {
         using wt = editor_script::wrapper_type;
         case wt::ui: // widget
            if (auto* u = w.widget) {
               assert(w.parts[0].signature == editor_script::wrapper_part_types::ui_font_data);
               return u->font();
            }
            break;
         case wt::ui_canvas_layer_data:
            if (auto* d = w.canvas_layer_data) {
               if (auto* tld = qobject_cast<CanvasWidgetLayerDataLuaText*>(d)) {
                  assert(w.parts[0].signature == editor_script::wrapper_part_types::ui_font_data);
                  return tld->font;
               }
            }
            break;
         case wt::ui_model_item:
            if (auto* o = w.model_observer) {
               if (auto* i = o->item()) {
                  assert(w.parts[0].signature == editor_script::wrapper_part_types::ui_font_role);
                  auto data = i->data(Qt::FontRole);
                  if (data.isValid() && data.type() == QMetaType::QFont)
                     return data.value<QFont>();
               }
            }
            break;
      }
      return QFont();
   }
   void _set_font(editor_script::wrapper& w, QFont f) {
      switch (w.type) {
         using wt = editor_script::wrapper_type;
         case wt::ui: // widget
            if (auto* u = w.widget) {
               u->setFont(f);
            }
            return;
         case wt::ui_canvas_layer_data:
            if (auto* d = w.canvas_layer_data) {
               if (auto* tld = qobject_cast<CanvasWidgetLayerDataLuaText*>(d)) {
                  tld->font = f;
               }
            }
            return;
         case wt::ui_model_item:
            if (auto* o = w.model_observer) {
               bool has_row = o->row >= 0;
               bool has_col = o->col >= 0;
               if (!has_row && !has_col)
                  return;
               if (has_row && has_col) {
                  if (auto* item = o->item()) {
                     if (f.resolve() == 0) {
                        item->setData(QVariant(), Qt::FontRole); // if it's an empty font, just clear it entirely
                     } else {
                        item->setData(f, Qt::FontRole);
                     }
                  }
                  return;
               }
               auto* model = o->model;
               if (!model)
                  return;
               Qt::Orientation orientation;
               int pos;
               if (has_row) {
                  pos = o->row;
                  orientation = ObservableStandardItemModelObserver::rowOrientation;
               } else {
                  pos = o->col;
                  orientation = ObservableStandardItemModelObserver::colOrientation;
               }
               if (f.resolve() == 0) {
                  model->setDefaultDataForSpan(Qt::FontRole, orientation, pos, QVariant()); // if it's an empty font, just clear it entirely
               } else {
                  model->setDefaultDataForSpan(Qt::FontRole, orientation, pos, f);
               }
            }
            return;
      }
   }
}

namespace editor_script::impl::font_properties {
   namespace {
      int __pcall_pull_helper(lua_State* L) {
         auto* hnd    = (handler*) lua_touserdata(L, lua_upvalueindex(1));
         auto* result = (QVariant*) lua_touserdata(L, lua_upvalueindex(2));
         int top = lua_gettop(L);
         assert(top >= 1);
         //
         *result = (hnd->pull)(L, 1);
         return 0;
      }
      QVariant _pcall_pull(lua_State* L, int stack_pos, const handler& hnd) {
         QVariant result;
         //
         stack_pos = lua_absindex(L, stack_pos);
         lua_pushlightuserdata(L, (void*)&hnd);   // upvalue 1
         lua_pushlightuserdata(L, (void*)&result); // upvalue 2
         lua_pushcclosure(L, &__pcall_pull_helper, 2);
         lua_pushvalue(L, stack_pos);
         if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_warning(L, "invalid value for property `", 1);
            lua_warning(L, hnd.name, 1);
            if (lua_isstring(L, -1)) {
               lua_warning(L, "`: ", 1);
               lua_warning(L, lua_tostring(L, -1), 0);
            } else {
               lua_warning(L, "`", 0);
            }
            result = QVariant();
         }
         //
         return result;
      }
   }
   namespace {
      extern int _getter(lua_State* L) {
         // Upvalue 1: string:         class metatable key (used to type-check self and get a valid wrapper-object)
         // Upvalue 2: light userdata: the handler set
         // Upvalue 3: light userdata: the handler name
         assert(lua_isstring(L, lua_upvalueindex(1)));
         assert(lua_islightuserdata(L, lua_upvalueindex(2)));
         assert(lua_isstring(L, lua_upvalueindex(3)));
         auto* class_metatable_key = lua_tostring(L, lua_upvalueindex(1));
         auto* class_handler_set   = (handler_set*) lua_touserdata(L, lua_upvalueindex(2));
         auto* property_name       = lua_tostring(L, lua_upvalueindex(3));
         assert(class_handler_set);
         assert(property_name && property_name[0]);
         //
         auto* wrap = (wrapper*) editor_script::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            return luaL_error(L, "function called with bad self (expected %s)", class_metatable_key);
         auto* hnd = class_handler_set->lookup(property_name);
         if (!hnd)
            return luaL_error(L, "property `%1` is not available here", property_name);
         //
         QVariant value;
         bool not_set = false;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [hnd, &wrap, &value, &not_set]() {
               auto font = _get_font(*wrap);
               if (hnd->resolve_mask) {
                  if (!cobb::qt::test_font_properties(font, hnd->resolve_mask)) {
                     not_set = true;
                     return;
                  }
               }
               value = (hnd->get)(font);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (not_set) {
            lua_pushnil(L);
            return 1;
         }
         return (hnd->push)(L, value);
      }
      extern int _setter(lua_State* L) {
         // Upvalue 1: string:         class metatable key (used to type-check self and get a valid wrapper-object)
         // Upvalue 2: light userdata: the handler set
         // Upvalue 3: light userdata: the handler name
         assert(lua_isstring(L, lua_upvalueindex(1)));
         assert(lua_islightuserdata(L, lua_upvalueindex(2)));
         assert(lua_isstring(L, lua_upvalueindex(3)));
         auto* class_metatable_key = lua_tostring (L, lua_upvalueindex(1));
         auto* class_handler_set   = (handler_set*) lua_touserdata(L, lua_upvalueindex(2));
         auto* property_name       = lua_tostring (L, lua_upvalueindex(3));
         assert(class_handler_set);
         assert(property_name && property_name[0]);
         //
         auto* wrap = (wrapper*)editor_script::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            return luaL_error(L, "function called with bad self (expected %s)", class_metatable_key);
         const auto* hnd = class_handler_set->lookup(property_name);
         if (!hnd)
            return luaL_error(L, "property `%1` is not available here", property_name);
         //
         if (hnd->resolve_mask && lua_isnoneornil(L, 2)) {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [hnd, &wrap, class_handler_set]() {
               QFont font = _get_font(*wrap);
               cobb::qt::clear_font_properties(font, hnd->resolve_mask);
               _set_font(*wrap, font);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            //
            return 0;
         }
         auto value = (hnd->pull)(L, 2);
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [hnd, &wrap, &value]() {
               auto font = _get_font(*wrap);
               (hnd->set)(font, value);
               _set_font(*wrap, font);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         //
         return 0;
      }
   }

   void handler_set::extend_lua_class(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept {
      getter_list_stack_pos = lua_absindex(L, getter_list_stack_pos);
      setter_list_stack_pos = lua_absindex(L, setter_list_stack_pos);
      lua_checkstack(L, 5);
      for (auto& hnd : *this) {
         {  // Getter
            lua_pushstring       (L, class_metatable_key);
            lua_pushlightuserdata(L, (void*)this);
            lua_pushstring       (L, hnd.name);
            lua_pushcclosure(L, &_getter, 3);
         }
         lua_setfield(L, getter_list_stack_pos, hnd.name);
         {
            lua_pushstring       (L, class_metatable_key);
            lua_pushlightuserdata(L, (void*)this);
            lua_pushstring       (L, hnd.name);
            lua_pushcclosure(L, &_setter, 3);
         }
         lua_setfield(L, setter_list_stack_pos, hnd.name);
      }
   }
   QFont handler_set::table_to_struct(lua_State* L, int table_pos) const noexcept {
      table_pos = lua_absindex(L, table_pos);
      int top = lua_gettop(L);
      //
      QFont out;
      for(auto& hnd : *this) {
         auto t = lua_getfield(L, table_pos, hnd.name);
         if (t == LUA_TNONE || t == LUA_TNIL) {
            lua_settop(L, top);
            continue;
         }
         auto v = _pcall_pull(L, top + 1, hnd);
         if (v.isValid())
            (hnd.set)(out, v);
         lua_settop(L, top);
      }
      return out;
   }
}

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::font;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
   }
   namespace _getters {
   }
   namespace _setters {
   }

   namespace _singleton_functions {
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace {
   template<typename T> int simple_push(lua_State* L, const QVariant& v) {
      if constexpr (std::is_same_v<T, bool>) {
         lua_pushboolean(L, v.toBool());
         return 1;
      }
      if constexpr (std::is_same_v<T, int>) {
         lua_pushinteger(L, v.toInt());
      } else if constexpr (std::is_same_v<T, const char*>) {
         lua_pushstring(L, v.toString().toUtf8());
      } else if constexpr (std::is_same_v<T, float>) {
         lua_pushnumber(L, v.toFloat());
      } else if constexpr (std::is_same_v<T, double>) {
         lua_pushnumber(L, v.toDouble());
      } else if constexpr (std::is_same_v<T, bool>) {
         lua_pushboolean(L, v.toBool());
      } else {
         // Lambda ugliness allows us to make static asserts conditional on their containing if-constexprs.
         []<bool flag = false>(){ static_assert(flag, "Unhandled Lua type!"); };
      }
      return 1;
   }
   template<typename T> QVariant simple_pull(lua_State* L, int stack_pos) {
      if constexpr (std::is_same_v<T, int>) {
         int isnum;
         int value = lua_tointegerx(L, stack_pos, &isnum);
         if (!isnum) {
            luaL_error(L, "integer or nil expected");
         }
         return QVariant::fromValue<int>(value);
      } else if constexpr (std::is_same_v<T, const char*>) {
         if (!lua_isstring(L, stack_pos)) {
            luaL_error(L, "string or nil expected");
         }
         auto* value = lua_tostring(L, stack_pos);
         return QVariant::fromValue<QString>(QString::fromUtf8(value));
      } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
         if (!lua_isnumber(L, stack_pos)) {
            luaL_error(L, "number or nil expected");
         }
         if constexpr (std::is_same_v<T, float>)
            return QVariant::fromValue<float>(lua_tonumber(L, stack_pos));
         QVariant::fromValue<double>(lua_tonumber(L, stack_pos));
      } else if constexpr (std::is_same_v<T, bool>) {
         if (!lua_isboolean(L, stack_pos)) {
            luaL_error(L, "boolean or nil expected");
         }
         return QVariant::fromValue<bool>(lua_toboolean(L, stack_pos));
      } else {
         // Lambda ugliness allows us to make static asserts conditional on their containing if-constexprs.
         []<bool flag = false>(){ static_assert(flag, "Unhandled Lua type!"); };
      }
      return QVariant();
   }
   
   template<auto func> int verbatim_push(lua_State* L, const QVariant& v) {
      static_assert(std::is_same_v<cobb::member_function_context_type<func>, QFont>, "The wrapped function must be a getter on QFont.");
      using T = cobb::return_type_of<func>;
      //
      return simple_push<T>(L, v);
   }
   template<auto func> QVariant verbatim_pull(lua_State* L, int stack_pos) {
      static_assert(std::is_same_v<cobb::member_function_context_type<func>, QFont>, "The wrapped function must be a setter on QFont.");
      using T = cobb::type_of_nth_argument<func, 0>;
      //
      return simple_pull<T>(L, stack_pos);
   }
   
   template<auto func, typename T> QVariant qvariant_get(const T& obj) {
      using raw_t = cobb::return_type_of<func>;
      using int_t = cobb::strip_enum_t<raw_t>;
      static_assert(std::is_base_of_v<cobb::member_function_context_type<func>, T>, "The wrapped function must be a getter on templated type T.");
      static_assert(cobb::is_const_function<func>, "The wrapped function must be a getter and therefore must be const.");
      static_assert(!std::is_same_v<raw_t, void>, "The wrapped function must be a getter and therefore must return a value.");
      //
      raw_t value = (obj.*func)();
      return QVariant::fromValue<int_t>((int_t)value);
   }
   template<auto func, typename T> void qvariant_set(T& obj, const QVariant& value) {
      using raw_t = cobb::type_of_nth_argument<func, 0>;
      using int_t = cobb::strip_enum_t<raw_t>;
      static_assert(std::is_base_of_v<cobb::member_function_context_type<func>, T>, "The wrapped function must be a setter on templated type T.");
      static_assert(!cobb::is_const_function<func>, "The wrapped function must be a setter and therefore cannot be const.");
      //
      (obj.*func)((raw_t)value.value<int_t>());
   }

   int push_indexed_integer(lua_State* L, const QVariant& v) {
      int i = v.toInt() + 1;
      lua_pushinteger(L, i);
      return 1;
   }
   QVariant pull_indexed_integer(lua_State* L, int stack_pos) {
      int isnum;
      int value = lua_tointegerx(L, stack_pos, &isnum);
      if (!isnum)
         luaL_error(L, "integer expected");
      --value;
      return QVariant::fromValue<int>(value);
   }
}

namespace {
   namespace _fields {
      namespace bold {
         QVariant get(const QFont& font) {
            return QVariant::fromValue<bool>(font.weight() >= QFont::Medium);
         }
         void set(QFont& font, const QVariant& value) {
            bool after = value.toBool();
            bool prior = font.weight() >= QFont::Medium;
            if (after == prior) {
               if (font.resolve() & QFont::ResolveProperties::WeightResolved)
                  return;
            }
            if (after)
               font.setWeight(QFont::Bold);
            else
               font.setWeight(QFont::Normal);
         }
         int push(lua_State* L, const QVariant& value) {
            lua_pushboolean(L, value.toBool());
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isboolean(L, stack_pos))
               luaL_error(L, "boolean or nil expected");
            return QVariant::fromValue<bool>(lua_toboolean(L, stack_pos));
         }
      }
      namespace capitalization {
         using C = QFont::Capitalization;
         static constexpr const std::array list = {
            std::pair{ C::MixedCase,    "normal" },
            std::pair{ C::AllUppercase, "uppercase" },
            std::pair{ C::AllLowercase, "lowercase" },
            std::pair{ C::SmallCaps,    "small caps" },
            std::pair{ C::Capitalize,   "capitalize" }
         };
         //
         int push(lua_State* L, const QVariant& value) {
            C v = (C)value.value<int>();
            for (auto& pair : list) {
               if (pair.first == v) {
                  lua_pushstring(L, pair.second);
                  return 1;
               }
            }
            return 0;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isstring(L, stack_pos))
               luaL_error(L, "string or nil expected");
            auto* v = lua_tostring(L, stack_pos);
            for (auto& pair : list) {
               if (_stricmp(v, pair.second) == 0) {
                  return QVariant::fromValue<int>(pair.first);
               }
            }
            std::string error = "unrecognized string; valid values are: ";
            size_t size = list.size();
            for (size_t i = 0; i < size; ++i) {
               if (i)
                  error += ", ";
               error += '"';
               error += list[i].second;
               error += '"';
            }
            luaL_error(L, error.c_str());
            __assume(0);
         }
      }
      namespace family {
         std::array generics_to_constants = {
            std::pair{ QFont::SansSerif, "sans-serif" },
            std::pair{ QFont::Serif,     "serif" },
            std::pair{ QFont::Monospace, "monospace" },
            std::pair{ QFont::Fantasy,   "fantasy" },
            std::pair{ QFont::Cursive,   "cursive" },
         };
         //
         QFont::StyleHint _check_generic(const QString& family) {
            for (auto& pair : generics_to_constants)
               if (family.compare(QByteArray(pair.second), Qt::CaseInsensitive) == 0)
                  return pair.first;
            return QFont::StyleHint::AnyStyle;
         }
         bool _handle_quotes(QString& family, bool& quoted) { // returns false on error
            if (family[0] != '"' && family[0] != '\'') // not quoted
               return true;
            auto quot = family[0];
            auto size = family.size();
            if (family[size - 1] != quot) // not quoted
               return true;
            bool escape = false;
            for (int i = 1; i < size - 1; ++i) {
               if (escape) {
                  escape = false;
                  continue;
               }
               if (family[i] == '\\') {
                  escape = true;
                  continue;
               }
               if (family[i] == quot) // unescaped quote in middle
                  return false;
            }
            family = family.mid(1, size - 2);
            return true;
         }
         void _strip_quotes(QString& family) { // assumes the family has already been checked for correctness, and skips those checks
            if (family[0] != '"' && family[0] != '\'') // not quoted
               return;
            auto quot = family[0];
            auto size = family.size();
            if (family[size - 1] != quot) // not quoted
               return;
            family = family.mid(1, size - 2);
         }
         bool _is_generic(const QString& family) {
            return _check_generic(family) != QFont::StyleHint::AnyStyle;
         }
         
         QVariant get(const QFont& font) {
            QString out;
            //
            auto mask = font.resolve();
            if (mask & QFont::ResolveProperties::FamilyResolved) {
               auto family = font.family();
               if (_is_generic(family)) {
                  out += '"';
                  out += family.replace('"', "\"");
                  out += '"';
               } else {
                  out += family;
               }
            }
            if (mask & QFont::ResolveProperties::FamiliesResolved) {
               for (auto& family : font.families()) {
                  if (!out.isEmpty())
                     out += ", ";
                  if (_is_generic(family)) {
                     out += '"';
                     out += family.replace('"', "\"");
                     out += '"';
                  } else {
                     out += family;
                  }
               }
            }
            if (mask & QFont::ResolveProperties::StyleHintResolved) {
               auto keyword = font.styleHint();
               for (auto& pair : generics_to_constants) {
                  if (pair.first == keyword) {
                     if (!out.isEmpty())
                        out += ", ";
                     out += pair.second;
                  }
               }
            }
            //
            return QVariant::fromValue<QString>(out);
         }
         void set(QFont& font, const QVariant& value) {
            auto list = value.value<QStringList>();
            auto size = list.size();
            //
            QStringList families;
            auto hint = QFont::StyleHint::AnyStyle;
            //
            for (int i = 0; i < size; ++i) {
               auto family = list[i];
               if (i == size - 1) {
                  hint = _check_generic(family);
                  if (hint != QFont::StyleHint::AnyStyle) {
                     continue;
                  }
               }
               _strip_quotes(family);
               families.push_back(family);
            }
            //
            font.setStyleHint(hint);
            font.setFamilies(families);
         }
         int push(lua_State* L, const QVariant& value) {
            if (!value.isValid())
               return 0;
            lua_pushstring(L, value.toString().toUtf8());
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isstring(L, stack_pos))
               luaL_error(L, "string or nil expected");
            //
            QStringList list;
            //
            QString source   = QString::fromUtf8(lua_tostring(L, stack_pos));
            QChar   in_quote = '\0';
            QString current;
            for (auto c : source) {
               if (c == ',') {
                  list.push_back(current.trimmed());
                  current.clear();
                  continue;
               }
               if (current.isEmpty() && c.isSpace())
                  continue;
               current += c;
            }
            if (!current.isEmpty()) {
               list.push_back(current);
               current.clear();
            }
            //
            int size = list.size();
            for(int i = 0; i < size; ++i) {
               auto family = list[i];
               bool quoted = false;
               if (!_handle_quotes(family, quoted)) {
                  luaL_error(L, "incorrect use of enclosing quotes");
               }
               if (!quoted) {
                  if (_is_generic(family) && i != size - 1) {
                     luaL_error(L, "a font family list can only contain one generic family, and it must be at the end of the list; to use a font actually named \"%s\", enclose it in quotes", family);
                  }
               }
            }
            //
            return QVariant::fromValue<QStringList>(list);
         }
      }
      namespace italics {
         QVariant get(const QFont& font) {
            return QVariant::fromValue<bool>(font.style() != QFont::Style::StyleNormal);
         }
         void set(QFont& font, const QVariant& value) {
            font.setStyle(value.toBool() ? QFont::Style::StyleItalic : QFont::Style::StyleNormal);
         }
         int push(lua_State* L, const QVariant& value) {
            lua_pushboolean(L, value.toBool());
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isboolean(L, stack_pos))
               luaL_error(L, "boolean or nil expected");
            return QVariant::fromValue<bool>(lua_toboolean(L, stack_pos));
         }
      }
      namespace letter_spacing {
         QVariant get(const QFont& font) {
            if (font.letterSpacingType() != QFont::SpacingType::AbsoluteSpacing)
               return QVariant::fromValue<qreal>(0.0);
            return QVariant::fromValue<qreal>(font.letterSpacing());
         }
         void set(QFont& font, const QVariant& value) {
            qreal n = value.value<qreal>();
            font.setLetterSpacing(QFont::SpacingType::AbsoluteSpacing, n);
         }
         int push(lua_State* L, const QVariant& value) {
            lua_pushnumber(L, value.value<qreal>());
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isnumber(L, stack_pos))
               luaL_error(L, "number or nil expected");
            return QVariant::fromValue<qreal>(lua_tonumber(L, stack_pos));
         }
      }
      namespace size {
         QVariant get(const QFont& font) {
            auto value = QString("%1%2");
            auto size  = font.pixelSize();
            if (size < 0) {
               size  = font.pointSize();
               value = value.arg(size).arg("pt");
            } else {
               value = value.arg(size).arg("px");
            }
            return value;
         }
         void set(QFont& font, const QVariant& value) {
            auto string = value.value<QString>();
            auto size   = string.size();
            //
            assert(!string.isEmpty());
            assert(size > 2);
            assert(string[size - 2] == 'p');
            //
            QChar type = string[size - 1]; // cannot use "auto" because that gets us a QCharRef
            string.chop(2);
            bool ok;
            auto num = string.toInt(&ok); // toInt ignores whitespace, so that ensures that "12 pt" and so on still works
            assert(ok);
            assert(num >= 0);
            //
            if (type == 't') { // point
               font.setPointSize(num);
            } else if (type == 'x') { // pixel
               font.setPixelSize(num);
            }
         }
         int push(lua_State* L, const QVariant& value) {
            lua_pushstring(L, value.value<QString>().toUtf8());
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isstring(L, stack_pos))
               luaL_error(L, "string or nil expected");
            auto string = QString::fromUtf8(lua_tostring(L, stack_pos)).trimmed(); // we remove leading and trailing whitespace here; whitespace between the number and unit is removed in (set)
            auto size   = string.size();
            if (size < 2)
               luaL_error(L, "invalid font size: the string must consist of a number followed by a unit (either 'px' or 'pt'), with no space");
            if (!string.endsWith("pt")) {
               if (!string.endsWith("px")) {
                  luaL_error(L, "invalid font size: you must specify a unit (either 'px' or 'pt') after the number, with no space");
               }
            }
            return string;
         }
      }
      namespace weight {
         QVariant pull(lua_State* L, int stack_pos) {
            int isnum;
            int value = lua_tointegerx(L, stack_pos, &isnum);
            if (!isnum)
               luaL_error(L, "integer or nil expected");
            if (value <= 0 || value > 100)
               luaL_error(L, "weight must be between 1 and 100 inclusive");
            --value;
            return QVariant::fromValue<int>(value);
         }
      }
      namespace width {
         int push(lua_State* L, const QVariant& value) {
            int stretch = value.toInt();
            if (stretch == QFont::AnyStretch) // this is basically the same as CSS "inherit"
               return 0;
            lua_pushinteger(L, stretch);
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            int isnum;
            int value = lua_tointegerx(L, stack_pos, &isnum);
            if (!isnum)
               luaL_error(L, "integer or nil expected");
            if (value <= 0)
               luaL_error(L, "font stretch must be greater than zero");
            if (value > 4000)
               luaL_error(L, "font stretch cannot be greater than 4000");
            return QVariant::fromValue<int>(value);
         }
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      //
      // Fields that are handled as "font property handlers" should go in the list of those below, not here.
      //
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      //
      // Fields that are handled as "font property handlers" should go in the list of those below, not here.
      //
   };

   using fph = editor_script::impl::font_properties::handler;
   /*static*/ const editor_script::impl::font_properties::handler_set cls::fph_handlers = {{
      fph{ 
         .name = "bold", // Boolean shortcut to make a font bold or not bold.
         .push = _fields::bold::push,
         .pull = _fields::bold::pull,
         .get  = _fields::bold::get,
         .set  = _fields::bold::set,
         .resolve_mask = QFont::ResolveProperties::WeightResolved,
      },
      fph{ 
         .name = "capitalization", // String enum used to change letter case or enable small caps.
         .push = _fields::capitalization::push,
         .pull = _fields::capitalization::pull,
         .get  = qvariant_get<&QFont::capitalization>,
         .set  = qvariant_set<&QFont::setCapitalization>,
         .resolve_mask = QFont::ResolveProperties::CapitalizationResolved,
      },
      fph{ 
         .name = "family", // Positive integer indicating the font's width as a percentage.
         .push = _fields::family::push,
         .pull = _fields::family::pull,
         .get  = _fields::family::get,
         .set  = _fields::family::set,
         .resolve_mask = (QFont::ResolveProperties)(QFont::ResolveProperties::FamilyResolved | QFont::ResolveProperties::FamiliesResolved | QFont::ResolveProperties::StyleHintResolved),
      },
      fph{ 
         .name = "italics", // Boolean to make a font italic or not italic.
         .push = _fields::italics::push,
         .pull = _fields::italics::pull,
         .get  = _fields::italics::get,
         .set  = _fields::italics::set,
         .resolve_mask = QFont::ResolveProperties::StyleResolved,
      },
      fph{ 
         .name = "letter_spacing", // Letter spacing as a signed number.
         .push = _fields::letter_spacing::push,
         .pull = _fields::letter_spacing::pull,
         .get  = _fields::letter_spacing::get,
         .set  = _fields::letter_spacing::set,
         .resolve_mask = QFont::ResolveProperties::LetterSpacingResolved,
      },
      fph{ 
         .name = "size", // Font size as a string, e.g. "12px" or "12pt"; a unit is required.
         .push = _fields::size::push,
         .pull = _fields::size::pull,
         .get  = _fields::size::get,
         .set  = _fields::size::set,
         .resolve_mask = QFont::ResolveProperties::SizeResolved,
      },
      fph{ 
         .name = "strikethrough", // Boolean indicating whether a line crosses through the middle of the text.
         .push = verbatim_push<&QFont::strikeOut>,
         .pull = verbatim_pull<&QFont::setStrikeOut>,
         .get  = qvariant_get<&QFont::strikeOut>,
         .set  = qvariant_set<&QFont::setStrikeOut>,
         .resolve_mask = QFont::ResolveProperties::StrikeOutResolved,
      },
      fph{ 
         .name = "weight", // Specific font weight (boldness) as an int between 1 and 100.
         .push = push_indexed_integer,
         .pull = _fields::weight::pull,
         .get  = qvariant_get<&QFont::weight>,
         .set  = qvariant_set<&QFont::setWeight>,
         .resolve_mask = QFont::ResolveProperties::WeightResolved,
      },
      fph{ 
         .name = "width", // Positive integer indicating the font's width as a percentage.
         .push = _fields::width::push,
         .pull = _fields::width::pull,
         .get  = qvariant_get<&QFont::stretch>,
         .set  = qvariant_set<&QFont::setStretch>,
         .resolve_mask = QFont::ResolveProperties::StretchResolved,
      },
      fph{ 
         .name = "word_spacing", // Number indicating added spacing between words; can be negative; doesn't apply to space-less writing systems.
         .push = verbatim_push<&QFont::wordSpacing>,
         .pull = verbatim_pull<&QFont::setWordSpacing>,
         .get  = qvariant_get<&QFont::wordSpacing>,
         .set  = qvariant_set<&QFont::setWordSpacing>,
         .resolve_mask = QFont::ResolveProperties::WordSpacingResolved,
      },
   }};

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      int index_class   = lua_absindex(L, -3);
      int index_getters = lua_absindex(L, -2);
      int index_setters = lua_absindex(L, -1);
      //
      cls::fph_handlers.extend_lua_class(L, cls::metatable_key, index_getters, index_setters);
   }

   /*static*/ QFont cls::pull(lua_State* L, int stack_pos) {
      if (auto* wrap = wrapper_from_stack<cls>(L, stack_pos)) {
         QFont out;
         //
         auto* task = new tasks::s2m::ui_read_lambda();
         task->handler = [&out, &wrap]() {
            out = _get_font(*wrap);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         return out;
      }
      return cls::fph_handlers.table_to_struct(L, stack_pos);
   }
}