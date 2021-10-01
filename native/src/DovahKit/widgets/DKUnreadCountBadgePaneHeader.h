#pragma once
#include <QLabel>
#include "DKUnreadCountBadge.h"

class DKUnreadCountBadgePaneHeader : public QWidget {
   Q_OBJECT;
   public:
      DKUnreadCountBadgePaneHeader(QWidget* parent = nullptr);

      inline DKUnreadCountBadge* badge() const noexcept { return this->_badge; }

      inline int badgeCount() const noexcept { return this->_badge->count(); }
      inline QString badgeFormat() const noexcept { return this->_badge->format(); }
      inline QString text() const noexcept { return this->_text->text(); }

   public slots:
      inline void setBadgeCount(int v) { this->_badge->setCount(v); }
      inline void setBadgeFormat(const QString& t) { this->_badge->setFormat(t); }
      inline void setText(const QString& t) { this->_text->setText(t); }

   protected:
      QLabel* _text = nullptr;
      DKUnreadCountBadge* _badge = nullptr;
};