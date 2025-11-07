#pragma once
#include <QDialog>
#include "ui_FormSubdialogIdleNewActionRoot.h" // generated
namespace dovah {
   class form_stub;
}
class IdleNewActionRootPickerFilter;

class FormSubdialogIdleNewActionRoot : public QDialog {
   Q_OBJECT;
   public:
      FormSubdialogIdleNewActionRoot(QWidget* parent = nullptr);

      // The user won't be allowed to create a new root idle for these actions, because 
      // they already have one.
      void setExistingActionRoots(const std::vector<dovah::form_stub*>&);
      void setExistingActionRoots(std::vector<dovah::form_stub*>&&);

      dovah::form_stub* action() const;
      QString idleEditorID() const;
      
   protected:
      Ui::FormSubdialogIdleNewActionRoot ui;
      IdleNewActionRootPickerFilter* _action_filter = nullptr;
};