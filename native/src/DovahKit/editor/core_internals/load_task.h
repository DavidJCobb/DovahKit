#pragma once
#include "../../helpers/performance.h"
#include "../core.h"
#include "../../dovah/files/tes_file_reading/results.h"

namespace DovahKitEditorInternals {
   struct multithreadable_load_results {
      dovah::tes_file_reading::read_results data;
      //
      // Data types registered with Qt as "metatypes" must define a default constructor, a 
      // copy constructor, and a destructor, unless they are POD structs:
      //
      multithreadable_load_results() = default;
      multithreadable_load_results(const multithreadable_load_results&) = default;
      ~multithreadable_load_results() = default;
   };

   class load_task : public QObject {
      //
      // Helper class. This can be used as a worker for a QThread, allowing for 
      // asynchronous loading; it can also just be executed synchronously.
      //
      Q_OBJECT
      public:
         using file_load_stats = DovahKitCore::file_load_stats;
      public:
         load_task(DovahKitCore& ed);
         //
         cobb::benchmark benchmark;
         file_load_stats stats;
         DovahKitCore&   editor;
         dovah::tes_file_reading::read_results results;
         bool result = false;
         //
      signals:
         void complete(DovahKitCore::file_load_stats); // this must be fully qualified, or Qt's preprocessor will make a mistake and attempts to pass this across threads at run-time will fail. <https://blog.debiania.in.ua/posts/2019-06-10-nested-structs-as-signal-arguments-in-qt.html>
         void failed();
         void ended(); // called after (complete) and (failed)
      public slots:
         void exec();
   };
}

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(DovahKitEditorInternals::multithreadable_load_results)
// needed so that QObject::connect can pass these across threads (by copying them). refer to load_task's constructor as well.