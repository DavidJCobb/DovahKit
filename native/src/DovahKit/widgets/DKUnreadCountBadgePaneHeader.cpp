#include "DKUnreadCountBadgePaneHeader.h"
#include <QBoxLayout>

DKUnreadCountBadgePaneHeader::DKUnreadCountBadgePaneHeader(QWidget* parent) : QWidget(parent) {
   this->_text  = new QLabel(this);
   this->_badge = new DKUnreadCountBadge(this);
   //
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
   this->setLayout(layout);
   layout->setSizeConstraint(QLayout::SetMinimumSize);
   layout->addWidget(this->_text);
   layout->addWidget(this->_badge);
   layout->setStretch(0, 0);
   layout->setStretch(1, 0);
   layout->addStretch(1);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   {
      auto f = this->_text->font();
      f.setBold(true);
      this->_text->setFont(f);
   }
}