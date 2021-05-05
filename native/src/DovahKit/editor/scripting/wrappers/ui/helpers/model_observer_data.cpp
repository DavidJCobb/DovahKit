#include "model_observer_data.h"
#include "../../../systems/messaging.h"
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
      extern int fail_to_push(lua_State* L, const QVariant&) {
         luaL_error(L, "unrecognized property");
         return 0;
      }
      extern QVariant fail_to_pull(lua_State* L, int stack_pos) {
         luaL_error(L, "unrecognized property");
         return QVariant();
      }
   }

   extern QMap<Qt::ItemDataRole, QVariant> extract_role_dataset_from_table(lua_State* L, int table_pos, const model_observer_property_handler* const list, int size) {
      table_pos = lua_absindex(L, table_pos);
      int top = lua_gettop(L);
      //
      QMap<Qt::ItemDataRole, QVariant> out;
      for (int i = 0; i < size; ++i) {
         auto& moph = list[i];
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
      editor_script::util::ui::push_color(L, v.value<QColor>());
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