#include "script_window_package.h"
#include <QCloseEvent>
#include <QDirIterator>
#include <QMessageBox>
#include "../../dovahscript/dovahscript_host.h"
#include "../../editor/script_packages/manifest_parser.h"
#include "script_window/hyperlink_confirm.h"

namespace {
   static QDir _get_script_path() {
      auto path = QCoreApplication::applicationDirPath();
      auto dir = QDir(QDir(path).absoluteFilePath("userdata/script-packages/"));
      if (!dir.exists()) {
         dir = QDir::current().absoluteFilePath("userdata/script-packages/"); // During debugging, the program's path is at ./x64/ConfigurationName/ and the current working directory is at ./
      }
      return dir;
   }
}

EditorScriptPackageWindow::EditorScriptPackageWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   QObject::connect(this->ui.packagePicker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorScriptPackageWindow::redrawPackage);
   QObject::connect(this->ui.buttonRefreshPackageList, &QPushButton::clicked, this, &EditorScriptPackageWindow::reloadPackageList);
   
   this->_updateEvalEnableState();
   this->reloadPackageList();

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
   }
   
   #pragma region Script and log actions
      QObject::connect(this->ui.buttonRun, &QPushButton::clicked, this, &EditorScriptPackageWindow::runCurrentPackage);
      QObject::connect(this->ui.buttonForceKill, &QPushButton::clicked, this, [this]() {
         DovahscriptHost::get().abort();
      });
      QObject::connect(this->ui.buttonToggleLog, &QPushButton::clicked, this, [this]() {
         this->ui.log->setVisible(!this->ui.log->isVisible());
      });
      QObject::connect(this->ui.buttonClearLog, &QPushButton::clicked, this, [this]() {
         this->ui.log->setRowCount(0);
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
      QObject::connect(&host, &DovahscriptHost::messageLogged, this, &EditorScriptPackageWindow::logMessage);
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

void EditorScriptPackageWindow::reloadPackageList() {
   auto* widget = this->ui.packagePicker;
   const QSignalBlocker blocker(widget);
   //
   this->script_packages.clear();
   //
   QDir base = _get_script_path();
   auto it   = QDirIterator(base, QDirIterator::NoIteratorFlags);
   while (it.hasNext()) {
      QDir current = it.next();
      auto path    = current.absoluteFilePath("manifest.xml");
      //
      auto load = QFile(path);
      if (load.open(QIODevice::ReadOnly)) {
         script_packages::manifest_parser parser;
         if (parser.parse(load.readAll())) {
            this->script_packages.push_back(parser.result());
            this->script_packages.back().root_folder = current;
         } else {
            qDebug("Failed to load manifest: %s\n - %s", parser.error_text(), path); // TODO: report this somewhere the user can see
         }
      }
   }
   //
   auto selection = widget->currentData().toString();
   widget->clear();
   for (auto& m : this->script_packages) {
      widget->addItem(m.name, m.root_folder.path());
   }
   if (!selection.isEmpty()) {
      auto i = widget->findData(selection);
      if (i >= 0)
         widget->setCurrentIndex(i);
   }
   this->redrawPackage();
}

void EditorScriptPackageWindow::redrawPackage() {
   script_packages::manifest* manifest = nullptr;
   //
   auto selection = this->ui.packagePicker->currentData().toString();
   if (!selection.isEmpty()) {
      for (auto& m : this->script_packages) {
         if (m.root_folder == selection) {
            manifest = &m;
            break;
         }
      }
   }
   if (!manifest) {
      this->ui.packageName->setText(tr("No package selected", "script package window"));
      this->ui.packageDescription->setText(tr("No description available.", "script package window"));
      return;
   }
   //
   this->ui.packageName->setText(manifest->name);
   this->ui.packageDescription->setText(manifest->description);
   return;
}

void EditorScriptPackageWindow::runCurrentPackage() {
   if (!this->isVisible())
      return;
   script_packages::manifest* manifest = nullptr;
   //
   auto selection = this->ui.packagePicker->currentData().toString();
   if (!selection.isEmpty()) {
      for (auto& m : this->script_packages) {
         if (m.root_folder == selection) {
            manifest = &m;
            break;
         }
      }
   }
   if (!manifest)
      return;
   //
   dovahscript::script_set request;
   request.package_folder_name = manifest->root_folder.path();
   for (auto& file : manifest->files) {
      if (!file.endsWith(".lua", Qt::CaseInsensitive))
         continue;
      auto path = manifest->root_folder.absoluteFilePath(file);
      if (manifest->root_folder.relativeFilePath(path).startsWith("../")) {
         this->logMessage(tr("Blocked package from loading file outside of its folder: %1").arg(file));
         continue;
      }
      QFile code(path);
      if (code.open(QIODevice::ReadOnly)) {
         dovahscript::pending_script f;
         f.contents = code.readAll();
         f.filename = QFileInfo(code).fileName();
         request.files.push_back(std::move(f));
      } else {
         this->logMessage(tr("Failed to open file requested by package: %1").arg(file));
      }
   }
   DovahscriptHost::get().runScripts(request);
}

void EditorScriptPackageWindow::logMessage(const QString& text) {
   auto* widget = this->ui.log;
   auto  index  = widget->rowCount();
   widget->insertRow(index);
   widget->setItem(index, 0, new QTableWidgetItem(text));
}

void EditorScriptPackageWindow::_onScriptStartStop(bool script_running) {
   this->state.script_running = script_running;
   if (!script_running)
      this->state.eval_pending = false;
   //
   this->ui.buttonRun->setDisabled(script_running);
   this->ui.buttonForceKill->setDisabled(!script_running);
   //
   this->_updateEvalEnableState();
}
void EditorScriptPackageWindow::_updateEvalEnableState() {
   bool enable = this->state.script_running && !this->state.eval_pending;
   //this->ui.buttonEval->setEnabled(enable);
   //this->ui.eval->setReadOnly(!enable);
}

bool EditorScriptPackageWindow::_checkAllowClose() {
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
void EditorScriptPackageWindow::closeEvent(QCloseEvent* event) {
   if (this->_checkAllowClose())
      event->accept();
   else
      event->ignore();
}
void EditorScriptPackageWindow::reject() {
   if (this->_checkAllowClose())
      QDialog::reject();
}