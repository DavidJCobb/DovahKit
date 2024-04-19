#pragma once
#include "../core.h"

namespace dovah::notices {
   class base_error;
   class base_warning;
}

namespace DovahKitEditorInternals {
   struct multithreadable_detailed_notice {
      dovah::detailed_notice warning;
      //
      // Data types registered with Qt as "metatypes" must define a default constructor, a 
      // copy constructor, and a destructor, unless they are POD structs:
      //
      multithreadable_detailed_notice() = default;
      multithreadable_detailed_notice(const multithreadable_detailed_notice&) = default;
      ~multithreadable_detailed_notice() = default;
   };

   class detailed_notice_dispatcher : public QObject {
      Q_OBJECT
      public:
         using file_load_stats = DovahKitCore::file_load_stats;
      public:
         static detailed_notice_dispatcher& get() {
            static detailed_notice_dispatcher instance;
            return instance;
         }
         detailed_notice_dispatcher();
         
      signals:
         void received(multithreadable_detailed_notice);
         void receivedError(dovah::notices::base_error*); // recipient must delete received pointer
         void receivedWarning(dovah::notices::base_warning*); // recipient must delete received pointer
         
      public slots:
         void send(const dovah::detailed_notice&);
         void send(const dovah::notices::base_error&);
         void send(const dovah::notices::base_warning&);
   };
}

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(DovahKitEditorInternals::multithreadable_detailed_notice)
// needed so that QObject::connect can pass these across threads (by copying them). refer to detailed_notice_dispatcher's constructor as well.