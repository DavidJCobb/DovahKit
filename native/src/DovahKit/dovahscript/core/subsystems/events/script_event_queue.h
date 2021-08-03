#pragma once
#include <mutex>
#include <vector>
#include <QObject>

namespace dovahscript::impl {
   class script_event;

   struct script_event_queue {
      protected:
         std::vector<script_event*> list;
         mutable std::recursive_mutex lock;
      public:
         void   clear();                  // script thread should call this when doing cleanup
         size_t process();                // script thread should call this to process pending events; returns number of events processed
         void   push_back(script_event*); // main thread should call this to send events to lua

         void forget_about(QObject&);

         [[nodiscard]] size_t size() const noexcept;

         inline bool empty() const noexcept {
            return this->size() == 0;
         }
   };
}