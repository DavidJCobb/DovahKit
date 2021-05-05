#include "model_observer_data.h"
#include "../../../systems/messaging.h"
#include "../../../wrapper.h"
#include "../../../cross_thread_tasks/s2m/lambda.h"
#include "../../../ui/util/alignment.h"
#include "../../../ui/util/color.h"

namespace editor_script::helpers {
   extern [[nodiscard]] QVariant get_model_items_data(ObservableStandardItemModelObserver* observer, int role) {
      QVariant result;
      //
      auto* task = new tasks::s2m::ui_read_lambda();
      task->handler = [observer, role, &result]() {
         bool has_row = observer->row >= 0;
         bool has_col = observer->col >= 0;
         if (!has_row && !has_col)
            return;
         if (has_row && has_col) {
            if (auto* item = observer->item())
               result = item->data(role);
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
         result = model->getDefaultDataForSpan(role, orientation, pos);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
      delete task;
      //
      return result;
   }
   extern void set_model_items_data(ObservableStandardItemModelObserver* observer, int role, QVariant data) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [observer, role, data]() {
         bool has_row = observer->row >= 0;
         bool has_col = observer->col >= 0;
         if (!has_row && !has_col)
            return;
         if (has_row && has_col) {
            if (auto* item = observer->item())
               item->setData(data, role);
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
         model->setDefaultDataForSpan(role, orientation, pos, data);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }
}

namespace editor_script::moph {
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
         auto* wrap = (wrapper*) editor_script::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            return luaL_error(L, "function called with bad self (expected %s)", class_metatable_key);
         auto& self = *wrap;
         if (!self.model_observer)
            return 0;
         auto* moph = class_handler_set->lookup(property_name);
         if (!moph) {
            return luaL_error(L, "property `%1` is not available here", property_name);
         }
         QVariant result = helpers::get_model_items_data(self.model_observer, moph->role);
         return moph->push(L, result);
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
         auto* wrap = (wrapper*)editor_script::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            return luaL_error(L, "function called with bad self (expected %s)", class_metatable_key);
         auto& self = *wrap;
         if (!self.model_observer)
            return 0;
         auto* moph = class_handler_set->lookup(property_name);
         if (!moph) {
            return luaL_error(L, "property `%1` is not available here", property_name);
         }
         QVariant value = moph->pull(L, 2);
         if (!value.isValid()) {
            if (!moph->clear_if_invalid) {
               return luaL_error(L, "the value is invalid"); // TODO: can we report specific errors?
            }
         }
         helpers::set_model_items_data(self.model_observer, moph->role, value);
         return 0;
      }
   }

   void handler_set::extend(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept {
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
   QMap<Qt::ItemDataRole, QVariant> handler_set::extract(lua_State* L, int table_pos) const noexcept {
      table_pos = lua_absindex(L, table_pos);
      int top = lua_gettop(L);
      //
      QMap<Qt::ItemDataRole, QVariant> out;
      for(auto& moph : *this) {
         lua_getfield(L, table_pos, moph.name);
         auto v = moph.pull(L, top + 1);
         if (v.isValid())
            out[moph.role] = v;
         lua_settop(L, top);
      }
      return out;
   }

   extern int push_alignment(lua_State* L, const QVariant& v) {
      std::string out;
      editor_script::util::ui::alignment_to_string(v.value<QFlags<Qt::AlignmentFlag>>(), out);
      lua_pushstring(L, out.c_str());
      return 1;
   }
   extern QVariant pull_alignment(lua_State* L, int stack_pos) {
      std::string h;
      std::string v;
      bool h_valid = false;
      bool v_valid = false;
      auto align   = editor_script::util::ui::alignment_from_string(lua_tostring(L, stack_pos), h, v, h_valid, v_valid);
      //
      // TODO: How do we support "unchanged" here?
      // 
      // TODO: How do we report warnings or errors here? In this case, unrecognized alignment values should 
      //       report an error. (Note that the above function does not recognize "unchanged" and this is by 
      //       design.)
      //
      return QVariant::fromValue<QFlags<Qt::AlignmentFlag>>(align);
   }

   extern int push_color(lua_State* L, const QVariant& v) {
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
      editor_script::util::ui::push_color(L, color);
      return 1;
   }
   extern QVariant pull_color(lua_State* L, int stack_pos) {
      return editor_script::util::ui::pull_color(L, stack_pos);
   }

   extern int push_string(lua_State* L, const QVariant& v) {
      if (!v.isValid()) {
         lua_pushnil(L);
         return 1;
      }
      QString s = v.toString();
      lua_pushstring(L, s.toUtf8());
      return 1;
   }
   extern QVariant pull_string(lua_State* L, int stack_pos) {
      if (!lua_isstring(L, stack_pos))
         return QVariant();
      return QVariant::fromValue(QString::fromUtf8(lua_tostring(L, stack_pos)));
   }
}