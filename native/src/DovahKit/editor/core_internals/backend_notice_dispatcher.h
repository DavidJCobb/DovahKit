#pragma once
#include "../core.h"

namespace dovah::notices {
   class base_error;
   class base_warning;
}

namespace DovahKitEditorInternals {
   class backend_notice_dispatcher : public QObject {
      Q_OBJECT
      public:
         using file_load_stats = DovahKitCore::file_load_stats;
      public:
         static backend_notice_dispatcher& get() {
            static backend_notice_dispatcher instance;
            return instance;
         }
         
      signals:
         void receivedError(dovah::notices::base_error*); // recipient must delete received pointer
         void receivedWarning(dovah::notices::base_warning*); // recipient must delete received pointer
         
      public slots:
         void send(const dovah::notices::base_error&);
         void send(const dovah::notices::base_warning&);
   };
}