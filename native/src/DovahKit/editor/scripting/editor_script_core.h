#pragma once
#include <QString>
#include <QWidget>
#include "../../helpers/singleton.h"

//
// An interface to the DovahKit script VM. This object doesn't do anything on its 
// own; it acts as a facade through which outside systems can start and stop the 
// Lua script VM.
//
class DovahKitScriptVM : public QObject, cobb::singleton {
   Q_OBJECT
   protected:
      DovahKitScriptVM();

   public:
      static DovahKitScriptVM& get() {
         static DovahKitScriptVM instance;
         return instance;
      }
      
      bool is_aborted() const noexcept;
      bool is_running() const noexcept;
      
   signals:
      void messageLogged(const QString&);
      void scriptStarted();
      void scriptEnded(bool error);
      void userClickedLink(const QString& url, QWidget* opener);
      //
   public slots:
      void abort();
      void runScript(const QString& code, const QString& name);
      void setPaused(bool);
      void setUIParentWidget(QWidget*); // only allowed when a script is not running
};