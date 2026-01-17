#pragma once
#include <QMenu>
class RegionsDialog;
class DKGameFilePicker;
class QCheckBox;
class QSpinBox;

namespace ui::region::fragments {
   class landscape {
      public:
         landscape(RegionsDialog& o);

         struct controls {
            struct {
               QCheckBox* enable   = nullptr;
               QCheckBox* override = nullptr;
               QSpinBox*  priority = nullptr;
            } header;
            DKGameFilePicker* texture = nullptr;
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