#include "model_observer_data.h"
#include "../../../systems/messaging.h"
#include "../../../cross_thread_tasks/s2m/lambda.h"

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