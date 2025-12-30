#pragma once
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}
class ObjectReferenceReflectedObjectsModel;
class QGroupBox;
class QTableView;
class QWidget;

namespace ui::reference::fragments {
   class reflected_refs {
      public:
         using loaded_form_type = dovah::loaded_forms::ObjectReference;
         using model_type       = ObjectReferenceReflectedObjectsModel;

         struct control_collection {
            QGroupBox*  groupbox = nullptr;
            QTableView* view     = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
         static bool is_placed_water(loaded_form_type&);

      protected:
         control_collection controls;
         dovah::form_stub*  stub  = nullptr;
         model_type*        model = nullptr;
   };
}
