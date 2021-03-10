#pragma once
#include <QWidget>
#include "ui_PapyrusFragmentEditor.h"

namespace dovah {
   class compiled_papyrus_script;
}

class PapyrusFragmentEditor : public QWidget {
   Q_OBJECT
   //
   // TIP: Since this is derived from a plain ol' QWidget, when using it in Qt 
   // Designer, you must set the "focusProxy" property to "TabFocus" in order 
   // to move the widget within the containing window's tab order.
   //
   public:
      PapyrusFragmentEditor(QWidget* parent = nullptr);
      //
      QString currentScriptname() const noexcept;
      QString currentFunction() const noexcept;
      void setCurrentScriptname(const QString&);
      void setCurrentScriptname(const char*);
      void setCurrentFunction(const QString&);
      void setCurrentFunction(const char*);
      //
      int scriptnameMaxLength() const noexcept;
      void setScriptnameMaxLength(int) noexcept;
      int functionMaxLength() const noexcept;
      void setFunctionMaxLength(int) noexcept;
      //
      void addScriptname(const QString&);
      void clearAvailableScriptnames();
      void clearCurrentValues();
      void removeScriptname(const QString&);
      //
   signals:
      void currentScriptnameChanged(const QString&);
      void currentFunctionChanged(const QString&);
      //
   protected:
      Ui::PapyrusFragmentEditor ui;
      
      struct script {
         QString name;
         dovah::compiled_papyrus_script* compiled = nullptr;
      };
      QList<script> scripts;

      script* _getScriptData(const QString& name);
      script* _getOrCreateScriptData(const QString& name);
};