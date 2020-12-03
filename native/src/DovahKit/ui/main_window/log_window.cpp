#include "log_window.h"
#include "../../helpers/qt/strings.h"
#include "../../editor/core.h"
#include "../../dovah/files/file_read_warning.h"
#include "../../dovah/notice_code_list.h"
#include <QClipboard>

LogWindow::LogWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   this->ui.list->setAlternatingRowColors(true);
   this->ui.list->setWordWrap(true);
   //
   QObject::connect(this->ui.buttonCopySelected, &QPushButton::clicked, this, [this]() {
      QString out;
      //
      auto* widget = this->ui.list;
      auto* model  = widget->model();
      auto  sel    = widget->selectionModel()->selectedRows();
      for (auto index : sel) {
         int  i = index.row();
         auto i_text = model->index(i, 0);
         auto i_file = model->index(i, 1);
         //
         QString text = model->data(i_text, Qt::DisplayRole).toString();
         QString file = model->data(i_file, Qt::DisplayRole).toString();
         //
         if (text.isEmpty())
            continue;
         if (!out.isEmpty())
            out += tr("\n\n");
         if (!file.isEmpty())
            out += tr("[%1]\n").arg(file);
         out += text;
      }
      if (out.isEmpty())
         return;
      QClipboard* clipboard = QApplication::clipboard();
      clipboard->setText(out);
      
   });
   QObject::connect(this->ui.buttonCopyAll, &QPushButton::clicked, this, [this]() {
      QString out;
      //
      auto* widget = this->ui.list;
      auto* model  = widget->model();
      int   count  = model->rowCount();
      for (int i = 0; i < count; ++i) {
         auto i_text = model->index(i, 0);
         auto i_file = model->index(i, 1);
         //
         QString text = model->data(i_text, Qt::DisplayRole).toString();
         QString file = model->data(i_file, Qt::DisplayRole).toString();
         //
         if (text.isEmpty())
            continue;
         if (!out.isEmpty())
            out += tr("\n\n");
         if (!file.isEmpty())
            out += tr("[%1]\n").arg(file);
         out += text;
      }
      if (out.isEmpty())
         return;
      QClipboard* clipboard = QApplication::clipboard();
      clipboard->setText(out);
   });
}