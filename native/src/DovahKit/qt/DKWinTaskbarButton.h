#pragma once
#include <QIcon>
#include <QObject>
#include <QString>
class QWindow;

class DKWinTaskbarProgress;

class DKWinTaskbarButtonPrivate;
class DKWinTaskbarButton : public QObject {
   Q_OBJECT;
   public:
      DKWinTaskbarButton(QObject* parent = nullptr);
      ~DKWinTaskbarButton();

   public:
      // Alt text for the currently-displayed overlay icon.
      QString overlayAccessibleDescription() const;

      // The current overlay icon. This is a smaller icon displayed overtop the main 
      // program icon, e.g. for things like "online status" or an "unread message 
      // count" badge.
      QIcon overlayIcon() const;

      DKWinTaskbarProgress* progress() const;

      void setWindow(QWindow*);
      QWindow* window() const;

   public: // impl
      // Listen for `TaskbarButtonCreated` window message; perform initial update.
      virtual bool eventFilter(QObject*, QEvent*) override;

   public slots:
      // Clear the overlay icon and alt text.
      void clearOverlayIcon();

      void setOverlayAccessibleDescription(QString);
      void setOverlayIcon(QIcon);

      #pragma region Extensions beyond the original API
         // Set the overlay icon and alt text together, avoiding redundant internal updates.
         void setOverlay(QIcon, QString accessible_desc);
      #pragma endregion

   protected:
      DKWinTaskbarButtonPrivate* _private = nullptr;
};