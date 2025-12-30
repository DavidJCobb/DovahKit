#pragma once
#include <cstdint>
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class light;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKFloatSlider;
class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QPushButton;
class QWidget;

namespace ui::reference::fragments {
   class light {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::light;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         struct control_collection {
            struct {
               QPushButton*    reset   = nullptr;
               DKFloatSlider*  slider  = nullptr;
               QDoubleSpinBox* spinbox = nullptr;
            } depth_bias;
            struct {
               QCheckBox*      same_as_radius = nullptr;
               QDoubleSpinBox* spinbox        = nullptr;
            } end_cap;
            struct {
               QPushButton*    reset   = nullptr;
               QDoubleSpinBox* spinbox = nullptr;
            } fade;
            struct {
               QCheckBox* can_cast_shadows     = nullptr;
               QCheckBox* does_not_light_land  = nullptr;
               QCheckBox* does_not_light_water = nullptr;
               QCheckBox* never_fades          = nullptr;
            } flags;
            struct {
               QPushButton*    reset   = nullptr;
               QDoubleSpinBox* spinbox = nullptr;
            } fov;
            QGroupBox* groupbox = nullptr;
            struct {
               QPushButton*    reset   = nullptr;
               QDoubleSpinBox* spinbox = nullptr;
            } radius;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&, uint32_t& record_flags);
         void save(loaded_form_type&, uint32_t& record_flags);

         loaded_form_type& loaded();

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
   };
}
