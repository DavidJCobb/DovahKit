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
      void scriptEnded(bool error);
      void userClickedLink(const QString& url, QWidget* opener);
      
   public slots:
      void abort();
      void runScript(const QString& code, const QString& name);
      void runScripts(dovahscript::script_set&&); // use std::move for the argument
      void setPaused(bool);
      void setUIParentWidget(QWidget*); // only allowed when a script is not running
};