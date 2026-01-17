#pragma once
#include <QMenu>
class RegionsDialog;
class QCheckBox;
class QLineEdit;
class QSpinBox;

namespace ui::region::fragments {
   class map {
      public:
         map(RegionsDialog& o);

         struct controls {
            struct {
               QCheckBox* enable   = nullptr;
               QCheckBox* override = nullptr;
               QSpinBox*  priority = nullptr;
            } header;
            QLineEdit* name = nullptr;
         };

      public:
         RegionsDialog& owner;
      protected:
         controls ui;

      public:
         void set_controls(controls&&);
         void reload();
         void commit();

         void on_header_edited();
         void on_data_edited();
   };
}