#pragma once
#include <functional>
#include <QAbstractItemModel>
#include <QObject>

namespace ui::model_utils {
   class ViewEventFilter_RemoveRowOnDelKey : public QObject {
      Q_OBJECT;
      public:
         using QObject::QObject;

         using CustomRemoveFunction = std::function<void(QAbstractItemModel*, const QModelIndex&)>;

      public:
         virtual bool eventFilter(QObject* object, QEvent* event) override;

         void setCustomRemoveFunction(CustomRemoveFunction&& f) {
            this->_custom_remove_function = f;
         }

      protected:
         CustomRemoveFunction _custom_remove_function;

         void _remove_row(QAbstractItemModel&, const QModelIndex&);
         void _remove_row(QAbstractItemModel&, const QList<QPersistentModelIndex>&);
   };
}
