#pragma once
#include "_base.h"
#include <QLabel>

namespace DovahKitDebug {
   class TestBadgeWidget : public QWidget {
      Q_OBJECT;
      public:
         TestBadgeWidget(QWidget* parent = nullptr);

         virtual QSize minimumSizeHint() const noexcept;
         virtual QSize sizeHint() const noexcept;

      public slots:
         void setText(const QString& t);

      protected:
         QLabel* _text = nullptr;

         virtual void paintEvent(QPaintEvent*) override;
   };

   class TitleWithBadge : public QWidget {
      Q_OBJECT;
      public:
         TitleWithBadge(QWidget* parent = nullptr);

      public slots:
         void setBadgeText(const QString& t);
         void setText(const QString& t);

      protected:
         QLabel* _text = nullptr;
         TestBadgeWidget* _badge = nullptr;
   };
}

namespace DovahKitDebug {
   namespace features {
      struct ui_collapsible_pane : debug_feature {
         static constexpr const char* name = "DKCollapsiblePane test";
         static void execute(QWidget* from);
      };
   }
}
