#include "DKKeyPickerWidget.h"
#include <QBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#if !defined(QT_DESIGNER_LIB)
#else
#endif

DKKeyPickerWidget::DKKeyPickerWidget(QWidget* parent) : QWidget(parent) {
   auto* le = this->subwidgets.line = new QLineEdit(this);
   #if defined(QT_DESIGNER_LIB)
      //
      // Setting the line-edit as read-only will prevent a text-editing cursor from 
      // being displayed, which leaves no obvious visual indicator of when it has 
      // focus. Handling keyboard events is generally sufficient to prevent the 
      // value from being edited, but we only do that outside of Qt Designer.
      //
      le->setReadOnly(true);
   #endif
   le->installEventFilter(this);
   //
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
   layout->addWidget(le);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   this->setFocusPolicy(Qt::TabFocus);
   this->setFocusProxy(le);
}

QVector<cobb::qt::key> DKKeyPickerWidget::keys() const {
   QVector<cobb::qt::key> out;
   for (auto& k : this->state.keys)
      out.push_back((cobb::qt::key)k);
   return out;
}
QString DKKeyPickerWidget::toString() const {
   QString name;
   //
   #if !defined(QT_DESIGNER_LIB)
      auto& list = this->state.keys;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         const auto& key = list[i];
         auto here = key.toString(true);
         if (!here.isEmpty()) {
            if (!name.isEmpty())
               name += " + ";
            name += here;
         }
      }
      //
      this->subwidgets.line->setText(name);
   #endif
   //
   return name;
}

void DKKeyPickerWidget::setAllowKeyCombinations(bool b) {
   if (this->allowKeyCombinations() == b)
      return;
   this->props.allowKeyCombinations = b;
   if (!b) {
      auto& list = this->state.keys;
      if (list.size() > 1) {
         size_t first = 0;
         for (size_t i = 0; i < list.size(); ++i) {
            if (list[i].still_down) {
               first = i;
               break;
            }
         }
         if (first) {
            auto k = list[first];
            list.clear();
            list.push_back(k);
         } else {
            list.resize(1);
         }
      }
   }
}
void DKKeyPickerWidget::setKeys(const QVector<cobb::qt::key>& keys) {
   auto& list = this->state.keys;
   list.clear();
   list.reserve(keys.size());
   for (auto& k : keys)
      list.push_back(k);
   //
   this->_redraw();
}

void DKKeyPickerWidget::_redraw() {
   this->subwidgets.line->setText(this->toString());
}

#if !defined(QT_DESIGNER_LIB)
void DKKeyPickerWidget::_keyDown(QKeyEvent* event) {
   if (event->isAutoRepeat())
      return;
   //
   _key k = cobb::qt::key::from_qt_event(event);
   k.still_down = true;
   //
   if (!this->allowKeyCombinations()) {
      this->state.keys.clear();
      this->state.keys.push_back(k);
      this->_redraw();
      return;
   }
   //
   auto& list = this->state.keys;
   list.erase(
      std::remove_if(
         list.begin(),
         list.end(),
         [k](const _key& entry) {
            return !entry.still_down || entry == k;
         }
      ),
      list.end()
   );
   list.push_back(k);
   this->_redraw();
}
void DKKeyPickerWidget::_keyUp(QKeyEvent* event) {
   auto k = cobb::qt::key::from_qt_event(event);
   for (auto& entry : this->state.keys) {
      if (entry == k) {
         entry.still_down = false;
         break;
      }
   }
}
#endif
bool DKKeyPickerWidget::eventFilter(QObject* target, QEvent* event) {
   if (target != this->subwidgets.line)
      return false;
   #if !defined(QT_DESIGNER_LIB)
      switch (event->type()) {
         case QEvent::Type::KeyPress:
            this->_keyDown((QKeyEvent*)event);
            return true;
         case QEvent::Type::KeyRelease:
            this->_keyUp((QKeyEvent*)event);
            return true;
         case QEvent::Type::FocusOut:
            for (auto& k : this->state.keys)
               k.still_down = false;
            return false;
            //
         case QEvent::InputMethod:
         case QEvent::InputMethodQuery:
            //
            // Reject IME events; this widget is for setting keyboard binds, not IME binds. 
            // Qt doesn't afford us a way to handle individual IME glyphs or "keypresses."
            //
            return true;
            //
         case QEvent::LanguageChange:
            this->_redraw();
            return false;
            //
         case QEvent::ShortcutOverride: // ensure we don't trigger keyboard shortcuts while remapping (can't prevent Windows-level ones like Alt+F4)
            event->accept();
            return true;
      }
   #endif
   return false;
}