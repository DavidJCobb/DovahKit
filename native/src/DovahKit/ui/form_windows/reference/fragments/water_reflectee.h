#pragma once
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}
class ObjectReferenceReflectingWaterModel;
class QGroupBox;
class QItemSelection;
class QPushButton;
class QTableView;
class QWidget;

namespace ui::reference::fragments {
   class water_reflectee {
      public:
         using loaded_form_type = dovah::loaded_forms::ObjectReference;
         using model_type       = ObjectReferenceReflectingWaterModel;

         struct control_collection {
            struct {
               QPushButton* add    = nullptr;
               QPushButton* remove = nullptr;
            } buttons;
            QGroupBox*  groupbox = nullptr;
            QTableView* view     = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
      protected:
         void _on_selection_changed(const QItemSelection&);
         void _try_add_water();
         void _try_remove_water();

      protected:
         control_collection controls;
         dovah::form_stub*  stub  = nullptr;
         QWidget*           owner = nullptr;
         model_type*        model = nullptr;
   };
}
