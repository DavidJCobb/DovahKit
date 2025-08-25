#pragma once
#include <QDialog>
#include "ui_FormSubdialogPackageLocation.h" // generated
#include "dovah/data/packages/interrupt_override_type.h"
namespace dovah {
   class form_stub;
}
namespace ui::types::packages {
   struct package_location;
}

class FormSubdialogPackageLocation : public QDialog {
   Q_OBJECT;
   public:
      using value_type = ui::types::packages::package_location;

   public:
      FormSubdialogPackageLocation(QWidget* parent = nullptr);

      void setInterruptOverrideType(dovah::packages::interrupt_override_type);
      void setOwningPackage(dovah::form_stub*);
      void setOwningQuest(dovah::form_stub*);

      value_type value() const;
      void setValue(const value_type&);
      
   protected:
      Ui::FormSubdialogPackageLocation ui;
};