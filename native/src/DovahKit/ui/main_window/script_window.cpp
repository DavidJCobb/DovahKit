#include "script_window.h"
#include "../../editor/scripting/editor_script_core.h"
#include <QMessageBox>

EditorScriptWindow::EditorScriptWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   QObject::connect(this->ui.buttonLoad, &QPushButton::clicked, this, [this]() {
      // TODO: let the user load the text of a script from a file
   });
   QObject::connect(this->ui.buttonSave, &QPushButton::clicked, this, [this]() {
      // TODO: let the user save the text of a script to a file
   });
   QObject::connect(this->ui.buttonRun, &QPushButton::clicked, this, [this]() {
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
   vm.adopt_from_owner_thread();
   QObject::connect(&vm, &DovahKitScriptVM::scriptStarted, this, [this]() {
      this->_setUILocked(true);
      QMessageBox::information(this, tr("Script started!", "script (debug)"), tr("The script has started."));
   });
   QObject::connect(&vm, &DovahKitScriptVM::scriptEnded, this, [this](bool error) {
      if (error) {
         QMessageBox::information(this, tr("Script error!", "editor script"), tr("The script encountered an error."));
      } else {
         QMessageBox::information(this, tr("Script complete!", "editor script"), tr("The script ran to completion."));
      }
      this->_setUILocked(false);
   });
   QObject::connect(&vm, &DovahKitScriptVM::messageLogged, this, [this](const QString& text) {
      auto* widget = this->ui.log;
      auto  index  = widget->rowCount();
      widget->insertRow(index);
      auto* item   = new QTableWidgetItem(text);
      widget->setItem(index, 0, item);
   });
   //
   // TODO: Closing the window should pop a confirmation prompt asking the user whether they want to terminate any currently-running script.
   // Anything that force-closes the window should force-terminate the script.
   //
   this->_setUILocked(false);
}
void EditorScriptWindow::_setUILocked(bool locked) { // TODO: rename this to reflect that it syncs the controls to whether a script is running
   this->ui.buttonLoad->setDisabled(locked);
   this->ui.buttonSave->setDisabled(locked);
   this->ui.buttonRun->setDisabled(locked);
   this->ui.buttonForceKill->setDisabled(!locked);
   this->ui.script->setDisabled(locked);
}