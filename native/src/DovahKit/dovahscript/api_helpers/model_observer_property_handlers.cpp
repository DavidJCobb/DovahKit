#include "model_observer_property_handlers.h"
#include "../../helpers/lua/error.h"
#include "../../helpers/lua/warning.h"
#include "../../helpers/qt/get_model_of.h"
#include "../../ui/generic/ObservableStandardItemModel.h"
#include "../core/subsystems/resources.h"
#include "../core/subsystems/userdata.h"
#include "../core/classes.h"
#include "../tasks/s2m/ui_read_lambda.h"
#include "../tasks/s2m/ui_write_lambda.h"
#include "../tasks/s2m/ui_write_lambda_ex.h"
#include "../push_native_object.h"
#include "../send_script_task.h"
#include "../task_reference.h"
#include "../wrapper.h"

#include "../wrappers/resource/dds.h"
#include "../wrappers/resource/raster.h"
#include "../wrappers/ui/various/font.h"

#include "model_observers.h"
#include "qt_alignment.h"
#include "qt_color.h"

namespace dovahscript::api_helpers::moph {
   namespace {
      int __pcall_moph_pull_helper(lua_State* L) {
         auto* moph   = (model_observer_property_handler*) lua_touserdata(L, lua_upvalueindex(1));
         auto* result = (QVariant*) lua_touserdata(L, lua_upvalueindex(2));
         int top = lua_gettop(L);
         assert(top >= 1);
         //
         *result = (moph->pull)(L, 1);
         return 0;
      }
      QVariant _pcall_moph_pull(lua_State* L, int stack_pos, const model_observer_property_handler& moph) {
         QVariant result;
         //
         stack_pos = lua_absindex(L, stack_pos);
         lua_pushlightuserdata(L, (void*)&moph);   // upvalue 1
         lua_pushlightuserdata(L, (void*)&result); // upvalue 2
         lua_pushcclosure(L, &__pcall_moph_pull_helper, 2);
         lua_pushvalue(L, stack_pos);
         if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_warning(L, "invalid value for property `", 1);
            lua_warning(L, moph.name, 1);
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

   namespace util {
      extern int getter(lua_State* L) {
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
         auto* wrap = (wrapper*) classes::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            cobb::lua::error(L, "function called with bad self (expected %s)", class_metatable_key);
         auto& self = *wrap;
         if (!self.model_observer)
            return 0;
         auto* moph = class_handler_set->lookup(property_name);
         if (!moph)
            cobb::lua::error(L, "property `%1` is not available here", property_name);
         QVariant result = api_helpers::get_model_items_data(self.model_observer, moph->role);
         return (moph->push)(L, result, self);
      }
      extern int setter(lua_State* L) {
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
         auto* wrap = (wrapper*) classes::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            cobb::lua::error(L, "function called with bad self (expected %s)", class_metatable_key);
         auto& self = *wrap;
         if (!self.model_observer)
            return 0;
         const auto* moph = class_handler_set->lookup(property_name);
         if (!moph)
            cobb::lua::error(L, "property `%1` is not available here", property_name);
         QVariant value = (moph->pull)(L, 2);
         if (!value.isValid()) {
            if (!moph->clear_if_invalid)
               cobb::lua::error(L, "the value is invalid"); // TODO: can we report specific errors?
         }
         //
         auto* task = new tasks::s2m::ui_write_lambda_ex(false, [observer = task_reference(self.model_observer), value, moph]() mutable {
            int role = moph->role;
            //
            bool has_row = observer->row >= 0;
            bool has_col = observer->col >= 0;
            if (!has_row && !has_col)
               return;
            if (has_row && has_col) {
               if (auto* item = observer->item()) {
                  if (moph->transform) {
                     auto prior = item->data(role);
                     value = (moph->transform)(prior, value);
                  }
                  item->setData(value, role);
               }
               return;
            }
            auto* model = observer->model;
            if (!model)
               return;
            Qt::Orientation orientation;
            int pos;
            if (has_row) {
               pos = observer->row;
               orientation = ObservableStandardItemModelObserver::rowOrientation;
            } else {
               pos = observer->col;
               orientation = ObservableStandardItemModelObserver::colOrientation;
            }
            if (moph->transform) {
               auto prior = model->getDefaultDataForSpan(role, orientation, pos);
               value = (moph->transform)(prior, value);
            }
            model->setDefaultDataForSpan(role, orientation, pos, value);
         });
         send_script_ui_task(*task);
         return 0;
      }
   }

   void handler_set::extend(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const {
      getter_list_stack_pos = lua_absindex(L, getter_list_stack_pos);
      setter_list_stack_pos = lua_absindex(L, setter_list_stack_pos);
      lua_checkstack(L, 5);
      for (auto& moph : *this) {
         {  // Getter
            lua_pushstring       (L, class_metatable_key);
            lua_pushlightuserdata(L, (void*)this);
            lua_pushstring       (L, moph.name);
            lua_pushcclosure(L, &util::getter, 3);
         }
         lua_setfield(L, getter_list_stack_pos, moph.name);
         {
            lua_pushstring       (L, class_metatable_key);
            lua_pushlightuserdata(L, (void*)this);
            lua_pushstring       (L, moph.name);
            lua_pushcclosure(L, &util::setter, 3);
         }
         lua_setfield(L, setter_list_stack_pos, moph.name);
      }
   }
   QMap<Qt::ItemDataRole, QVariant> handler_set::extract(lua_State* L, int table_pos) const {
      table_pos = lua_absindex(L, table_pos);
      int top = lua_gettop(L);
      //
      QMap<Qt::ItemDataRole, QVariant> out;
      for(auto& moph : *this) {
         auto t = lua_getfield(L, table_pos, moph.name);
         if (t == LUA_TNONE || t == LUA_TNIL) {
            lua_settop(L, top);
            continue;
         }
         auto v = _pcall_moph_pull(L, top + 1, moph);
         if (v.isValid())
            out[moph.role] = v;
         lua_settop(L, top);
      }
      return out;
   }

   extern int push_alignment(lua_State* L, const QVariant& v, const wrapper& observer) {
      std::string out;
      api_helpers::alignment_to_string((Qt::Alignment)v.value<Qt::Alignment::Int>(), out);
      lua_pushstring(L, out.c_str());
      return 1;
   }
   extern QVariant pull_alignment(lua_State* L, int stack_pos) {
      if (!lua_isstring(L, stack_pos)) {
         cobb::lua::error(L, "expected a string value for the text alignment");
      }
      std::string h;
      std::string v;
      bool h_valid = false;
      bool v_valid = false;
      auto align   = api_helpers::alignment_from_string(lua_tostring(L, stack_pos), h, v, h_valid, v_valid);
      if (!h_valid && _stricmp(h.c_str(), "unchanged") != 0) {
         cobb::lua::warning(L, "%s is not a recognized horizontal text alignment keyowrd", h.c_str());
      }
      if (!v_valid && _stricmp(v.c_str(), "unchanged") != 0) {
         cobb::lua::warning(L, "%s is not a recognized vertical text alignment keyowrd", v.c_str());
      }
      return QVariant::fromValue<Qt::Alignment::Int>(align);
   }
   extern QVariant transform_alignment(const QVariant& existing, const QVariant& changes) {
      if (!existing.isValid() && !changes.isValid())
         return QVariant();
      Qt::Alignment prior = (Qt::Alignment) existing.value<Qt::Alignment::Int>();
      Qt::Alignment after = (Qt::Alignment) changes.value<Qt::Alignment::Int>();
      Qt::Alignment out = after;
      auto ph = prior & Qt::AlignHorizontal_Mask;
      auto pv = prior & Qt::AlignVertical_Mask;
      auto ah = after & Qt::AlignHorizontal_Mask;
      auto av = after & Qt::AlignVertical_Mask;
      if (!ah)
         out |= ph;
      if (!av) {
         out |= pv;
         if (!pv && ah) // use a default, but not if the string was "unchanged unchanged"
            out |= Qt::AlignVCenter; // this default is suitable for QTableView; don't know if it'll work for other model/view widgets
      }
      if (!out) {
         return QVariant(); // handle "unchanged unchanged"
      }
      return QVariant::fromValue<Qt::Alignment::Int>(out);
   }

   extern int push_color(lua_State* L, const QVariant& v, const wrapper& observer) {
      QColor color;
      switch (v.type()) {
         case QMetaType::QBrush:
            color = v.value<QBrush>().color();
            break;
         case QMetaType::QColor:
            color = v.value<QColor>();
            break;
         default:
            lua_pushnil(L);
            return 1;
      }
      api_helpers::push_color(L, color);
      return 1;
   }
   extern QVariant pull_color(lua_State* L, int stack_pos) {
      return api_helpers::pull_color(L, stack_pos);
   }

   extern int push_font(lua_State* L, const QVariant& v, const wrapper& observer) {
      if (v.isValid()) {
         assert(v.type() == QMetaType::QFont);
         wrapper out = observer;
         out.append_part(wrapper_part_types::ui_font_role);
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::font::metatable_key);
      }
      return 0;
   }
   extern QVariant pull_font(lua_State* L, int stack_pos) {
      if (lua_isnoneornil(L, stack_pos))
         return QVariant();
      QFont converted = wrappers::ui::font::pull(L, stack_pos);
      return QVariant::fromValue(converted);
   }

   extern int push_icon(lua_State* L, const QVariant& v, const wrapper& observer) {
      switch (v.type()) {
         case QMetaType::QBrush:
            api_helpers::push_color(L, v.value<QBrush>().color());
            return 1;
         case QMetaType::QColor:
            api_helpers::push_color(L, v.value<QColor>());
            return 1;
      }
      if (v.type() == qMetaTypeId<DovahscriptResourceHandle>() || v.type() == qMetaTypeId<DovahscriptResourceUIHandle>()) {
         auto* resource = DovahscriptResourceHandle::extract_from_variant(v);
         return push_native_object(resource);
      }
      lua_pushnil(L);
      return 1;
   }
   extern QVariant pull_icon(lua_State* L, int stack_pos) {
      if (lua_isnoneornil(L, stack_pos))
         return QVariant();
      if (auto* wrap = wrapper_from_stack<wrappers::resource::dds>(L, stack_pos)) {
         if (!wrap->managed_resource)
            return QVariant();
         return QVariant::fromValue<DovahscriptResourceUIHandle>(wrap->managed_resource);
      }
      if (auto* wrap = wrapper_from_stack<wrappers::resource::raster>(L, stack_pos)) {
         if (!wrap->managed_resource)
            return QVariant();
         return QVariant::fromValue<DovahscriptResourceUIHandle>(wrap->managed_resource);
      }
      return api_helpers::pull_color(L, stack_pos);
   }

   extern int push_string(lua_State* L, const QVariant& v, const wrapper& observer) {
      if (!v.isValid()) {
         lua_pushnil(L);
         return 1;
      }
      QString s = v.toString();
      lua_pushstring(L, s.toUtf8());
      return 1;
   }
   extern QVariant pull_string(lua_State* L, int stack_pos) {
      const char* s = luaL_tolstring(L, stack_pos, nullptr);
      return QVariant::fromValue(QString::fromUtf8(s));
   }
}