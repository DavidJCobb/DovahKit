#pragma once
#include <array>
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}
class QGroupBox;
class QDoubleSpinBox;
class QWidget;

namespace ui::reference::fragments {
   class water_currents {
      public:
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         struct control_collection {
            QGroupBox* groupbox = nullptr;
            struct {
               union {
                  std::array<QDoubleSpinBox*, 3> all = {};
                  struct {
                     QDoubleSpinBox* x;
                     QDoubleSpinBox* y;
                     QDoubleSpinBox* z;
                  };
               } angular;
               QGroupBox* angular_groupbox = nullptr;
               union {
                  std::array<QDoubleSpinBox*, 3> all = {};
                  struct {
                     QDoubleSpinBox* x;
                     QDoubleSpinBox* y;
                     QDoubleSpinBox* z;
                  };
               } linear;
               QGroupBox* linear_groupbox = nullptr;
            } velocity;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
         bool can_have_currents(loaded_form_type&) const;

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
   };
}
