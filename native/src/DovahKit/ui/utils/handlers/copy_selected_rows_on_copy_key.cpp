#include "./copy_selected_rows_on_copy_key.h"
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QShortcut>

namespace ui::utils::handlers {
   copy_selected_rows_on_copy_key::copy_selected_rows_on_copy_key(QAbstractItemView& view) : QObject(&view) {
      this->_view     = &view;
      this->_shortcut = new QShortcut(&view);
      this->_shortcut->setKey(QKeySequence::StandardKey::Copy);
      QObject::connect(this->_shortcut, &QShortcut::activated, this, &copy_selected_rows_on_copy_key::_activated);
   }

   /*static*/ copy_selected_rows_on_copy_key& copy_selected_rows_on_copy_key::install(QAbstractItemView& view) {
      // Qt's ownership model will take care of deleting the instance when the view dies.
      auto* handler = new copy_selected_rows_on_copy_key{ view };
      return *handler;
   }

   void copy_selected_rows_on_copy_key::setCopyHeaders(bool v) {
      this->_copy_headers = v;
   }

   void copy_selected_rows_on_copy_key::_activated() {
      if (!this->_view)
         return;
      auto* model = this->_view->model();
      auto* sm    = this->_view->selectionModel();

      QString text;
      auto    rows = sm->selectedRows();
      if (!rows.isEmpty()) {
         const auto row_count = rows.size();
         const int  col_count = model->columnCount({});
         const auto print_row = [model, &text, row_count, col_count](const QModelIndex& row_qmi, int row_i) {
            QString row_text;
            for (int i = 0; i < col_count; ++i) {
               auto qmi  = row_qmi.siblingAtColumn(i);
               auto data = model->data(qmi, Qt::DisplayRole);
               if (data.isNull()) {
                  data = model->data(qmi, Qt::EditRole);
               }
               row_text += data.toString();
               if (i + 1 < col_count)
                  row_text += '\t';
            }
            text += row_text;
            if (row_i + 1 < row_count)
               text += "\r\n";
         };

         if (this->_copy_headers) {
            QString row_text;
            for (int i = 0; i < col_count; ++i) {
               auto data = model->headerData(i, Qt::Orientation::Horizontal, Qt::DisplayRole);
               row_text += data.toString();
               if (i + 1 < col_count)
                  row_text += '\t';
            }
            text += row_text;
            text += "\r\n- - -\r\n";
         }
         for (size_t row_i = 0; row_i < row_count; ++row_i) {
            print_row(rows[row_i], row_i);
         }
      }

      QClipboard* cb = QApplication::clipboard();
      if (cb)
         cb->setText(text);
   }
}