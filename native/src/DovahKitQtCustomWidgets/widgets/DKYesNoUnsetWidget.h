#pragma once
#include <QCheckBox>
#include <QPushButton>

class DKYesNoUnsetWidget : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(QString        text       READ text       WRITE setText       DESIGNABLE true USER true);
   Q_PROPERTY(Qt::CheckState checkState READ checkState WRITE setCheckState DESIGNABLE true USER true);
   public:
      DKYesNoUnsetWidget(QWidget* parent = nullptr);

      Qt::CheckState checkState() const;
      QString text() const;

   public slots:
      void setCheckState(Qt::CheckState);
      void setText(QString);

   public:
      #pragma region Overrides
         virtual bool event(QEvent*) override;
      #pragma endregion

   signals:
      void stateChanged(Qt::CheckState);

   protected:
      struct {
         QPushButton* pushbutton = nullptr;
         QCheckBox*   checkbox = nullptr;
      } _subwidgets;
};