#include "./remove_rows_on_del_key.h"
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QKeyEvent>

namespace ui::utils::handlers {
   remove_rows_on_del_key::remove_rows_on_del_key(QAbstractItemView& view) : QObject(&view) {
      this->_view = &view;
      view.installEventFilter(this);
   }

   /*static*/ remove_rows_on_del_key& remove_rows_on_del_key::install(QAbstractItemView& view) {
      // Qt's ownership model will take care of deleting the instance when the view dies.
      auto* handler = new remove_rows_on_del_key{ view };
      return *handler;
   }

   void remove_rows_on_del_key::setConfirmFunction(std::function<bool()>&& func) {
      this->_confirm = std::move(func);
   }

   /*virtual*/ bool remove_rows_on_del_key::eventFilter(QObject* object, QEvent* event) /*override*/ {
      constexpr const bool swallow_event = true;
      constexpr const bool pass_on_event = false;

      constexpr const bool default_result = pass_on_event;

      if (object != this->_view)
         return default_result;
      if (event->type() != QEvent::Type::KeyPress)
         return default_result;
      if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Delete)
         return default_result;

      auto* sm = this->_view->selectionModel();
      if (!sm)
         return swallow_event;
      auto* model = this->_view->model();
      if (!model)
         return swallow_event;
      auto sel = sm->selection();
      if (!sel.empty()) {
         if (this->_confirm) {
            bool proceed = (this->_confirm)();
            if (!proceed)
               return swallow_event;
         }
         for (auto& range : sel)
            model->removeRows(range.top(), range.height(), {});
      }

      return swallow_event;
   }
}