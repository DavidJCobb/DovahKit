#pragma once
#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components::extra_data_types {
         class activate_parents;
      }
      class ObjectReference;
   }
   class form_stub;
}
class DKCompactObjectReferencePicker;
class ObjectReferenceActivateParentsModel;
class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QItemSelection;
class QPushButton;
class QTableView;
class QWidget;

namespace ui::reference::fragments {
   class activate_parents {
      public:
         using extra_data_type  = dovah::loaded_forms::components::extra_data_types::activate_parents;
         using loaded_form_type = dovah::loaded_forms::ObjectReference;
         using model_type       = ObjectReferenceActivateParentsModel;

         struct control_collection {
            struct {
               QPushButton* add    = nullptr;
               QPushButton* remove = nullptr;
            } buttons;
            struct {
               QDoubleSpinBox* delay    = nullptr;
               QGroupBox*      groupbox = nullptr;
               DKCompactObjectReferencePicker* ref = nullptr;
            } edit;
            struct {
               QCheckBox* parent_activate_only = nullptr;
            } flags;
            QTableView* view = nullptr;
         };
         
      public:
         void setup(QWidget& owner, const control_collection&);
         void load(loaded_form_type&);
         void save(loaded_form_type&);
      protected:
         void _on_selection_changed(const QItemSelection&);
         void _try_add_link();
         void _remove_selected_link();
         void _update_selected_link();

      protected:
         control_collection controls;
         dovah::form_stub*  stub = nullptr;

         QWidget*    owner = nullptr;
         model_type* model = nullptr;
   };
}
