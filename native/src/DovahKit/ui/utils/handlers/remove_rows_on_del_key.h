#pragma once
#include <functional>
#include <QObject>
#include <QPointer>
class QAbstractItemView;

namespace ui::utils::handlers {
   //
   // A fire-and-forget handler that can be installed on an item view. When the view 
   // has focus and the user presses the Del key, the selected row(s) will be deleted 
   // via a call to the model's `removeRows` override.
   //
   class remove_rows_on_del_key : public QObject {
      Q_OBJECT;
      public:
         remove_rows_on_del_key(QAbstractItemView&);

         // Creates an instance of the handler owned by the view, and installs it on the view.
         static remove_rows_on_del_key& install(QAbstractItemView&);

         constexpr const std::function<bool()>& confirmFunction() const noexcept { return this->_confirm; }

         // Optionally set a confirmation handler, e.g. show a confirmation prompt and return 
         // `true` if the user confirms the deletion.
         void setConfirmFunction(std::function<bool()>&&);

         virtual bool eventFilter(QObject* object, QEvent* event) override;

      protected:
         std::function<bool()> _confirm;
         QPointer<QAbstractItemView> _view;
   };
}