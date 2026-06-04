#include "./DKWinTaskbarButtonPrivate.h"
#include <windows.h>
#include <shobjidl.h>
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#include <QGuiApplication>

#pragma region NativeEventFilter
   DKWinTaskbarButtonPrivate::NativeEventFilter::NativeEventFilter() {
      this->message_id = RegisterWindowMessageW(L"TaskbarButtonCreated");
      this->q_event_id = QEvent::registerEventType();
   }
   DKWinTaskbarButtonPrivate::NativeEventFilter::~NativeEventFilter() {
   }

   /*static*/ DKWinTaskbarButtonPrivate::NativeEventFilter& DKWinTaskbarButtonPrivate::NativeEventFilter::get() {
      static NativeEventFilter instance;
      return instance;
   }

   /*virtual*/ bool DKWinTaskbarButtonPrivate::NativeEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) /*override*/ {
      const MSG* msg = (MSG*)message;
      if (msg->message != this->message_id)
         return false;
      bool filter_out = false;

      QWindow* target = nullptr;
      for (QWindow* candidate : QGuiApplication::topLevelWindows()) {
         if (candidate->handle() && (HWND)candidate->winId() == msg->hwnd) {
            target = candidate;
            break;
         }
      }
      if (target) {
         QEvent* event = new QEvent((QEvent::Type)this->q_event_id);
         QCoreApplication::sendEvent(target, event);
         delete event;
      }
      *result = 0;
      return true;
   }
#pragma endregion

DKWinTaskbarButtonPrivate::DKWinTaskbarButtonPrivate() {
   NativeEventFilter::get();

   this->progress = new DKWinTaskbarProgress;
   {
      DKWinTaskbarProgress* p = this->progress.get();
      auto f = [this]() { this->update_progress(); };
      QObject::connect(p, &DKWinTaskbarProgress::minimumChanged, f);
      QObject::connect(p, &DKWinTaskbarProgress::maximumChanged, f);
      QObject::connect(p, &DKWinTaskbarProgress::pausedChanged, f);
      QObject::connect(p, &DKWinTaskbarProgress::stoppedChanged, f);
      QObject::connect(p, &DKWinTaskbarProgress::valueChanged, f);
      QObject::connect(p, &DKWinTaskbarProgress::visibilityChanged, f);
   }

   // Qt itself will have already set up COM, so all we need to do is create the taskbar-list instance.
   HRESULT result = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList4, (void**)&this->taskbar_list);
   if (FAILED(result)) {
      this->taskbar_list = nullptr;
      #if _DEBUG
         __debugbreak();
      #endif
   } else {
      if (FAILED(result = this->taskbar_list->HrInit())) {
         this->taskbar_list->Release();
         this->taskbar_list = nullptr;
         #if _DEBUG
            __debugbreak();
         #endif
      }
   }
}
DKWinTaskbarButtonPrivate::~DKWinTaskbarButtonPrivate() {
   if (this->taskbar_list) {
      this->taskbar_list->Release();
      this->taskbar_list = nullptr;
   }
   if (this->progress) {
      if (!this->progress->parent())
         delete this->progress;
      this->progress = nullptr;
   }
}

/*static*/ QEvent::Type DKWinTaskbarButtonPrivate::event_type() {
   return NativeEventFilter::get().event_type();
}

HWND DKWinTaskbarButtonPrivate::handle() {
   return (HWND)this->window->winId();
}

void DKWinTaskbarButtonPrivate::update_overlay() {
   if (!this->taskbar_list)
      return;
   if (!this->window)
      return;

   wchar_t* description = nullptr;
   HICON    icon        = nullptr;
   if (QString& str = this->overlay.accessible_description; !str.isEmpty()) {
      description = new wchar_t[str.length() + 1];
      description[str.toWCharArray(description)] = 0;
   }
   if (!this->overlay.icon.isNull()) {
      icon = this->overlay.icon.pixmap(GetSystemMetrics(SM_CXSMICON)).toImage().toHICON();
      if (!icon) {
         icon = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_SHARED));
      }
   }

   // This API is annotated improperly; it permits a null icon, but VS will warn 
   // if you pass one.
   #pragma warning(suppress:6387)
   this->taskbar_list->SetOverlayIcon(this->handle(), icon, description ? description : L"");

   if (icon)
      DestroyIcon(icon);
   if (description)
      delete[] description;
}
void DKWinTaskbarButtonPrivate::update_progress() {
   if (!this->taskbar_list)
      return;
   if (!this->window)
      return;
   
   auto    handle      = this->handle();
   TBPFLAG state_flags = {};
   if (this->progress) {
      auto min   = this->progress->minimum();
      auto max   = this->progress->maximum();
      auto range = max - min;
      if (range > 0) {
         ULONGLONG value = qRound(100.0 * (double)(this->progress->value() - min)) / (double)range;
         this->taskbar_list->SetProgressValue(handle, (ULONGLONG)value, 100);
      }
      if (!this->progress->isVisible()) {
         state_flags = TBPF_NOPROGRESS;
      } else {
         if (min == 0 && max == 0) {
            state_flags = TBPF_INDETERMINATE;
         } else if (this->progress->isStopped()) {
            state_flags = TBPF_ERROR;
         } else if (this->progress->isPaused()) {
            state_flags = TBPF_PAUSED;
         } else {
            state_flags = TBPF_NORMAL;
         }
      }
   } else {
      state_flags = TBPF_NOPROGRESS;
   }
   this->taskbar_list->SetProgressState(handle, state_flags);
}