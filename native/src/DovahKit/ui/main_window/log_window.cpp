#include "log_window.h"
#include <optional>
#include <QApplication>
#include <QClipboard>
#include <QMdiSubWindow>
#include <QMetaMethod> // for QObject::disconnect
#include "widgets/DKHeaderView.h"
#include "editor/subsystems/message_log/core.h"
#include "editor/subsystems/message_log/model.h"

namespace {
   using logging_subsystem = dovahkit::subsystems::message_log::core;
}

LogWindow::LogWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);

   this->_model = logging_subsystem::get_or_create().model();
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
            header->setColumnFlex(model_type::Column::Type,    0, 0, icon_size + 8);
            header->setColumnFlex(model_type::Column::Context, 0, 0, icon_size + 8);
            header->setColumnFlex(model_type::Column::Text,    1, 1, 2);
            header->setColumnFlex(model_type::Column::File,    0, 0, metrics.boundingRect("Dragonborn.esm").width() * 1.5F + 4);
            header->setSectionResizeMode(model_type::Column::Text, QHeaderView::Interactive);
            header->setSectionResizeMode(model_type::Column::File, QHeaderView::Interactive);
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
         auto i_text = model->index(i, model_type::Column::Text);
         auto i_file = model->index(i, model_type::Column::File);
         
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
      if (auto* model = dynamic_cast<model_type*>(widget->model())) {
         model->clear();
         this->_redraw_selected_entry();
      }
   });

   //
   // Y'know, there are a lot of ways to design the Q<Whatever>Application family of 
   // singletons to account for the possibility of a program running headless. This... 
   // is not the design I would've chosen:
   // 
   //  - QApplication is the most generic classname but refers to the most specific 
   //    subclass (for a GUI with a specific dependence on the QtWidgets sub-library)
   // 
   //  - QCoreApplication is the most generic class but has a less generic classname 
   //    than one of its subclasses, and an equally generic classname to the rest of 
   //    them
   // 
   //  - Subclasses of QCoreApplication don't override the getter to return specific 
   //    classes, so constant manual casting is necessary
   //
   if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
      QObject::connect(app, &QApplication::focusChanged, this, [this](QWidget* prior, QWidget* after) {
         logging_subsystem::get().set_log_ui_has_focus(this->hasFocus());
      });
   }
}
LogWindow::~LogWindow() {
   logging_subsystem::get().set_log_ui_has_focus(false);
}

bool LogWindow::hasFocus() const {
   auto* focused = QGuiApplication::focusObject();
   if (!focused)
      return false;
   if (focused == this)
      return true;
   while (focused = focused->parent()) {
      if (focused == this)
         return true;
   }
   return false;
}

/*virtual*/ void LogWindow::changeEvent(QEvent* event) /*override*/ {
   if (event->type() != QEvent::Type::ParentChange)
      return;
   this->_update_mdi_focus_handler();
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

   auto qmi = model->index(row.value(), model_type::Column::Text, {});
   QString text = model->data(qmi, Qt::DisplayRole).toString();

   this->ui.detailsText->setText(text);
}

void LogWindow::_mdi_focus_handler(Qt::WindowStates prior, Qt::WindowStates after) {
   logging_subsystem::get().set_log_ui_has_focus(after & Qt::WindowState::WindowActive);
}
void LogWindow::_update_mdi_focus_handler() {
   if (this->_last_qmi_parent) {
      auto* prior = (QMdiSubWindow*)this->_last_qmi_parent.data();
      QObject::disconnect(prior, &QMdiSubWindow::windowStateChanged, (QObject*)this, nullptr);
   }
   auto* after = qobject_cast<QMdiSubWindow*>(this->parent());
   this->_last_qmi_parent = after;
   if (after) {
      QObject::connect(after, &QMdiSubWindow::windowStateChanged, this, &LogWindow::_mdi_focus_handler);
   }
}