#include "script_window_single.h"
#include "dovahscript/dovahscript_host.h"
#include "editor/subsystems/options/core.h"
#include "./script_window/hyperlink_confirm.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSaveFile>
#include <QTextStream>
#include "../generic/DKLuaSyntaxHighlighter.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMenu>
#include <QAction>

namespace {
   static QDir _get_script_path() {
      return dovahkit::subsystems::options::core::get().get_user_script_path();
   }
}

EditorSingleScriptWindow::EditorSingleScriptWindow(QWidget* parent) : QMainWindow(parent) {
   ui.setupUi(this);
   //
   #pragma region UI configuration
   {  // Font for script editor
      QFont font("Lucida Console", 10);
      font.setStyleHint(QFont::Monospace);
      this->ui.script->setFont(font);
      this->ui.eval->setFont(font);
      //
      new DKLuaSyntaxHighlighter(this->ui.script->document());
      new DKLuaSyntaxHighlighter(this->ui.eval->document());
   }
   this->_updateEvalEnableState();
   {  // Visuals for log pane
      auto* widget = this->ui.log;
      widget->setAlternatingRowColors(true);
      widget->setWordWrap(true);
      //
      widget->verticalHeader()->setDefaultSectionSize(0);
      //
      // The next call is needed for proper word-wrapping in table cells. The "wordWrap" 
      // property on table cells enables word-wrapping if the cell is tall enough, but 
      // doesn't actually resize table cells, so by default, the table behaves exactly 
      // as if word-wrapping were disabled. The next call automatically resizes cells 
      // by way of the vertical header: even if we disable the vertical header, every 
      // row still has a vertical header section associated with it, and that can be 
      // configured to resize.
      //
      // Naturally, pretty much none of this information is mentioned in the Qt docs 
      // for QTableView::setWordWrap, at least as of this writing.
      //
      widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for proper word-wrapping in table cells
      //
      // Header:
      //
      {
         auto* header = this->subwidgets.log_header = new DKUnreadCountBadgePaneHeader(this);
         header->setBadgeFormat(tr("%1 unread", "script log pane"));
         this->ui.paneLog->setTitleWidget(header);
         //
         QObject::connect(this->ui.paneLog, &DKCollapsiblePane::contentsExpanded, this, [header]() {
            header->setBadgeCount(0);
         });
      }
      //
      // Context menu:
      //
      {
         widget->setContextMenuPolicy(Qt::CustomContextMenu);
         auto* menu = new QMenu(this);
         {
            auto* action = new QAction(tr("Copy log item"), menu);
            menu->addAction(action);
            QObject::connect(action, &QAction::triggered, action, [widget]() {
               auto* item = widget->currentItem();
               if (!item)
                  return;
               auto data = item->data(Qt::UserRole).toString();
               QGuiApplication::clipboard()->setText(data);
            });
         }
         QObject::connect(widget, &QWidget::customContextMenuRequested, menu, [widget, menu](const QPoint& pos) {
            menu->popup(widget->mapToGlobal(pos));
         });
      }
   }
   #pragma endregion
   //
   #pragma region Menu bar
      {  // File
         QObject::connect(this->ui.actionLoadScript, &QAction::triggered, this, [this]() {
            auto path = QFileDialog::getOpenFileName(this, tr("Select script file"), _get_script_path().path(), tr("Lua scripts (*.lua)"));
            if (path.isEmpty())
               return;
            auto file = QFile(path);
            if (!file.open(QIODevice::OpenModeFlag::ExistingOnly | QIODevice::OpenModeFlag::ReadOnly)) {
               QMessageBox::critical(this, tr("Error"), tr("Unable to open the file."));
               return;
            }
            if (!this->ui.script->toPlainText().isEmpty()) {
               auto choice = QMessageBox::question(
                  this,
                  tr("Are you sure?"),
                  tr("Replace the currently loaded script with this file?"),
                  QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                  QMessageBox::StandardButton::NoButton
               );
               if (choice == QMessageBox::StandardButton::NoButton)
                  return;
            }
            this->ui.script->setPlainText(file.readAll());
         });
         QObject::connect(this->ui.actionSaveScript, &QAction::triggered, this, [this]() {
            auto path = QFileDialog::getSaveFileName(this, tr("Select script file"), _get_script_path().path(), tr("Lua scripts (*.lua)"));
            if (path.isEmpty())
               return;
            auto file = QSaveFile(path);
            if (!file.open(QIODevice::OpenModeFlag::WriteOnly)) {
               QMessageBox::critical(this, tr("Error"), tr("Unable to open the file for writing."));
               return;
            }
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Encoding::Utf8);
            stream << this->ui.script->toPlainText();
            file.commit();
         });
      }
      {  // Format
         this->ui.actionWordWrap->setChecked(this->ui.script->wordWrapMode() != QTextOption::NoWrap);
         QObject::connect(this->ui.actionWordWrap, &QAction::toggled, this, [this](bool checked) {
            this->ui.script->setWordWrapMode(checked ? QTextOption::WordWrap : QTextOption::NoWrap);
         });
      }
   #pragma endregion
   //
   #pragma region Script and log actions
      QObject::connect(this->ui.buttonRun, &QPushButton::clicked, this, [this]() {
         if (!this->isVisible())
            return;
         DovahscriptHost::get().runScript(this->ui.script->toPlainText());
      });
      QObject::connect(this->ui.buttonForceKill, &QPushButton::clicked, this, [this]() {
         DovahscriptHost::get().abort();
      });
      QObject::connect(this->ui.actionClearLog, &QAction::triggered, this, [this]() {
         this->ui.log->setRowCount(0);
      });
      QObject::connect(this->ui.actionRunEval, &QAction::triggered, this, [this]() {
         if (this->state.eval_pending || !this->state.script_running)
            return;
         if (!this->isVisible())
            return;
         QString code = this->ui.eval->toPlainText();
         if (code.isEmpty())
            return;
         if (DovahscriptHost::get().evalScript(code)) {
            this->state.eval_pending = true;
            this->_updateEvalEnableState();
         }
      });
   #pragma endregion
   #pragma region Script execution event handlers
      auto& host = DovahscriptHost::get();
      QObject::connect(&host, &DovahscriptHost::scriptStarted, this, [this]() {
         this->_onScriptStartStop(true);
         QMessageBox::information(this, tr("Script started!", "script (debug)"), tr("The script has started."));
      });
      QObject::connect(&host, &DovahscriptHost::scriptEnded, this, [this]() {
         if (this->isVisible()) {
            QMessageBox::information(this, tr("Script stopped", "editor script"), tr("Script execution has ended."));
         }
         this->_onScriptStartStop(false);
      });
      QObject::connect(&host, &DovahscriptHost::messageLogged, this, [this](const QString& text) {
         auto* widget = this->ui.log;
         auto  index  = widget->rowCount();
         widget->insertRow(index);
         
         auto* item = new QTableWidgetItem;
         {
            constexpr const size_t max_lines  = 8;
            constexpr const size_t max_length = 4096;

            auto view = QStringView(text);
            if (text.size() > max_length) {
               view = view.slice(0, max_length);
            }
            int end = view.size();
            int i   = view.indexOf('\n');
            {
               int lines = 0;
               while (i >= 0) {
                  ++lines;
                  if (lines > max_lines) {
                     end = i;
                     break;
                  }
                  i = view.indexOf('\n', i + 1);
               }
            }
            if (end == text.size()) {
               item->setData(Qt::DisplayRole, text);
            } else {
               item->setData(Qt::DisplayRole, view.slice(0, end).toString());
            }
         }
         item->setData(Qt::UserRole, text);
         widget->setItem(index, 0, item);

         auto* pane = this->ui.paneLog;
         if (pane->collapsed()) {
            auto* header = this->subwidgets.log_header;
            header->setBadgeCount(header->badgeCount() + 1);
         }
      });
      QObject::connect(&host, &DovahscriptHost::evalComplete, this, [this]() {
         this->state.eval_pending = false;
         this->_updateEvalEnableState();
      });
      QObject::connect(&host, &DovahscriptHost::userClickedLink, this, [this](const QString& text, QWidget* opener) {
         auto* confirm = new ScriptWindowHyperlinkConfirmDialog(text, opener ? opener : this);
         confirm->open();
      });
      this->_onScriptStartStop(false);
   #pragma endregion
}
void EditorSingleScriptWindow::_onScriptStartStop(bool script_running) {
   this->state.script_running = script_running;
   if (!script_running)
      this->state.eval_pending = false;
   //
   this->ui.buttonRun->setDisabled(script_running);
   this->ui.buttonForceKill->setDisabled(!script_running);
   this->ui.script->setReadOnly(script_running);
   //
   this->ui.actionLoadScript->setDisabled(script_running);
   this->ui.actionSaveScript->setDisabled(script_running);
   //
   this->_updateEvalEnableState();
}

void EditorSingleScriptWindow::_updateEvalEnableState() {
   bool enable = this->state.script_running && !this->state.eval_pending;
   this->ui.actionRunEval->setEnabled(enable);
   this->ui.eval->setReadOnly(this->state.eval_pending);
}

bool EditorSingleScriptWindow::_checkAllowClose() {
   auto& host = DovahscriptHost::get();
   if (!host.is_running())
      return true;
   //
   bool result = false;
   host.setPaused(true);
   auto confirm = QMessageBox::question(this, "Abort the script?", "A script is currently running. Do you want to force it to stop?", QMessageBox::Yes | QMessageBox::No);
   if (confirm == QMessageBox::Yes) {
      host.abort();
      result = true;
   }
   host.setPaused(false);
   return result;
}
void EditorSingleScriptWindow::closeEvent(QCloseEvent* event) {
   if (this->_checkAllowClose()) {
      event->accept();
      emit closed();
   } else
      event->ignore();
}
/*void EditorSingleScriptWindow::reject() {
   if (this->_checkAllowClose())
      QDialog::reject();
}*/