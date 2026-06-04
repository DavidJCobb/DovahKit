#include "./DKWinTaskbarButton.h"
#include "./DKWinTaskbarButtonPrivate.h"

DKWinTaskbarButton::DKWinTaskbarButton(QObject* parent) : QObject(parent) {
   this->_private = new DKWinTaskbarButtonPrivate;
   this->setWindow(qobject_cast<QWindow*>(parent));
}
DKWinTaskbarButton::~DKWinTaskbarButton() {
   delete this->_private;
   this->_private = nullptr;
}

QString DKWinTaskbarButton::overlayAccessibleDescription() const {
   return this->_private->overlay.accessible_description;
}
QIcon DKWinTaskbarButton::overlayIcon() const {
   return this->_private->overlay.icon;
}
DKWinTaskbarProgress* DKWinTaskbarButton::progress() const {
   return this->_private->progress;
}

void DKWinTaskbarButton::setWindow(QWindow* w) {
   if (this->_private->window == w)
      return;
   this->_private->window = w;
   this->_private->update_overlay();
   this->_private->update_progress();
}
QWindow* DKWinTaskbarButton::window() const {
   return this->_private->window;
}

/*virtual*/ bool DKWinTaskbarButton::eventFilter(QObject* subject, QEvent* event) /*override*/ {
   if (subject != this->_private->window)
      return false;
   if (event->type() != DKWinTaskbarButtonPrivate::event_type())
      return false;
   this->_private->update_overlay();
   this->_private->update_progress();
   return false;
}

void DKWinTaskbarButton::clearOverlayIcon() {
   this->_private->overlay = {};
   this->_private->update_overlay();
}
void DKWinTaskbarButton::setOverlayAccessibleDescription(QString s) {
   auto& dst = this->_private->overlay.accessible_description;
   if (dst == s)
      return;
   dst = s;
   this->_private->update_overlay();
}
void DKWinTaskbarButton::setOverlayIcon(QIcon v) {
   this->_private->overlay.icon = v;
   this->_private->update_overlay();
}

#pragma region Extensions beyond the original API
   void DKWinTaskbarButton::setOverlay(QIcon v, QString accessible_desc) {
      this->_private->overlay.icon = v;
      this->_private->overlay.accessible_description = accessible_desc;
      this->_private->update_overlay();
   }
#pragma endregion