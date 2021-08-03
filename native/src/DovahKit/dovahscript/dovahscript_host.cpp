#include "dovahscript_host.h"
#include "core/subsystems/coordinator.h"

namespace {
   using coordinator_t = dovahscript::core::subsystems::coordinator;
}

DovahscriptHost::DovahscriptHost() {
   auto& coordinator_s = coordinator_t::get();
   QObject::connect(&coordinator_s, &coordinator_t::scriptStarted, this, [this]() {
      emit this->scriptStarted();
   });
   QObject::connect(&coordinator_s, &coordinator_t::scriptEnded, this, [this](bool b) {
      emit this->scriptEnded(b);
   });
   QObject::connect(&coordinator_s, &coordinator_t::messageLogged, this, [this](const QString& v) {
      emit this->messageLogged(v);
   });
}

bool DovahscriptHost::is_aborted() const noexcept {
   return coordinator_t::get().is_aborted();
}
bool DovahscriptHost::is_running() const noexcept {
   return coordinator_t::get().is_running();
}

void DovahscriptHost::abort() {
   coordinator_t::get().abort();
}
void DovahscriptHost::runScript(const QString& code, const QString& name) {
   dovahscript::script_set package;
   package.files.emplace_back(dovahscript::pending_script{
      .filename = name,
      .contents = code
   });
   coordinator_t::get().execute_scripts(std::move(package));
}
void DovahscriptHost::runScripts(dovahscript::script_set&& scripts) {
   coordinator_t::get().execute_scripts(std::move(scripts));
}
void DovahscriptHost::setPaused(bool b) {
   coordinator_t::get().set_pause_state(b);
}
void DovahscriptHost::setUIParentWidget(QWidget* widget) {
   coordinator_t::get().set_ui_parent(widget);
}