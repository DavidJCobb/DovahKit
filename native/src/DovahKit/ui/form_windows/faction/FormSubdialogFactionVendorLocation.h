#pragma once
#include <QDialog>
#include "ui_FormSubdialogFactionVendorLocation.h" // generated
#include "dovah/forms/Faction.h"

class FormSubdialogFactionVendorLocation : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::Faction;
      using location_type    = decltype(loaded_form_type::package_location_vendor);

   public:
      FormSubdialogFactionVendorLocation(QWidget* parent = nullptr);

      void importFrom(const location_type&);
      void commitTo(loaded_form_type& dst_owner, location_type& dst) const;
      
   protected:
      Ui::FormSubdialogFactionVendorLocation ui;
};