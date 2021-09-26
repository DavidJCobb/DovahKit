#include "DKCollapsiblePane.h"
#include <QActionEvent>
#include <QBoxLayout>

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
   QObject::connect(toggle, &QPushButton::clicked, this, &DKCollapsiblePane::toggleCollapsed);
   this->setFocusProxy(toolbar);
   QWidget::setTabOrder(label,   toolbar);
   QWidget::setTabOrder(toolbar, toggle);
   QWidget::setTabOrder(toggle,  body);
   //
   this->_updateToggle();
}

void DKCollapsiblePane::setCollapsed(bool state) {
   if (this->collapsed() == state)
      return;
   this->subwidgets.body->setVisible(!state);
   this->_updateToggle(state);
   if (state)
      emit this->contentsCollapsed();
   else
      emit this->contentsExpanded();
}
void DKCollapsiblePane::setTitle(const QString& t) {
   this->subwidgets.label->setText(t);
   this->setAccessibleName(t);
}

void DKCollapsiblePane::_updateToggle() {
   this->_updateToggle(this->collapsed());
}
void DKCollapsiblePane::_updateToggle(bool state) {
   auto* toggle = this->subwidgets.toggle;
   if (state) {
      toggle->setAccessibleName(tr("expand", "accessible name for toggle button"));
      toggle->setText((const char*)u8"\u2BEF");
   } else {
      toggle->setAccessibleName(tr("collapse", "accessible name for toggle button"));
      toggle->setText((const char*)u8"\u2BED");
   }
}

#pragma region DKCollapsiblePane::_Toolbar
DKCollapsiblePane::_Toolbar::_Toolbar(DKCollapsiblePane& owner) {
   this->container = new QWidget(&owner);
   //
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   this->container->setLayout(layout);
}

int DKCollapsiblePane::_Toolbar::indexOf(QAction* subject) const noexcept {
   int size = this->entries.size();
   for (int i = 0; i < size; ++i)
      if (this->entries[i].action == subject)
         return i;
   return -1;
}

void DKCollapsiblePane::_Toolbar::insertAction(QAction* subject, QAction* before) {
   int i = this->entries.size();
   if (before) {
      i = this->indexOf(before);
      assert(i >= 0);
   }
   _ToolbarEntry entry;
   entry.action = subject;
   entry.widget = new QToolButton(this->container);
   entry.widget->setDefaultAction(subject);
   this->entries.insert(i, entry);
   //
   auto* layout = (QBoxLayout*) this->container->layout();
   layout->addWidget(entry.widget);
   //
   if (i == 0)
      this->container->setFocusProxy(entry.widget);
}
void DKCollapsiblePane::_Toolbar::updateAction(QAction* subject) {
   int i = this->indexOf(subject);
   if (i < 0)
      return;
   auto& entry = this->entries[i];
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
   if (i == 0) {
      QWidget* next = nullptr;
      if (!this->entries.isEmpty())
         next = this->entries[0].widget;
      this->container->setFocusProxy(next);
   }
}
#pragma endregion

void DKCollapsiblePane::actionEvent(QActionEvent* event) {
   auto* action = event->action();
   switch (event->type()) {
      case QEvent::Type::ActionAdded:
         if (auto* before = event->before()) {
         }
         break;
      case QEvent::Type::ActionChanged:
         break;
      case QEvent::Type::ActionRemoved:
         break;
   }
}