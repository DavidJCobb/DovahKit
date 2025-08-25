#pragma once
#include <QDialog>
#include "ui_FormSubdialogPackageTarget.h" // generated
#include "dovah/data/packages/interrupt_override_type.h"
namespace dovah {
   class form_stub;
}
namespace ui::types::packages {
   struct package_target;
}

class FormSubdialogPackageTarget : public QDialog {
   Q_OBJECT;
   public:
      using value_type = ui::types::packages::package_target;

   public:
      FormSubdialogPackageTarget(QWidget* parent = nullptr);

      void setInterruptOverrideType(dovah::packages::interrupt_override_type);
      void setOwningQuest(dovah::form_stub*);

      value_type value() const;
      void setValue(const value_type&);
      
   protected:
      Ui::FormSubdialogPackageTarget ui;
};