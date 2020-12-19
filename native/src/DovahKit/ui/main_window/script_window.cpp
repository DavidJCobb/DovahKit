#include "script_window.h"
#include "../../editor/scripting/core.h"

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
      // TODO: run the current script
   });
   QObject::connect(this->ui.buttonToggleLog, &QPushButton::clicked, this, [this]() {
      // TODO: show/hide the log view
   });
}