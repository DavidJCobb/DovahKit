#pragma once
#include <QWidget>
#include "ui_TESFilePicker.h"

class TESFilePicker : public QWidget {
   Q_OBJECT
   //
   // TIP: Since this is derived from a plain ol' QWidget, when using it in Qt 
   // Designer, you must set the "focusProxy" property to "TabFocus" in order 
   // to move the widget within the containing window's tab order.
   //
   public:
      TESFilePicker(QWidget* parent = nullptr);
      QString currentPath() const noexcept; // e.g. "data/meshes/foo.nif"
      void setCurrentPath(const QString&);
      void setCurrentPath(const char*);
      //
   signals:
      void currentPathChanged();
      void currentPathEdited();
      //
   protected:
      Ui::TESFilePicker ui;
};