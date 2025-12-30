#pragma once
#include <utility> // std::pair
#include "helpers/vector3.h"
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class teleport;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKFormPicker;
class DKObjectReferencePicker;
class QGroupBox;
class QPushButton;
class QWidget;

namespace ui::reference::fragments {
   class teleport {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::teleport;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         struct control_collection {
            struct {
               QPushButton* view_marker = nullptr;
               QPushButton* view_ref    = nullptr;
            } buttons;
            QGroupBox*    groupbox = nullptr;
            DKFormPicker* name     = nullptr;
            DKObjectReferencePicker* ref = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void issue_initial_warnings(loaded_form_type&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
      protected:
         bool _is_random_teleport_door(loaded_form_type&);
         void _view_linked_door();
         void _view_linked_marker();
         void _on_ref_picked(dovah::form_stub*);
         bool _validate_picked_ref(dovah::form_stub*) const;

         bool _is_legal_teleport_destination(dovah::form_stub&) const;
         static dovah::form_stub* _destination_of(const loaded_form_type&);
         void _disconnect_old_destination(dovah::form_stub&);
         void _auto_place_teleport_marker(loaded_form_type& src_form, extra_data_type& src_extra, dovah::form_stub& destination);
         static std::pair<cobb::vector3<float>, cobb::vector3<float>> _calc_teleport_marker_position(const loaded_form_type& in_front_of);

         loaded_form_type& loaded();

      protected:
         control_collection controls;
         dovah::form_stub*  stub  = nullptr;
         QWidget*           owner = nullptr;
         struct {
            bool ever_changed   = false;
            bool is_random_door = false;
         } state;
   };
}
