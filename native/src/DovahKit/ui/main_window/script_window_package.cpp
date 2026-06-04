#include "script_window_package.h"
#include <QCloseEvent>
#include <QDirIterator>
#include <QFileDialog>
#include <QMessageBox>
#include "../generic/DKLuaSyntaxHighlighter.h"
#include "dovahscript/dovahscript_host.h"
#include "editor/script_packages/manifest_parser.h"
#include "editor/subsystems/options/core.h"
#include "./script_window/hyperlink_confirm.h"
#include "./script_window_package/DovahscriptAuthorWidget.h"

namespace {
   static QDir _get_script_path() {
      return dovahkit::subsystems::options::core::get().get_user_script_package_path();
   }
}

EditorScriptPackageWindow::EditorScriptPackageWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   QObject::connect(this->ui.packagePicker, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorScriptPackageWindow::redrawPackage);
   QObject::connect(this->ui.buttonRefreshPackageList, &QPushButton::clicked, this, &EditorScriptPackageWindow::reloadPackageList);
   QObject::connect(this->ui.buttonBrowseForPackage, &QPushButton::clicked, this, &EditorScriptPackageWindow::browseForPackage);
   this->ui.packageBody->setCurrentIndex(0);

   {
      auto& dv = this->dovahkit_version;
      auto  v  = QApplication::applicationVersion();
      if (!v.isEmpty()) {
         auto m = QRegularExpression(R"--(^(\d+)\.(\d+)\.(\d+)\.(\d+)$)--").match(v);
         if (m.hasMatch()) {
            dv.major = m.capturedView(1).toInt();
            dv.minor = m.capturedView(2).toInt();
            dv.patch = m.capturedView(3).toInt();
            dv.build = m.capturedView(4).toInt();
         }
      }
   }
   {  // Font for script editor
      QFont font("Lucida Console", 10);
      font.setStyleHint(QFont::Monospace);
      this->ui.eval->setFont(font);
      //
      new DKLuaSyntaxHighlighter(this->ui.eval->document());
   }
   
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
   }
   
   #pragma region Script and log actions
      QObject::connect(this->ui.buttonRun, &QPushButton::clicked, this, &EditorScriptPackageWindow::runCurrentPackage);
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

void EditorScriptPackageWindow::browseForPackage() {
   auto path = QFileDialog::getExistingDirectory(this, tr("Select script package folder"), _get_script_path().path());
   if (path.isEmpty())
      return;
   auto load = QFile(QDir(path).absoluteFilePath("manifest.xml"));
   if (!load.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(this, tr("Failed to load script package"), tr("Unable to open the manifest.xml file. %1").arg(load.errorString()));
      return;
   }
   script_packages::manifest_parser parser;
   if (!parser.parse(load.readAll())) {
      QMessageBox::critical(this, tr("Failed to load script package"), tr("This folder's manifest.xml file is not a valid script package manifest:\n\n%1").arg(parser.error_text()));
      return;
   }
   auto  folder = path;
   auto* widget = this->ui.packagePicker;
   for (auto& prior : this->user_loaded_packages) {
      if (prior.root_folder == folder) {
         prior = parser.result();
         prior.root_folder.setPath(folder);
         //
         int i = widget->findData(prior.root_folder.path(), Qt::UserRole);
         if (i >= 0)
            widget->setCurrentIndex(i);
         //
         return;
      }
   }
   this->user_loaded_packages.push_back(parser.result());
   auto& added = this->user_loaded_packages.back();
   added.root_folder.setPath(folder);
   widget->addItem(added.name, added.root_folder.path());
   widget->setCurrentIndex(widget->count() - 1);
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
   {  // Update user-loaded packages
      QVector<int> to_remove;
      auto& list = this->user_loaded_packages;
      int   size = list.size();
      for (int i = 0; i < size; ++i) {
         auto& pack = list[i];
         //
         // If (pack) is a package in the normal script package folder, then the above link will 
         // have grabbed it. Remove it from the user-loaded package list.
         //
         auto  rel  = base.relativeFilePath(pack.root_folder.path());
         if (!rel.startsWith("../")) {
            if (rel.endsWith('/'))
               rel.chop(1);
            int seen = 0;
            for (int j = 0; j < rel.size(); ++j)
               if (rel[j] == '/')
                  if (++seen > 1)
                     break;
            if (seen <= 1) {
               to_remove.push_back(i);
               continue;
            }
         }
         //
         // Else, verify that the package still exists, and remove it from the user-loaded package 
         // list if it doesn't.
         //
         auto folder = pack.root_folder;
         auto path   = folder.absoluteFilePath("manifest.xml");
         //
         auto load = QFile(path);
         if (load.open(QIODevice::ReadOnly)) {
            script_packages::manifest_parser parser;
            if (parser.parse(load.readAll())) {
               pack = parser.result();
               pack.root_folder = folder;
               continue;
            }
            qDebug("Failed to refresh user-loaded manifest: %s\n - %s", parser.error_text(), path);
         } else {
            qDebug("Failed to refresh user-loaded manifest: could not open the file: %s\n -  %s", load.errorString(), path);
         }
         to_remove.push_back(i);
      }
      if (!to_remove.empty()) {
         size = to_remove.size();
         for (int i = 0; i < size; ++i)
            list.removeAt(to_remove[i] - i);
      }
   }
   //
   auto selection = widget->currentData().toString();
   widget->clear();
   for (auto& m : this->script_packages) {
      widget->addItem(m.name, m.root_folder.path());
   }
   for (auto& m : this->user_loaded_packages) {
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
   int tab_credits = this->ui.packageBody->indexOf(this->ui.packageTabAuthors);
   assert(tab_credits >= 0);
   //
   script_packages::manifest* manifest = this->_getSelectedManifest();
   if (!manifest) {
      this->ui.packageName->setText(tr("No package selected", "script package window"));
      this->ui.packageVersion->setText(QString());
      this->ui.packageVersion->setVisible(false);
      this->ui.packageBody->setTabVisible(tab_credits, false);
      this->ui.packageDescription->setText(tr("No description available.", "script package window"));
      return;
   }
   //
   this->ui.packageName->setText(manifest->name);
   this->ui.packageDescription->setText(manifest->description);
   {
      QString text;
      QLabel* widget = this->ui.packageVersion;
      //
      auto& vi = manifest->version_info;
      if (vi.package) {
         text = tr("version %1.%2.%3.%4", "package version info (package version)");
         if (vi.dovah_minimum) {
            text = tr((const char*)u8"version %1.%2.%3.%4 — supports DovahKit versions %5.%6.%7.%8 and up", "package version info (package version and minimum DovahKit version)");
         }
      } else if (vi.dovah_minimum) {
         text = tr("supports DovahKit versions %5.%6.%7.%8 and up", "package version info (minimum DovahKit version)");
      }
      if (!text.isEmpty()) {
         text = text
            .arg(vi.package.major)
            .arg(vi.package.minor)
            .arg(vi.package.patch)
            .arg(vi.package.build)
            .arg(vi.dovah_minimum.major)
            .arg(vi.dovah_minimum.minor)
            .arg(vi.dovah_minimum.patch)
            .arg(vi.dovah_minimum.build);
         widget->setText(text);
         widget->setVisible(true);
      } else {
         widget->setVisible(false);
         widget->setText(QString());
      }
   }
   {  // Authors
      auto* body   = this->ui.packageTabAuthors;
      auto* layout = body->layout();
      assert(layout);
      //
      QLayoutItem* child;
      while ((child = layout->takeAt(0)) != nullptr) {
         if (auto* widget = child->widget())
            delete widget;
         delete child;
      }
      //
      this->ui.packageBody->setTabVisible(tab_credits, !manifest->authors.isEmpty());
      for (auto& author : manifest->authors) {
         auto* widget = new DovahscriptAuthorWidget(body);
         layout->addWidget(widget);
         if (author.name.isEmpty()) {
            widget->setName(tr("Anonymous", "script package window - unnamed author"));
         } else {
            widget->setName(author.name);
         }
         for (auto& link : author.links)
            widget->addLink(link.name, link.url);
      }
      layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
   }
   return;
}

void EditorScriptPackageWindow::runCurrentPackage() {
   if (!this->isVisible())
      return;
   script_packages::manifest* manifest = this->_getSelectedManifest();
   if (!manifest)
      return;
   //
   if (auto& current = this->dovahkit_version) {
      if (auto& desired = manifest->version_info.dovah_minimum) {
         if (current < desired) {
            auto text = tr("This script was designed for DovahKit versions %1.%2.%3.%4 or newer. You are currently running DovahKit version %5.%6.%7.%8, so the script may not function as intended.\n\nRun it anyway?");
            text = text
               .arg(desired.major)
               .arg(desired.minor)
               .arg(desired.patch)
               .arg(desired.build)
               .arg(current.major)
               .arg(current.minor)
               .arg(current.patch)
               .arg(current.build);
            auto confirm = QMessageBox::question(this, tr("Run potentially unsupported script?"), text, QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::No) {
               return;
            }
         }
      }
   }
   //
   dovahscript::script_set request;
   request.package_folder_name = manifest->root_folder.path();
   for (auto& file : manifest->files) {
      if (!file.endsWith(".lua", Qt::CaseInsensitive))
         continue;
      auto path = manifest->root_folder.absoluteFilePath(file);
      auto rel  = manifest->root_folder.relativeFilePath(path);
      if (rel.startsWith("../")) {
         this->logMessage(tr("Blocked package from loading file outside of its folder: %1").arg(file));
         continue;
      }
      QFile code(path);
      if (code.open(QIODevice::ReadOnly)) {
         dovahscript::pending_script f;
         f.contents = code.readAll();
         f.filename = rel;
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
   //
   auto* pane = this->ui.paneLog;
   if (pane->collapsed()) {
      auto* header = this->subwidgets.log_header;
      header->setBadgeCount(header->badgeCount() + 1);
   }
}

script_packages::manifest* EditorScriptPackageWindow::_getSelectedManifest() noexcept {
   auto selection = this->ui.packagePicker->currentData().toString();
   if (!selection.isEmpty()) {
      for (auto& m : this->script_packages)
         if (m.root_folder == selection)
            return &m;
      for (auto& m : this->user_loaded_packages)
         if (m.root_folder == selection)
            return &m;
   }
   return nullptr;
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
   this->ui.actionRunEval->setEnabled(enable);
   this->ui.eval->setReadOnly(this->state.eval_pending);
}

bool EditorScriptPackageWindow::_checkAllowClose() {
   auto& host = DovahscriptHost::get();
   if (!host.is_running())
      return true;
   //
   bool result = false;
   host.setPaused(true);
   auto confirm = QMessageBox::question(this, tr("Abort the script?"), tr("A script is currently running. Do you want to force it to stop?"), QMessageBox::Yes | QMessageBox::No);
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