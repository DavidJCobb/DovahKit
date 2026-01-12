#pragma once
#include <optional>
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_region.h" // generated
#include "dovah/forms/Region.h"
#include "dovah/form_stub.h"
#include "ui/types/regions/region.h"
namespace dovah::exceptions {
   class form_creation_failed;
}
class RegionObjectsModel;
class RegionSoundsModel;
class RegionWeatherModel;

class RegionsDialog : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::Region;

   public:
      RegionsDialog(QWidget* parent = nullptr);

      void focusRegion(dovah::form_stub&);

   protected:
      Ui::RegionsDialog ui;
      std::optional<ui::types::regions::region> _current_region;
      struct {
         RegionObjectsModel* objects = nullptr;
         RegionSoundsModel*  sounds  = nullptr;
         RegionWeatherModel* weather = nullptr;
      } models;

      QModelIndex _get_selected_row();

      #pragma region Idle tree context menu
         void _context_region_create();
         void _context_region_use_info();
      #pragma endregion

      void _report_region_create_error(const dovah::exceptions::form_creation_failed&);

      void _pull_selected_region_to_ui();
      void _push_selected_region_to_form();
      void _set_form_ui_enable_state(bool);
};