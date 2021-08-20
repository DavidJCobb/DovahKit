#include "model_observers.h"
#include "../../helpers/qt/get_model_of.h"
#include "../core/subsystems/lifetime.h"
#include "../tasks/s2m/ui_read_lambda.h"
#include "../tasks/s2m/ui_write_lambda.h"
#include "../send_script_task.h"

namespace dovahscript::api_helpers {
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
      send_script_ui_task(*task);
      delete task;
      //
      return result;
   }
   extern void set_model_items_data(ObservableStandardItemModelObserver* observer, int role, QVariant data) {
      auto* task = new tasks::s2m::ui_write_lambda(false);
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
      send_script_ui_task(*task);
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
      auto* task    = new tasks::s2m::ui_write_lambda(true);
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
      send_script_ui_task(*task);
      delete task;
      //
      core::subsystems::lifetime::get().zombify_all_invalid_model_observers();
   }
}