#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class ownership;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKFormPicker;
class QComboBox;
class QWidget;

namespace ui::reference::fragments {
   class ownership {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::ownership;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;

         struct control_collection {
            DKFormPicker* owner_form = nullptr;
            QComboBox*    owner_rank = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
      protected:
         void _update_ownership_rank_picker();

         loaded_form_type& loaded();

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;
   };
}
