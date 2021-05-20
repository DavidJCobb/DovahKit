#include "model_observer_data.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/userdata.h"
#include "../../../wrapper.h"
#include "../../../cross_thread_tasks/s2m/lambda.h"
#include "../../../ui/util/alignment.h"
#include "../../../ui/util/color.h"
#include "../../../../../helpers/qt/get_model_of.h"

#include "../../resource/raster.h"

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

   extern void remove_items_from_model(QWidget* widget, int row, int col, QModelIndex parent) {
      if (row < 0) {
         if (row != -2)
            return;
      }
      if (col < 0) {
         if (col != -2)
            return;
      }
      auto* task    = new tasks::s2m::lambda(true);
      task->handler = [widget, row, col, parent]() {
         auto* base = cobb::qt::get_underlying_model_of(widget);
         if (!base)
            return;
         auto* model = qobject_cast<ObservableStandardItemModel*>(base);
         if (!model)
            return;
         //
         QStandardItem* parent_item;
         if (parent.isValid())
            parent_item = model->itemFromIndex(parent);
         else
            parent_item = model->invisibleRootItem();
         //
         if (col == -2) {
            // Remove an entire row.
            int target = row;
            int count  = 1;
            if (row == -2) {
               // Clear all content.
               target = 0;
               count = model->rowCount();
            }
            parent_item->removeRows(target, count);
            return;
         }
         if (row == -2) {
            // Remove an entire column
            parent_item->removeColumns(col, 1);
            return;
         }
         parent_item->setChild(row, col, nullptr);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
      delete task;
      //
      DovahKitScriptVMCore::get().zombify_all_invalid_model_observers();
   }
}

namespace editor_script::moph {
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
         const auto* moph = class_handler_set->lookup(property_name);
         if (!moph) {
            return luaL_error(L, "property `%1` is not available here", property_name);
         }
         QVariant value = (moph->pull)(L, 2);
         if (!value.isValid()) {
            if (!moph->clear_if_invalid) {
               return luaL_error(L, "the value is invalid"); // TODO: can we report specific errors?
            }
         }
         //
         auto* task     = new tasks::s2m::lambda(false);
         auto* observer = self.model_observer;
         task->handler  = [observer, value, moph]() mutable {
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
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         //
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

   extern int push_alignment(lua_State* L, const QVariant& v) {
      std::string out;
      editor_script::util::ui::alignment_to_string((Qt::Alignment)v.value<Qt::Alignment::Int>(), out);
      lua_pushstring(L, out.c_str());
      return 1;
   }
   extern QVariant pull_alignment(lua_State* L, int stack_pos) {
      if (!lua_isstring(L, stack_pos)) {
         luaL_error(L, "expected a string value for the text alignment");
      }
      std::string h;
      std::string v;
      bool h_valid = false;
      bool v_valid = false;
      auto align   = editor_script::util::ui::alignment_from_string(lua_tostring(L, stack_pos), h, v, h_valid, v_valid);
      if (!h_valid && _stricmp(h.c_str(), "unchanged") != 0) {
         lua_warning(L, h.c_str(), 1);
         lua_warning(L, " is not a recognized horizontal text alignment keyword", 0);
      }
      if (!v_valid && _stricmp(v.c_str(), "unchanged") != 0) {
         lua_warning(L, v.c_str(), 1);
         lua_warning(L, " is not a recognized vertical text alignment keyword", 0);
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

   extern int push_icon(lua_State* L, const QVariant& v) {
      switch (v.type()) {
         case QMetaType::QBrush:
            editor_script::util::ui::push_color(L, v.value<QBrush>().color());
            return 1;
         case QMetaType::QColor:
            editor_script::util::ui::push_color(L, v.value<QColor>());
            return 1;
      }
      if (v.type() == qMetaTypeId<LuaManagedResourceHandle>()) {
         auto* resource = LuaManagedResourceHandle::extract_from_variant(v);
         if (resource)
            return wrappers::resource::raster::wrap_and_push(L, *resource);
      }
      lua_pushnil(L);
      return 1;
   }
   extern QVariant pull_icon(lua_State* L, int stack_pos) {
      auto* wrap = wrapper_from_stack<wrappers::resource::raster>(L, stack_pos);
      if (wrap) {
         if (!wrap->managed_resource)
            return QVariant();
         auto handle = LuaManagedResourceHandle(wrap->managed_resource, nullptr);
         return QVariant::fromValue<LuaManagedResourceHandle>(handle);
      }
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
      const char* s = luaL_tolstring(L, stack_pos, nullptr);
      return QVariant::fromValue(QString::fromUtf8(s));
   }
}