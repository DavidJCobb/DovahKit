#pragma once
#include "../../helpers/performance.h"
#include "../core.h"

namespace DovahKitEditorInternals {
   class load_task : public QObject {
      //
      // Helper class. This can be used as a worker for a QThread, allowing for 
      // asynchronous loading; it can also just be executed synchronously.
      //
      Q_OBJECT
      public:
         using file_load_stats = DovahKitCore::file_load_stats;
      public:
         load_task(DovahKitCore& ed) : editor(ed) {}
         //
         cobb::benchmark benchmark;
         file_load_stats stats;
         DovahKitCore&   editor;
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