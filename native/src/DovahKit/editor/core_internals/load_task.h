#pragma once
#include "../../helpers/performance.h"
#include "../core.h"

namespace DovahKitEditorInternals {
   class load_task : public QObject {
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
         void complete(DovahKitCore::file_load_stats); // this must be fully qualified, or Qt's preprocessor will make a mistake and attempts to pass this across threads at run-time will fail
         void failed();
         void ended(); // called after (complete) and (failed)
      public slots:
         void exec();
   };
}