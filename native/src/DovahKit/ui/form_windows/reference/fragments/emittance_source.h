#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class emittance_source;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKFormPicker;
class QRadioButton;
class QWidget;

namespace ui::reference::fragments {
   class emittance_source {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::emittance_source;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         enum class emittance_type {
            none,
            internal,
            external,
         };

         struct control_collection {
            struct {
               QRadioButton* radio = nullptr;
            } none;
            struct {
               QRadioButton* radio = nullptr;
               DKFormPicker* form = nullptr;
            } light;
            struct {
               QRadioButton* radio = nullptr;
               DKFormPicker* form  = nullptr;
            } region;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
      protected:
         void set_type(emittance_type);

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
   };
}
