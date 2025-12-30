#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class map_marker;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKFormPicker;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLineEdit;
class QWidget;

namespace ui::reference::fragments {
   class map_marker {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::map_marker;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         struct control_collection {
            struct {
               QCheckBox* can_be_fast_traveled_to   = nullptr;
               QCheckBox* is_unaffected_by_show_all = nullptr;
               QCheckBox* is_visible                = nullptr;
            } flags;
            QGroupBox*      groupbox = nullptr;
            QComboBox*      icon     = nullptr;
            QLineEdit*      name     = nullptr;
            QDoubleSpinBox* radius   = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
         bool is_map_marker(const loaded_form_type&);

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
   };
}
