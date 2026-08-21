#pragma once
#include <QObject>
#include <QPointer>
class QAbstractItemView;
class QShortcut;

namespace ui::utils::handlers {
   //
   // A fire-and-forget handler that can be installed on an item view. When the view 
   // has focus and the user presses Ctrl + C, the selected row(s) will be copied as 
   // plaintext (i.e. Qt::DisplayRole), with columns separated by tabs.
   //
   class copy_selected_rows_on_copy_key : public QObject {
      Q_OBJECT;
      public:
         copy_selected_rows_on_copy_key(QAbstractItemView&);

         // Creates an instance of the handler owned by the view, and installs it on the view.
         static copy_selected_rows_on_copy_key& install(QAbstractItemView&);

         void setCopyHeaders(bool);

      protected:
         QPointer<QAbstractItemView> _view;
         QPointer<QShortcut> _shortcut;
         bool _copy_headers = false;

         void _activated();
   };
}