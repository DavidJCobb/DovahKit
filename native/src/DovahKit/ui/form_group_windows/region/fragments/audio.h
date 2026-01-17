#pragma once
#include <QMenu>
class DKFormPicker;
class RegionsDialog;
class RegionSoundsModel;
class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QModelIndex;
class QPushButton;
class QSpinBox;
class QTableView;
namespace ui::model_utils {
   class ViewEventFilter_RemoveRowOnDelKey;
}

namespace ui::region::fragments {
   class audio {
      public:
         using model_type = RegionSoundsModel;

      public:
         audio(RegionsDialog& o);

         struct controls {
            struct {
               QCheckBox* enable   = nullptr;
               QCheckBox* override = nullptr;
               QSpinBox*  priority = nullptr;
            } header;
            DKFormPicker* music = nullptr;
            struct {
               QPushButton* add    = nullptr;
               QPushButton* remove = nullptr;
            } buttons;
            struct {
               QGroupBox* container = nullptr;
               //
               QDoubleSpinBox* chance = nullptr;
               DKFormPicker*   sound = nullptr;
               struct {
                  QCheckBox* pleasant = nullptr;
                  QCheckBox* cloudy   = nullptr;
                  QCheckBox* rainy    = nullptr;
                  QCheckBox* snowy    = nullptr;
               } weather;
            } edit;
            QTableView* view = nullptr;
         };

      public:
         RegionsDialog& owner;
      protected:
         controls    ui;
         model_type* model = nullptr;
         struct {
            QMenu menu;
            struct {
               QAction* insert = nullptr;
               QAction* remove = nullptr;
            } actions;
         } view_context;
         ui::model_utils::ViewEventFilter_RemoveRowOnDelKey* remove_row_on_del = nullptr;

      public:
         void set_controls(controls&&);
         void reload();
         void commit();

         void on_item_selected(const QModelIndex&);
         void on_no_item_selected();
         void on_header_edited();
         void on_item_edited();
         void on_model_layout_edited();

         void try_add_sound();
         void try_remove_item();
   };
}