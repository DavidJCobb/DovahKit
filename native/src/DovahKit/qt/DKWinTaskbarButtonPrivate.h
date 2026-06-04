#pragma once
#include <QAbstractNativeEventFilter>
#include <QEvent>
#include <QIcon>
#include <QPointer>
#include <QString>
#include <QWindow>
#include "./DKWinTaskbarProgress.h"
struct ITaskbarList4;
using HWND = struct HWND__*;

struct DKWinTaskbarButtonPrivate {
   public:
      // Routes Win32 events pertaining to taskbar buttons into Qt's event system.
      class NativeEventFilter : public QAbstractNativeEventFilter {
         protected:
            NativeEventFilter();
            ~NativeEventFilter();

            int message_id = 0;
            int q_event_id = 0;

         public:
            static NativeEventFilter& get();
            constexpr QEvent::Type event_type() const noexcept { return (QEvent::Type)this->q_event_id; }

         public:
            virtual bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
      };

   public:
      DKWinTaskbarButtonPrivate();
      ~DKWinTaskbarButtonPrivate();

   public:
      struct {
         QString accessible_description;
         QIcon   icon;
      } overlay;
      QPointer<DKWinTaskbarProgress> progress;
      ITaskbarList4* taskbar_list = nullptr; // COM
      QPointer<QWindow> window;

   public:
      static QEvent::Type event_type();

      HWND handle();

      void update_overlay();
      void update_progress();
};