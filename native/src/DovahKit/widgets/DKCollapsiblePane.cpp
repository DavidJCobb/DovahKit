#include "DKCollapsiblePane.h"
#include <QAction>
#include <QActionEvent>
#include <QBoxLayout>

namespace {
   static struct {
      QIcon collapse;
      QIcon expand;
      bool initialized = false;

      void setup() {
         if (this->initialized)
            return;
         this->initialized = true;
         this->collapse = QIcon(":/widget-assets/collapse.png");
         this->collapse.addFile(":/widget-assets/collapse-16.png", { 16, 16 });
         this->expand = QIcon(":/widget-assets/expand.png");
         this->expand.addFile(":/widget-assets/expand-16.png", { 16, 16 });
      }
   } icons;
}

DKCollapsiblePane::DKCollapsiblePane(QWidget* parent) : QFrame(parent), toolbar(*this) {
   this->setFrameStyle(QFrame::Sunken);
   this->setFrameShape(QFrame::Shape::WinPanel);
   //
   auto* title   = this->subwidgets.title  = new QFrame(this);
   auto* label   = this->subwidgets.label  = new QLabel(title);
   auto* toolbar = this->toolbar.container;
   auto* toggle  = this->subwidgets.toggle = new QPushButton(title);
   auto* body    = this->subwidgets.body   = new QWidget(this);
   {
      title->setFrameStyle(QFrame::Raised);
      title->setFrameShape(QFrame::Shape::WinPanel);
      //
      auto f = label->font();
      f.setBold(true);
      label->setFont(f);
      //
      auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
      title->setLayout(layout);
      layout->addWidget(label);
      layout->addWidget(toolbar);
      layout->addWidget(toggle);
      layout->setContentsMargins({ 6, 3, 6, 3 });
      layout->setStretch(0, 1);
      layout->setStretch(1, 0);
      layout->setStretch(2, 0);
      //
      toggle->setMaximumWidth(32);
   }
   {
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down);
      this->setLayout(layout);
      layout->addWidget(title);
      layout->addWidget(body);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->setSpacing(0);
      layout->setStretch(0, 0);
      layout->setStretch(1, 1);
   }
   //
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(toggle, &QPushButton::clicked, this, &DKCollapsiblePane::toggleCollapsed);
   #endif
   this->setFocusProxy(toolbar);
   QWidget::setTabOrder(toolbar, toggle);
   QWidget::setTabOrder(toggle,  body);
   //
   this->_updateToggle();
}

void DKCollapsiblePane::setCollapsed(bool state) {
   if (this->collapsed() == state)
      return;
   this->state.collapsed = state;
   if (this->subwidgets.body) {
      this->subwidgets.body->setVisible(!state);
   }
   this->_updateToggle(state);
   auto policy = this->sizePolicy();
   if (state) {
      this->toolbar.container->setVisible(this->state.show_actions_when_collapsed);
      policy.setVerticalPolicy(QSizePolicy::Policy::Maximum);
   } else {
      this->toolbar.container->setVisible(true);
      policy.setVerticalPolicy(QSizePolicy::Policy::Preferred);
   }
   this->setSizePolicy(policy);
   if (state)
      emit this->contentsCollapsed();
   else
      emit this->contentsExpanded();
}
void DKCollapsiblePane::setTitle(const QString& t) {
   this->state.title = t;
   if (auto* widget = this->subwidgets.label) {
      QMetaObject::invokeMethod(widget, "setText", Qt::AutoConnection, Q_ARG(QString, t));
   }
   this->setAccessibleName(t);
}
void DKCollapsiblePane::setTitleWidget(QWidget* w) {
   auto* header = this->subwidgets.title;
   if (w) {
      w->setParent(header);
   }
   auto* layout = (QBoxLayout*) header->layout();
   if (auto* prior = this->subwidgets.label) {
      if (w) {
         auto* item = layout->replaceWidget(prior, w, Qt::FindDirectChildrenOnly);
         assert(item && "There should've been something in the layout already!");
         delete item;
      } else {
         layout->removeWidget(prior);
      }
      if (prior->parent() == header) {
         prior->setParent(nullptr);
         delete prior;
      }
   } else {
      if (w) {
         layout->insertWidget(0, w, 1);
      }
   }
   this->subwidgets.label = w;
   if (w)
      QMetaObject::invokeMethod(w, "setText", Qt::AutoConnection, Q_ARG(QString, this->state.title));
}
QWidget* DKCollapsiblePane::takeTitleWidget() {
   auto* widget = this->subwidgets.label;
   if (widget) {
      this->subwidgets.label = nullptr;
      {
         auto* layout = (QBoxLayout*) this->subwidgets.title->layout();
         layout->removeWidget(widget);
      }
      widget->setParent(nullptr);
   }
   return widget;
}
void DKCollapsiblePane::setViewport(QWidget* w) {
   if (w) {
      w->setParent(this);
      w->setVisible(!this->state.collapsed);
   }
   auto* layout = (QBoxLayout*) this->layout();
   if (auto* prior = this->subwidgets.body) {
      if (w) {
         auto* item = layout->replaceWidget(prior, w, Qt::FindDirectChildrenOnly);
         assert(item && "There should've been something in the layout already!");
         delete item;
      } else {
         layout->removeWidget(prior);
      }
      if (prior->parent() == this) {
         prior->setParent(nullptr);
         delete prior;
      }
   } else {
      //
      // No body.
      //
      if (w) {
         layout->addWidget(w);
         layout->setStretch(1, 1);
      }
   }
   this->subwidgets.body = w;
}
QWidget* DKCollapsiblePane::takeViewport() {
   auto* widget = this->subwidgets.body;
   if (widget) {
      this->subwidgets.body = nullptr;
      {
         auto* layout = (QBoxLayout*) this->layout();
         layout->removeWidget(widget);
      }
      widget->setParent(nullptr);
   }
   return widget;
}
void DKCollapsiblePane::setShowActionsWhenCollapsed(bool s) {
   if (this->state.show_actions_when_collapsed == s)
      return;
   this->state.show_actions_when_collapsed = s;
   if (this->collapsed())
      this->toolbar.container->setVisible(s);
}

void DKCollapsiblePane::_updateToggle() {
   this->_updateToggle(this->collapsed());
}
void DKCollapsiblePane::_updateToggle(bool state) {
   icons.setup();
   //
   auto*   toggle = this->subwidgets.toggle;
   QIcon   icon;
   QString fallback;
   if (state) {
      toggle->setAccessibleName(tr("expand", "accessible name for toggle button"));
      icon     = icons.expand;
      fallback = (const char*)u8"\u2BED";
   } else {
      toggle->setAccessibleName(tr("collapse", "accessible name for toggle button"));
      icon     = icons.collapse;
      fallback = (const char*)u8"\u2BEF";
   }
   if (icon.isNull()) {
      toggle->setIcon(QIcon());
      toggle->setText(fallback);
   } else {
      toggle->setIcon(icon);
      toggle->setText(QString());
   }
}

#pragma region DKCollapsiblePane::_Toolbar
DKCollapsiblePane::_Toolbar::_Toolbar(DKCollapsiblePane& owner) {
   this->container = new QWidget(&owner);
   //
   auto* widget = this->container;
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   widget->setLayout(layout);
   widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
}

int DKCollapsiblePane::_Toolbar::indexOf(QAction* subject) const noexcept {
   int size = this->entries.size();
   for (int i = 0; i < size; ++i)
      if (this->entries[i].action == subject)
         return i;
   return -1;
}

void DKCollapsiblePane::_Toolbar::_synchronize(QPushButton* widget, QAction* action) {
   const auto blocker = QSignalBlocker(widget);
   //
   widget->setUpdatesEnabled(false);
   //
   widget->setIcon(action->icon());
   {
      auto t = action->iconText();
      if (t.isEmpty())
         t = action->text();
      widget->setText(t);
   }
   widget->setEnabled(action->isEnabled());
   //
   widget->setCheckable(action->isCheckable());
   widget->setChecked(action->isChecked());
   widget->setShortcut(action->shortcut());
   widget->setToolTip(action->toolTip());
   widget->setWhatsThis(action->whatsThis());
   //
   widget->setUpdatesEnabled(true);
   widget->updateGeometry();
}
void DKCollapsiblePane::_Toolbar::_updateTabOrder() {
   int size = this->entries.size();
   for (int i = 0; i < size - 1; ++i) {
      auto* a = this->entries[i].widget;
      auto* b = this->entries[i + 1].widget;
      QWidget::setTabOrder(a, b);
   }
   if (size) {
      this->container->setFocusProxy(this->entries[0].widget);
      this->container->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   } else {
      this->container->setFocusProxy(nullptr);
      this->container->setFocusPolicy(Qt::FocusPolicy::NoFocus);
   }
}

void DKCollapsiblePane::_Toolbar::insertAction(QAction* subject, QAction* before) {
   int i = this->entries.size();
   if (before) {
      i = this->indexOf(before);
      assert(i >= 0);
   }
   _ToolbarEntry entry;
   entry.action = subject;
   entry.widget = new QPushButton(this->container);
   {
      auto* widget = entry.widget;
      QObject::connect(widget, &QPushButton::clicked, subject, &QAction::trigger);
      widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
      this->_synchronize(widget, subject);
   }
   this->entries.insert(i, entry);
   //
   auto* layout = (QBoxLayout*) this->container->layout();
   layout->insertWidget(i, entry.widget);
   //
   this->_updateTabOrder();
}
void DKCollapsiblePane::_Toolbar::updateAction(QAction* subject) {
   int i = this->indexOf(subject);
   if (i < 0)
      return;
   auto& entry = this->entries[i];
   this->_synchronize(entry.widget, subject);
   entry.widget->update();
}
void DKCollapsiblePane::_Toolbar::removeAction(QAction* subject) {
   int i = this->indexOf(subject);
   if (i < 0)
      return;
   auto* widget = this->entries[i].widget;
   this->entries.removeAt(i);
   if (widget) {
      widget->setParent(nullptr);
      delete widget;
   }
   this->_updateTabOrder();
}
#pragma endregion

void DKCollapsiblePane::actionEvent(QActionEvent* event) {
   auto* action = event->action();
   switch (event->type()) {
      case QEvent::Type::ActionAdded:
         this->toolbar.insertAction(action, event->before());
         break;
      case QEvent::Type::ActionChanged:
         this->toolbar.updateAction(action);
         break;
      case QEvent::Type::ActionRemoved:
         this->toolbar.removeAction(action);
         break;
   }
}