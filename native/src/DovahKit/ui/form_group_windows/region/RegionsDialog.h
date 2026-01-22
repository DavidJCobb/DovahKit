#pragma once
#include <optional>
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_region.h" // generated
#include "dovah/forms/Region.h"
#include "dovah/form_stub.h"
#include "ui/types/regions/region.h"
#include "./fragments/audio.h"
#include "./fragments/grass.h"
#include "./fragments/landscape.h"
#include "./fragments/map.h"
#include "./fragments/objects.h"
#include "./fragments/weather.h"
#include "./fragment_passkey.h"
namespace dovah::exceptions {
   class form_creation_failed;
}
class RegionsAvailableInWorldModel;
class RegionCanvasWidget;

class RegionsDialog : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::Region;

   public:
      RegionsDialog(QWidget* parent = nullptr);

      void focusRegion(dovah::form_stub&);

   public: // passkeyed
      ui::types::regions::region& region_data(ui::region::fragment_passkey);
      void on_region_modified(ui::region::fragment_passkey);

   protected:
      Ui::RegionsDialog ui;
      ui::types::regions::region _current_region;
      bool _current_region_edited = false;
      struct {
         ui::region::fragments::audio     audio;
         ui::region::fragments::grass     grass;
         ui::region::fragments::landscape landscape;
         ui::region::fragments::map       map;
         ui::region::fragments::objects   objects;
         ui::region::fragments::weather   weather;
      } fragments;
      struct {
         RegionsAvailableInWorldModel* available_regions = nullptr;
      } models;
      RegionCanvasWidget* canvas = nullptr;
      struct {
         struct {
            QMenu menu;
            struct {
               QAction* create_form = nullptr;
               QAction* delete_form = nullptr;
               QAction* use_info    = nullptr;
            } actions;
         } region_list;
      } context;

      #pragma region Event handlers
         virtual void closeEvent(QCloseEvent*) override;
         virtual void focusOutEvent(QFocusEvent*) override;
      #pragma endregion

      void _report_region_create_error(const dovah::exceptions::form_creation_failed&);

      void _commit_pending_changes();
      void _update_cells_in_region_areas();
      void _add_region_to_cell(dovah::form_stub& region, dovah::form_stub& cell);
      void _remove_region_from_cell(dovah::form_stub& region, dovah::form_stub& cell);
      void _set_selected_region(dovah::form_stub*);

      void _push_region_color_to_canvas();
      void _push_region_data_presence_to_canvas();
      void _update_region_canvas_color_reqs();

      void _pull_selected_region_to_ui();

      // Context menu handlers
      void _create_region();
      void _delete_selected_region();
      void _show_region_use_info();
};