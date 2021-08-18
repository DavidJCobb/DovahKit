#include "table_view.h"
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include "../../../../../ui/generic/ObservableStandardItemModel.h"
#include "../../coordinator.h"
#include "../../events.h"

#include "../../../../wrappers/ui/table_view/cell.h"
#include "../../../../wrappers/ui/table_view/col.h"
#include "../../../../wrappers/ui/table_view/row.h"

namespace dovahscript::impl::event_registration {
   /*static*/ result table_view::register_event(QObject& object, const char* event_name, const char* listener_name) {
      auto* casted = qobject_cast<QTableView*>(&object);
      if (!casted)
         return result::no_match;
      if (_stricmp(event_name, "OnChanged") == 0) {
         std::string ln = listener_name;
         core::subsystems::events::get()._connect_event(
            get_passkey(),
            QObject::connect(casted->selectionModel(), &QItemSelectionModel::selectionChanged, &impl::get_event_connection_recipient(),
               [casted, ln]() {
                  //
                  // We can't easily tell from the signal alone whether the selection is supposed to be a row, 
                  // a column, or a cell, so we'll just check the widget itself to find out.
                  //
                  std::vector<QVariant> selections;
                  {
                     auto* sm    = casted->selectionModel();
                     auto* proxy = (QSortFilterProxyModel*) casted->model();
                     auto* model = (ObservableStandardItemModel*) proxy->sourceModel();
                     switch (casted->selectionBehavior()) {
                        case QAbstractItemView::SelectionBehavior::SelectRows:
                           for (auto& qmi : sm->selectedRows()) {
                              auto remapped = proxy->mapToSource(qmi);
                              //
                              model_observer_event_argument arg;
                              arg.observer      = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::rowOrientation, remapped.row());
                              arg.metatable_key = dovahscript::wrappers::ui::table_view_row::metatable_key;
                              assert(arg.observer);
                              selections.push_back(QVariant::fromValue(arg));
                           }
                           break;
                        case QAbstractItemView::SelectionBehavior::SelectColumns:
                           for (auto& qmi : sm->selectedColumns()) {
                              auto remapped = proxy->mapToSource(qmi);
                              //
                              model_observer_event_argument arg;
                              arg.observer      = model->getOrCreateRegisteredObserver(QModelIndex(), ObservableStandardItemModel::colOrientation, remapped.column());
                              arg.metatable_key = dovahscript::wrappers::ui::table_view_col::metatable_key;
                              assert(arg.observer);
                              selections.push_back(QVariant::fromValue(arg));
                           }
                           break;
                        case QAbstractItemView::SelectionBehavior::SelectItems:
                           for (auto& qmi : sm->selectedIndexes()) {
                              auto remapped = proxy->mapToSource(qmi);
                              //
                              model_observer_event_argument arg;
                              arg.observer      = model->getOrCreateRegisteredObserver(remapped);
                              arg.metatable_key = dovahscript::wrappers::ui::table_view_cell::metatable_key;
                              assert(arg.observer);
                              selections.push_back(QVariant::fromValue(arg));
                           }
                           break;
                     }
                  }
                  core::subsystems::events::get().receive_event_from_main_thread(*casted, "OnSelectionChanged", ln.c_str(), selections);
               }
            ),
            object, event_name, listener_name
         );
         return result::success;
      }
      return result::failure;
   }
}