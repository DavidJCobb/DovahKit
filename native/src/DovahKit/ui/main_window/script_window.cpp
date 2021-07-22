#include "script_window.h"
#include "../../editor/scripting/editor_script_core.h"
#include "script_window/hyperlink_confirm.h"
#include <QMessageBox>

EditorScriptWindow::EditorScriptWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   {  // Font for script editor
      QFont font("Lucida Console", 10);
      font.setStyleHint(QFont::Monospace);
      this->ui.script->setFont(font);
   }
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
   //
   QObject::connect(this->ui.buttonLoad, &QPushButton::clicked, this, [this]() {
      // TODO: let the user load the text of a script from a file
   });
   QObject::connect(this->ui.buttonSave, &QPushButton::clicked, this, [this]() {
      // TODO: let the user save the text of a script to a file
   });
   QObject::connect(this->ui.buttonRun, &QPushButton::clicked, this, [this]() {
      if (!this->isVisible())
         return;
      auto& vm = DovahKitScriptVM::get();
      vm.runScript(this->ui.script->toPlainText(), "userscript");
   });
   QObject::connect(this->ui.buttonForceKill, &QPushButton::clicked, this, [this]() {
      DovahKitScriptVM::get().abort();
   });
   QObject::connect(this->ui.buttonToggleLog, &QPushButton::clicked, this, [this]() {
      // TODO: show/hide the log view
   });
   //
   auto& vm = DovahKitScriptVM::get();
   QObject::connect(&vm, &DovahKitScriptVM::scriptStarted, this, [this]() {
      this->_onScriptStartStop(true);
      QMessageBox::information(this, tr("Script started!", "script (debug)"), tr("The script has started."));
   });
   QObject::connect(&vm, &DovahKitScriptVM::scriptEnded, this, [this](bool error) {
      if (this->isVisible()) {
         if (error) {
            QMessageBox::information(this, tr("Script error!", "editor script"), tr("The script encountered an error."));
         } else {
            QMessageBox::information(this, tr("Script complete!", "editor script"), tr("The script ran to completion."));
         }
      }
      this->_onScriptStartStop(false);
   });
   QObject::connect(&vm, &DovahKitScriptVM::messageLogged, this, [this](const QString& text) {
      auto* widget = this->ui.log;
      auto  index  = widget->rowCount();
      widget->insertRow(index);
      widget->setItem(index, 0, new QTableWidgetItem(text));
   });
   QObject::connect(&vm, &DovahKitScriptVM::userClickedLink, this, [this](const QString& text, QWidget* opener) {
      auto* confirm = new ScriptWindowHyperlinkConfirmDialog(text, opener ? opener : this);
      confirm->open();
   });
   this->_onScriptStartStop(false);
}
void EditorScriptWindow::_onScriptStartStop(bool script_running) {
   this->ui.buttonLoad->setDisabled(script_running);
   this->ui.buttonSave->setDisabled(script_running);
   this->ui.buttonRun->setDisabled(script_running);
   this->ui.buttonForceKill->setDisabled(!script_running);
   this->ui.script->setDisabled(script_running);
}

bool EditorScriptWindow::_checkAllowClose() {
   auto& vm = DovahKitScriptVM::get();
   if (!vm.is_running())
      return true;
   //
   bool result = false;
   vm.setPaused(true);
   auto confirm = QMessageBox::question(this, "Abort the script?", "A script is currently running. Do you want to force it to stop?", QMessageBox::Yes | QMessageBox::No);
   if (confirm == QMessageBox::Yes) {
      DovahKitScriptVM::get().abort();
      result = true;
   }
   vm.setPaused(false);
   return result;
}
void EditorScriptWindow::closeEvent(QCloseEvent* event) {
   if (this->_checkAllowClose())
      event->accept();
   else
      event->ignore();
}
void EditorScriptWindow::reject() {
   if (this->_checkAllowClose())
      QDialog::reject();
}