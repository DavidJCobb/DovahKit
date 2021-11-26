#pragma once
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#if !defined(QT_DESIGNER_LIB)
#else
#endif

class DKBoundInputWidget : public QWidget {
   Q_OBJECT;
   public:
      DKBoundInputWidget(QWidget* parent = nullptr);

      QKeySequence value() const;

   public slots:

   signals:

   protected:
      struct {
         struct {
            QPushButton* show = nullptr;
            QComboBox*   mod  = nullptr;
         } boolean;
         struct {
            QComboBox* control = nullptr;
            QComboBox* sign    = nullptr;
         } scalar;
         struct {
            QComboBox* control = nullptr;
         } vector;
      } subwidgets;
      struct {
         QKeySequence value;
      } state;
};