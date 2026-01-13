#pragma once
#include <optional>
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_region.h" // generated
#include "dovah/forms/Region.h"
#include "dovah/form_stub.h"
#include "ui/types/regions/region.h"
#include "./fragments/objects.h"
#include "./fragments/weather.h"
#include "./fragment_passkey.h"
namespace dovah::exceptions {
   class form_creation_failed;
}
class RegionsAvailableInWorldModel;
class RegionSoundsModel;

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
         ui::region::fragments::objects objects;
         ui::region::fragments::weather weather;
      } fragments;
      struct {
         RegionsAvailableInWorldModel* available_regions = nullptr;
         RegionSoundsModel*  sounds  = nullptr;
      } models;

      void _report_region_create_error(const dovah::exceptions::form_creation_failed&);

      void _commit_pending_changes();
      void _set_selected_region(dovah::form_stub*);

      void _pull_selected_region_to_ui();
      void _push_selected_region_to_form();
      void _set_form_ui_enable_state(bool);
};