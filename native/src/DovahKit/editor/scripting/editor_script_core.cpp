#include "editor_script_core.h"
#include "systems/editor_script_inner_core.h"

DovahKitScriptVM::DovahKitScriptVM() {
   auto& core = DovahKitScriptVMCore::get();
   QObject::connect(&core, &DovahKitScriptVMCore::scriptStarted, this, [this]() {
      emit this->scriptStarted();
   });
   QObject::connect(&core, &DovahKitScriptVMCore::scriptEnded, this, [this](bool b) {
      emit this->scriptEnded(b);
   });
   QObject::connect(&core, &DovahKitScriptVMCore::messageLogged, this, [this](const QString& v) {
      emit this->messageLogged(v);
   });
}

bool DovahKitScriptVM::is_aborted() const noexcept {
   return DovahKitScriptVMCore::get().is_aborted();
}
bool DovahKitScriptVM::is_running() const noexcept {
   return DovahKitScriptVMCore::get().is_running();
}

void DovahKitScriptVM::abort() {
   DovahKitScriptVMCore::get().abort();
}
void DovahKitScriptVM::runScript(const QString& code, const QString& name) {
   DovahKitScriptVMCore::get().runScript(code, name);
}
void DovahKitScriptVM::setUIParentWidget(QWidget* widget) {
   DovahKitScriptVMCore::get().setUIParentWidget(widget);
}