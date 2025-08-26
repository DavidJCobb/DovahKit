#pragma once
#include <QCheckBox>
#include <QPushButton>
#if defined(QT_PLUGIN)
   #include "tools/in_place_text_editor/InPlaceTextEditableWidget.h"
#endif

class QStyleOptionButton;

class DKYesNoUnsetWidget : public QWidget
   #if defined(QT_PLUGIN)
      ,
      public InPlaceTextEditableWidget
   #endif
{
   Q_OBJECT;
   Q_PROPERTY(QString        text       READ text       WRITE setText       DESIGNABLE true USER true);
   Q_PROPERTY(Qt::CheckState checkState READ checkState WRITE setCheckState DESIGNABLE true);
   public:
      DKYesNoUnsetWidget(QWidget* parent = nullptr);

      Qt::CheckState checkState() const;
      QString text() const;

   public slots:
      void setCheckState(Qt::CheckState);
      void setText(QString);

   public:
      #pragma region Overrides
         #if defined(QT_PLUGIN)
            virtual QRect inPlaceTextEditingBounds() const override; // InPlaceTextEditableWidget
         #endif

         virtual QSize sizeHint() const override;
         virtual QSize minimumSizeHint() const override;

         virtual bool event(QEvent*) override;
         virtual bool eventFilter(QObject* watched, QEvent*) override;
      #pragma endregion

   signals:
      void stateChanged(Qt::CheckState);

   protected:
      struct {
         QPushButton* pushbutton = nullptr;
         QCheckBox*   checkbox = nullptr;
      } _subwidgets;

      void _initCheckboxStyleOption(QStyleOptionButton&) const;
};