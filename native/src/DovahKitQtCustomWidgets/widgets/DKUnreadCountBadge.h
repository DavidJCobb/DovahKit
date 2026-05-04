#pragma once
#include <QLabel>

class DKUnreadCountBadge : public QWidget {
   Q_OBJECT;
   public:
      DKUnreadCountBadge(QWidget* parent = nullptr);

      QColor backgroundColor() const noexcept;
      QColor textColor() const noexcept;

      inline int count() const noexcept { return this->_count; }
      inline QString format() const noexcept { return this->_format; }
      inline bool showIfZero() const noexcept { return this->_showIfZero; }

      virtual QSize minimumSizeHint() const noexcept;
      virtual QSize sizeHint() const noexcept;

   protected:
      void _updateFont();
      void _updateText();

   public slots:
      void setBackgroundColor(const QColor&);
      void setTextColor(const QColor&);

      void setCount(int);
      void setFormat(const QString& t);
      void setShowIfZero(bool);

   protected:
      QLabel* _text = nullptr;
      QString _format;
      int     _count = 0;
      bool    _showIfZero = false;

      virtual void changeEvent(QEvent*) override;
      virtual void paintEvent(QPaintEvent*) override;
};