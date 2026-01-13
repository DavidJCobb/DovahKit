#include "./ViewEventFilter_RemoveRowOnDelKey.h"
#include <QAbstractItemView>
#include <QKeyEvent>

namespace ui::model_utils {
   /*virtual*/ bool ViewEventFilter_RemoveRowOnDelKey::eventFilter(QObject* object, QEvent* event) /*override*/ {
      if (event->type() != QEvent::Type::KeyPress)
         return false;
      auto* key_event = static_cast<QKeyEvent*>(event);
      if (key_event->key() != Qt::Key::Key_Delete)
         return false;
      auto* view = qobject_cast<QAbstractItemView*>(object);
      if (!view)
         return false;
      if (auto* model = view->model()) {
         auto rows = view->selectionModel()->selectedRows();
         if (rows.size() < 2) {
            if (rows.size() == 1)
               this->_remove_row(*model, rows[0]);
         } else {
            //
            // No guarantees about the order of the rows AFAIK, so we have to 
            // convert the QMIs to QPMIs so that removal of any given row won't 
            // invalidate [our reference to] the next rows to be removed.
            //
            QList<QPersistentModelIndex> qpmi;
            for (auto& qmi : rows)
               qpmi.push_back(qmi);
            for (auto& item : qpmi)
               this->_remove_row(*model, item);
         }
      }
      return true;
   }

   void ViewEventFilter_RemoveRowOnDelKey::_remove_row(QAbstractItemModel& model, const QModelIndex& qmi) {
      if (this->_custom_remove_function)
         this->_custom_remove_function(&model, qmi);
      else
         model.removeRow(qmi.row(), qmi.parent());
   }
   void ViewEventFilter_RemoveRowOnDelKey::_remove_row(QAbstractItemModel& model, const QList<QPersistentModelIndex>& list) {
      if (this->_custom_remove_function) {
         for(auto& qpmi : list)
            this->_custom_remove_function(&model, qpmi);
      } else {
         for (auto& qpmi : list)
            model.removeRow(qpmi.row(), qpmi.parent());
      }
   }
}