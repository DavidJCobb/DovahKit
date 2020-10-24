#pragma once
#include "../core.h"

namespace DovahKitEditorInternals {
   struct multithreadable_file_read_warning {
      dovah::file_read_warning warning;
      //
      // Data types registered with Qt as "metatypes" must define a default constructor, a 
      // copy constructor, and a destructor, unless they are POD structs:
      //
      multithreadable_file_read_warning() = default;
      multithreadable_file_read_warning(const multithreadable_file_read_warning&) = default;
      ~multithreadable_file_read_warning() = default;
   };

   class read_warning_dispatcher : public QObject {
      Q_OBJECT
      public:
         using file_load_stats = DovahKitCore::file_load_stats;
      public:
         static read_warning_dispatcher& get() {
            static read_warning_dispatcher instance;
            return instance;
         }
         read_warning_dispatcher();
         //
      signals:
         void received(multithreadable_file_read_warning);
         //
      public slots:
         void send(const dovah::file_read_warning&);
   };
}

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(DovahKitEditorInternals::multithreadable_file_read_warning)
// needed so that QObject::connect can pass these across threads (by copying them). refer to read_warning_dispatcher's constructor as well.