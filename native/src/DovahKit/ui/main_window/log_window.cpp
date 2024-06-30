#include "log_window.h"
#include <optional>
#include <QClipboard>
#include "widgets/DKHeaderView.h"
#include "./log_window/log_list_view.h"

LogWindow::LogWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   
   this->_model = new LogListModel(this);
   {
      constexpr const size_t icon_size = 16;

      auto* widget = this->ui.list;
      widget->setModel(this->_model);
      widget->setIconSize({ icon_size, icon_size });
      //
      // Headers:
      //
      {
         auto metrics = QFontMetrics(widget->font());
         if (auto* vh = widget->verticalHeader()) {
            vh->setDefaultSectionSize(metrics.height()); // nix the janky padding QTableView adds to rows by default (wow! what a good widget!)
         }
         {
            auto* header = new DKHeaderView(Qt::Orientation::Horizontal, this);
            header->setFlexResizeEnabled(true);
            widget->setHorizontalHeader(header);

            header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
            header->setMinimumSectionSize(2);
            header->setColumnFlex(LogListModel::Column::Type,    0, 0, icon_size + 8);
            header->setColumnFlex(LogListModel::Column::Context, 0, 0, icon_size + 8);
            header->setColumnFlex(LogListModel::Column::Text,    1, 1, 2);
            header->setColumnFlex(LogListModel::Column::File,    0, 0, metrics.boundingRect("Dragonborn.esm").width() * 1.5F + 4);
            header->setSectionResizeMode(LogListModel::Column::Text, QHeaderView::Interactive);
            header->setSectionResizeMode(LogListModel::Column::File, QHeaderView::Interactive);
            header->setStretchLastSection(false);
         }
      }
      //
      // Misc:
      //
      widget->setAlternatingRowColors(true);
      widget->setWordWrap(true);
   }
   
   if (auto* sm = this->ui.list->selectionModel()) {
      QObject::connect(sm, &QItemSelectionModel::currentRowChanged, this, &LogWindow::_redraw_selected_entry);
   }
   QObject::connect(this->ui.buttonCopySelected, &QPushButton::clicked, this, [this]() {
      QString out;
      //
      auto* widget = this->ui.list;
      auto* model  = widget->model();
      auto  sel    = widget->selectionModel()->selectedRows();
      for (const auto& index : sel) {
         int  i = index.row();
         auto i_text = model->index(i, LogListModel::Column::Text);
         auto i_file = model->index(i, LogListModel::Column::File);
         
         QString text = model->data(i_text, Qt::DisplayRole).toString();
         QString file = model->data(i_file, Qt::DisplayRole).toString();
         
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
   QObject::connect(this->ui.buttonClearAll, &QPushButton::clicked, this, [this]() {
      auto* widget = this->ui.list;
      if (auto* model = dynamic_cast<LogListModel*>(widget->model())) {
         model->clear();
      }
   });
}

void LogWindow::_redraw_selected_entry() {
   auto* widget = this->ui.list;

   std::optional<size_t> row;
   if (auto* sm = widget->selectionModel()) {
      auto qmi = sm->currentIndex();
      if (qmi.isValid()) {
         row = qmi.row();
      } else {
         auto rows = sm->selectedRows();
         if (rows.size())
            row = rows[0].row();
      }
   }
   if (!row.has_value()) {
      this->ui.detailsText->setText("No log entry selected");
      return;
   }

   auto* model = widget->model();

   auto qmi = model->index(row.value(), LogListModel::Column::Text, {});
   QString text = model->data(qmi, Qt::DisplayRole).toString();

   this->ui.detailsText->setText(text);
}