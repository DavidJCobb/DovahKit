#pragma once
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}
class DKCompactObjectReferencePicker;
class DKFormPicker;
class ObjectReferenceLinkedRefsModel;
class QGroupBox;
class QItemSelection;
class QPushButton;
class QTableView;
class QWidget;

namespace ui::reference::fragments {
   class linked_refs {
      public:
         using loaded_form_type = dovah::loaded_forms::ObjectReference;
         using model_type       = ObjectReferenceLinkedRefsModel;

         struct control_collection {
            struct {
               QPushButton* add    = nullptr;
               QPushButton* remove = nullptr;
            } buttons;
            struct {
               QGroupBox*    groupbox = nullptr;
               DKFormPicker* keyword  = nullptr;
               DKCompactObjectReferencePicker* ref = nullptr;
            } edit;
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
