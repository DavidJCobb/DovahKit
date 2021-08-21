#pragma once
#include <QObject>
#include "../helpers/singleton.h"
#include "script_set.h"

// The singleton through which code outside of the script engine asks to run a 
// script and monitors execution status.
class DovahscriptHost : public QObject, cobb::singleton {
   Q_OBJECT;
   protected:
      DovahscriptHost();

   public:
      static DovahscriptHost& get() {
         static DovahscriptHost instance;
         return instance;
      }

      bool is_aborted() const noexcept;
      bool is_running() const noexcept;
      
   signals:
      void messageLogged(const QString&);
      void scriptStarted();
      void scriptEnded();
      void userClickedLink(const QString& url, QWidget* opener);

      void evalComplete();
      
   public slots:
      void abort();
      void runScript(const QString& code, const QString& filename = "userscript");
      void runScripts(const dovahscript::script_set&); // use std::move for the argument
      void setPaused(bool);
      void setUIParentWidget(QWidget*); // only allowed when a script is not running

      // Once a script session is up, you can use this to run additional code. Think of it 
      // like a debug console for the user; if a script with UI throws an error or does 
      // something unexpected, the user can use this (perhaps through some secondary text 
      // box) to run Lua commands and inspect the script state.
      //
      // Eval scripts are queued to run, not run instantly. Lua is single-threaded, so if 
      // other script functions are running, the eval script must wait on them to finish. 
      // This also means that you can't use this to debug infinite loops and similar.
      bool evalScript(const QString& code);
};

namespace dovahscript {
   using host = DovahscriptHost;
}