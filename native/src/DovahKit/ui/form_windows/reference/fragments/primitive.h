#pragma once
#include <array>
#include <QString>
#include "dovah/data/collision_layers.h"
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class primitive;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKColorPickerButton;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QWidget;

namespace ui::reference::fragments {
   class primitive {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::primitive;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         // The "Player Activation" checkbox just changes the layer type to L_NONCOLLIDABLE. 
         // If the base form is an Activator, then this enables an activation prompt on the 
         // primitive.
         static constexpr const dovah::collision_layer layer_for_player_activate_primitives = dovah::collision_layer::non_collidable;

         struct control_collection {
            DKColorPickerButton* color = nullptr;
            union {
               std::array<QDoubleSpinBox*, 3> all = {};
               struct {
                  QDoubleSpinBox* x;
                  QDoubleSpinBox* y;
                  QDoubleSpinBox* z;
               };
            } extents;
            QLabel*    function = nullptr;
            QComboBox* layer = nullptr;
            union {
               std::array<QDoubleSpinBox*, 3> all = {};
               struct {
                  QDoubleSpinBox* x;
                  QDoubleSpinBox* y;
                  QDoubleSpinBox* z;
               };
            } origin;
            QCheckBox* player_activation = nullptr;
            union {
               std::array<QDoubleSpinBox*, 3> all = {};
               struct {
                  QDoubleSpinBox* x;
                  QDoubleSpinBox* y;
                  QDoubleSpinBox* z;
               };
            } ref_position;
            QComboBox* shape = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void issue_initial_warnings(loaded_form_type&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);

         static bool is_primitive(const loaded_form_type&);
         static bool can_change_shape(const loaded_form_type&);

      protected:
         QString _primitive_function_text(dovah::form_stub* base_form);
         void _on_collision_layer_changed();
         void _set_player_activation(bool);

         loaded_form_type& loaded();

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
         struct {
            dovah::collision_layer prior_layer = dovah::collision_layer::unidentified;
         } state;
   };
}
