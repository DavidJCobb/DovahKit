#pragma once
#include <QWidget>
#include <QLineEdit>
#if !defined(QT_DESIGNER_LIB)
   #include "helpers/qt/keycodes.h"
#else
   namespace cobb::qt {
      struct key {
         Qt::Key code;
         QString glyph;
      };
   }
#endif

class DKKeyPickerWidget : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool allowKeyCombinations READ allowKeyCombinations WRITE setAllowKeyCombinations DESIGNABLE true);
   public:
      DKKeyPickerWidget(QWidget* parent = nullptr);

      QVector<cobb::qt::key> keys() const;
      QString toString() const;

      inline bool allowKeyCombinations() const { return this->props.allowKeyCombinations; }

   public slots:
      void setAllowKeyCombinations(bool);
      void setKeys(const QVector<cobb::qt::key>&);

   signals:
      void valueChanged();

   protected:
      struct _key : cobb::qt::key {
         bool still_down = false;
         //
         _key() {}
         _key(const cobb::qt::key& k, bool sd = false) : key(k), still_down(sd) {}
         //
         bool operator==(const cobb::qt::key& other) const noexcept { return *(const cobb::qt::key*)this == other; }
      };

      struct {
         QLineEdit* line = nullptr;
      } subwidgets;
      struct {
         QVector<_key> keys;
      } state;
      struct {
         bool allowKeyCombinations = true;
      } props;

      void _redraw();

      #if !defined(QT_DESIGNER_LIB)
      void _keyDown(QKeyEvent*);
      void _keyUp(QKeyEvent*);
      #endif
      virtual bool eventFilter(QObject* target, QEvent* event) override;
};