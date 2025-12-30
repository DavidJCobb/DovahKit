#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class lock;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKFormPicker;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QWidget;

namespace ui::reference::fragments {
   class lock {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::lock;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         struct control_collection {
            QGroupBox*    groupbox   = nullptr;
            QCheckBox*    is_leveled = nullptr;
            DKFormPicker* key        = nullptr;
            QComboBox*    level      = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);

         bool can_be_locked(const loaded_form_type&);

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
   };
}
