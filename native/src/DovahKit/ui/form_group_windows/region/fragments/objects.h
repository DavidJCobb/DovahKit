#pragma once
#include <QMenu>
class DKColorPickerButton;
class RegionObjectsModel;
class RegionsDialog;
class QCheckBox;
class QDoubleSpinBox;
class QLineEdit;
class QModelIndex;
class QPushButton;
class QSpinBox;
class QTreeView;
class QWidget;
namespace ui::model_utils {
   class ViewEventFilter_RemoveRowOnDelKey;
}

namespace ui::region::fragments {
   class objects {
      public:
         using model_type = RegionObjectsModel;

      public:
         objects(RegionsDialog& o);

         struct controls {
            struct {
               QCheckBox* enable   = nullptr;
               QCheckBox* override = nullptr;
               QSpinBox*  priority = nullptr;
            } header;
            struct {
               QPushButton* move_up   = nullptr;
               QPushButton* move_down = nullptr;
            } buttons;
            struct {
               QWidget* container = nullptr;
               //
               struct {
                  struct {
                     QCheckBox* x = nullptr;
                     QCheckBox* y = nullptr;
                     QCheckBox* z = nullptr;
                  } invertible;
                  struct {
                     QDoubleSpinBox* x = nullptr;
                     QDoubleSpinBox* y = nullptr;
                     QDoubleSpinBox* z = nullptr;
                  } ranges;
               } angle_variance;
               QLineEdit*      base_editor_id   = nullptr;
               QSpinBox*       clustering       = nullptr;
               QCheckBox*      conform_to_slope = nullptr;
               QDoubleSpinBox* density          = nullptr;
               struct {
                  QDoubleSpinBox* min = nullptr;
                  QDoubleSpinBox* max = nullptr;
               } height;
               QCheckBox* is_huge_rock = nullptr;
               QCheckBox* is_tree      = nullptr;
               struct {
                  DKColorPickerButton* color      = nullptr;
                  QCheckBox*           enabled    = nullptr;
                  QSpinBox*            percentage = nullptr;
               } paint_vertices;
               QSpinBox* radius            = nullptr;
               QSpinBox* radius_wrt_parent = nullptr;
               struct {
                  QDoubleSpinBox* base     = nullptr;
                  QDoubleSpinBox* variance = nullptr;
               } sink;
               struct {
                  QCheckBox* invertible = nullptr;
                  QDoubleSpinBox* range = nullptr;
               } size_variance;
               struct {
                  QSpinBox* min = nullptr;
                  QSpinBox* max = nullptr;
               } slope;
            } edit;
            QTreeView* view = nullptr;
         };

      public:
         RegionsDialog& owner;
      protected:
         controls    ui;
         model_type* model = nullptr;
         struct {
            QMenu menu;
            struct {
               QAction* move_up   = nullptr;
               QAction* move_down = nullptr;
               QAction* remove    = nullptr;
            } actions;
         } view_context;
         ui::model_utils::ViewEventFilter_RemoveRowOnDelKey* remove_row_on_del = nullptr;

      public:
         void set_controls(controls&&);
         void reload();
         void commit();

         void on_object_selected(const QModelIndex&);
         void on_no_object_selected();
         void on_header_edited();
         void on_object_edited();
         void on_model_layout_edited();

         void update_button_enable_states();
         void try_move_item_up();
         void try_move_item_down();
         void try_remove_item();
   };
}