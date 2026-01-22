#include "./RegionsDialog.h"
#include <cmath>
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/core_constants/exterior_cell_side_length.h"
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/forms/components/extra_data/types/c/cell_region_list.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/Region.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/form_stubs/helpers/get_worldspace_cell_by_grid.h"
#include "dovah/use_info/entry_flags/region.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/open_window_for_form.h"
#include "editor/subsystems/message_log/core.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./RegionsAvailableInWorldModel.h"
#include "./RegionCanvasWidget.h"

RegionsDialog::RegionsDialog(QWidget* parent) :
   QDialog(parent),
   fragments{
      .audio{*this},
      .grass{*this},
      .landscape{*this},
      .map{*this},
      .objects{*this},
      .weather{*this},
   }
{
   this->ui.setupUi(this);
   this->ui.tabWidget->setCurrentWidget(this->ui.tabGeneral);

   #pragma region Set up canvas widget
      this->canvas = new RegionCanvasWidget(this);
      {
         auto* layout = new QVBoxLayout;
         this->ui.gridFrame->setLayout(layout);
         layout->setContentsMargins(0, 0, 0, 0);
         layout->addWidget(this->canvas);
      }
      QObject::connect(this->canvas, &RegionCanvasWidget::onRegionAreasEdited, this, [this]() {
         this->_current_region_edited = true;
         this->_commit_pending_changes();
      });
      QObject::connect(this->canvas, &RegionCanvasWidget::onRegionChangeRequested, this, [this](dovah::form_stub* region) {
         this->_set_selected_region(region);
      });
      //
      // Checkboxes for showing region colors in the grid:
      //
      {
         const auto checkboxes = std::array{
            this->ui.gridFilterGrass,
            this->ui.gridFilterLand,
            this->ui.gridFilterMap,
            this->ui.gridFilterNothing,
            this->ui.gridFilterObjects,
            this->ui.gridFilterSound,
            this->ui.gridFilterWeather,
         };
         for (auto* checkbox : checkboxes) {
            QObject::connect(checkbox, &QCheckBox::toggled, this, &RegionsDialog::_update_region_canvas_color_reqs);
         }
         QObject::connect(this->ui.buttonCheckAllGridFilters, &QPushButton::clicked, this, [this, checkboxes]() {
            for (auto* checkbox : checkboxes) {
               const auto blocker = QSignalBlocker(checkbox);
               checkbox->setChecked(true);
            }
            this->_update_region_canvas_color_reqs();
         });
         QObject::connect(this->ui.buttonUncheckAllGridFilters, &QPushButton::clicked, this, [this, checkboxes]() {
            for (auto* checkbox : checkboxes) {
               const auto blocker = QSignalBlocker(checkbox);
               checkbox->setChecked(false);
            }
            this->_update_region_canvas_color_reqs();
         });
      }
      this->_update_region_canvas_color_reqs();
   #pragma endregion

   {
      auto* model = this->models.available_regions = new RegionsAvailableInWorldModel(this);
      auto* view  = this->ui.regions;
      view->setModel(model);
      ui::typical_tableview_config(view);
      ui::set_tableview_column_flex(view, [this](DKHeaderView& header, const QFontMetrics& metrics) {
         header.setColumnFlex(0, 1, 1);
      });

      auto* world_picker = this->ui.currentWorld;
      world_picker->setAllowedFormType(dovah::form_type::worldspace);
      QObject::connect(world_picker, &DKFormPicker::formChanged, model, &RegionsAvailableInWorldModel::setWorldspace);
      QObject::connect(world_picker, &DKFormPicker::formChanged, this->canvas, &RegionCanvasWidget::setWorldspace);

      auto* sel_model = view->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& sel) {
         if (sel.empty()) {
            _set_selected_region(nullptr);
         } else {
            auto* stub = sel[0].topLeft().data(RegionsAvailableInWorldModel::FormStubRole).value<dovah::form_stub*>();
            _set_selected_region(stub);
         }
      });
   }

   ui::set_unsigned_range<float>(this->ui.edgeFalloff);

   #pragma region Set up fragments
      this->fragments.objects.set_controls({
         .header = {
            .enable   = this->ui.objectsEnable,
            .override = this->ui.objectsOverride,
            .priority = this->ui.objectsPriority,
         },
         .buttons = {
            .move_up   = this->ui.buttonObjectsMoveUp,
            .move_down = this->ui.buttonObjectsMoveDown,
         },
         .edit = {
            .container = this->ui.currentObjectEditControls,
            //
            .angle_variance = {
               .invertible = {
                  .x = this->ui.currentObjectAngleSignedX,
                  .y = this->ui.currentObjectAngleSignedY,
                  .z = this->ui.currentObjectAngleSignedZ,
               },
               .ranges = {
                  .x = this->ui.currentObjectAngleRangeX,
                  .y = this->ui.currentObjectAngleRangeY,
                  .z = this->ui.currentObjectAngleRangeZ,
               },
            },
            .base_editor_id   = this->ui.currentObjectEditorID,
            .clustering       = this->ui.currentObjectClustering,
            .conform_to_slope = this->ui.currentObjectConformToSlope,
            .density          = this->ui.currentObjectDensity,
            .height = {
               .min = this->ui.currentObjectHeightMin,
               .max = this->ui.currentObjectHeightMax,
            },
            .is_huge_rock   = this->ui.currentObjectIsHugeRock,
            .is_tree        = this->ui.currentObjectIsTree,
            .paint_vertices = {
               .color      = this->ui.currentObjectPaintColor,
               .enabled    = this->ui.currentObjectPaintEnable,
               .percentage = this->ui.currentObjectPaintPercent,
            },
            .radius            = this->ui.currentObjectRadius,
            .radius_wrt_parent = this->ui.currentObjectRadiusWRTParent,
            .sink = {
               .base     = this->ui.currentObjectSink,
               .variance = this->ui.currentObjectSinkVariance,
            },
            .size_variance = {
               .invertible = this->ui.currentObjectSizeVarianceSigned,
               .range      = this->ui.currentObjectSizeVariance,
            },
            .slope = {
               .min = this->ui.currentObjectSlopeMin,
               .max = this->ui.currentObjectSlopeMax,
            },
         },
         .view = this->ui.objectsEntries,
      });
      this->fragments.weather.set_controls({
         .header = {
            .enable   = this->ui.weatherEnable,
            .override = this->ui.weatherOverride,
            .priority = this->ui.weatherPriority,
         },
         .buttons = {
            .add    = this->ui.buttonWeatherAdd,
            .remove = this->ui.buttonWeatherRemove,
         },
         .edit = {
            .container = this->ui.currentWeatherGroupbox,
            //
            .weather = this->ui.currentWeatherForm,
            .chance  = {
               .constant = {
                  .radio = this->ui.currentWeatherChanceTypeFixed,
                  .value = this->ui.currentWeatherChanceValueFixed,
               },
               .form = {
                  .radio = this->ui.currentWeatherChanceTypeGlobal,
                  .value = this->ui.currentWeatherChanceValueGlobal,
               },
            },
         },
         .view = this->ui.weatherEntries,
      });
      this->fragments.map.set_controls({
         .header = {
            .enable   = this->ui.mapEnable,
            .override = this->ui.mapOverride,
            .priority = this->ui.mapPriority,
         },
         .name = this->ui.mapName,
      });
      this->fragments.landscape.set_controls({
         .header = {
            .enable   = this->ui.landscapeEnable,
            .override = this->ui.landscapeOverride,
            .priority = this->ui.landscapePriority,
         },
         .texture = this->ui.landscapeTexture,
      });
      this->fragments.grass.set_controls({
         .header = {
            .enable   = this->ui.grassEnable,
            .override = this->ui.grassOverride,
            .priority = this->ui.grassPriority,
         },
      });
      this->fragments.audio.set_controls({
         .header = {
            .enable   = this->ui.soundEnable,
            .override = this->ui.soundOverride,
            .priority = this->ui.soundPriority,
         },
         .music = this->ui.soundMusicType,
         .buttons = {
            .add    = this->ui.buttonSoundNew,
            .remove = this->ui.buttonSoundRemove,
         },
         .edit = {
            .container = this->ui.currentSoundGroupbox,
            //
            .chance  = this->ui.currentSoundChance,
            .sound   = this->ui.currentSoundDescriptor,
            .weather = {
               .pleasant = this->ui.currentSoundPleasant,
               .cloudy   = this->ui.currentSoundCloudy,
               .rainy    = this->ui.currentSoundRainy,
               .snowy    = this->ui.currentSoundSnowy,
            },
         },
         .view = this->ui.sounds,
      });
   #pragma endregion

   #pragma region General tab
      QObject::connect(this->ui.editorID, &QLineEdit::textChanged, this, [this]() { this->_current_region_edited = true; });
      QObject::connect(this->ui.color, &DKColorPickerButton::colorChanged, this, [this](QColor color) {
         if (color == QColor(0, 0, 0)) {
            //
            // Bethesda uses RGB(0, 0, 0) to signal the absence of a color.
            //
            const auto blocker = QSignalBlocker(this->ui.color);
            color = QColor(1, 0, 0);
            this->ui.color->setColor(color);
         }
         this->_current_region_edited = true;
         this->_push_region_color_to_canvas();
      });
      QObject::connect(this->ui.colorPresent, &QCheckBox::toggled, this, [this](bool checked) {
         this->_current_region_edited = true;
         this->_push_region_color_to_canvas();
      });
      QObject::connect(this->ui.edgeFalloff, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this]() { this->_current_region_edited = true; });
      QObject::connect(this->ui.flagBorder, &QCheckBox::toggled, this, [this]() { this->_current_region_edited = true; });
   #pragma endregion

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_set_selected_region(nullptr);
   });
}

void RegionsDialog::focusRegion(dovah::form_stub& region) {
   if (region.form_type != dovah::form_type::region)
      return;

   auto* world = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::region::worldspace>(region);
   this->ui.currentWorld->setFormStub(world);

   auto region_qmi = this->models.available_regions->regionIndex(region);
   this->ui.regions->scrollTo(region_qmi);
   this->ui.regions->selectionModel()->select({ region_qmi, region_qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

ui::types::regions::region& RegionsDialog::region_data(ui::region::fragment_passkey) {
   return this->_current_region;
}
void RegionsDialog::on_region_modified(ui::region::fragment_passkey) {
   this->_current_region_edited = true;
   this->_push_region_data_presence_to_canvas();
}

#pragma region Event handlers
   /*virtual*/ void RegionsDialog::closeEvent(QCloseEvent* e) /*override*/ {
      this->_commit_pending_changes();
      QDialog::closeEvent(e);
   }
   /*virtual*/ void RegionsDialog::focusOutEvent(QFocusEvent* e) /*override*/ {
      this->_commit_pending_changes();
      QDialog::focusOutEvent(e);
   }
#pragma endregion

void RegionsDialog::_report_region_create_error(const dovah::exceptions::form_creation_failed& ex) {
   QString text;
   switch (ex.code) {
      using enum dovah::exceptions::form_creation_failed::error_code;
      case invalid_form_type:
         text = QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type. (Wait, what? How did you get the Region Editor window to try to do that?)");
         break;
      case no_active_file:
         text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
         break;
      case no_form_id_available:
         text = QObject::tr("You've used up all of the form IDs available to this file!");
         break;
      case unimplemented_form_type:
         text = QObject::tr("DovahKit does not support editing this form type. (Wait, what? How did you get the Region Editor window to try to do that?)");
         break;
      case invalid_parent_child_relationship:
         text = QObject::tr("The specified parent form cannot have a child form of this type. (Wait, what? How did you get the Region Editor window to try to do that?)");
         break;
      case exterior_grid_coordinates_already_taken:
         text = QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates. (Wait, what? How did you get the Region Editor window to try and create an exterior cell?)");
         break;
      case cannot_create_reference_with_no_parent_cell:
         text = QObject::tr("References cannot be created outside of a cell. (Wait, what? How did you get the Region Editor window to try and create a reference?)");
         break;
      case interior_cell_clone_cannot_have_parent:
         text = QObject::tr("Interior cells cannot have a parent worldspace. (Wait, what? How did you get the Region Editor window to try and create an interior cell?)");
         break;
      case exterior_cell_clone_must_have_parent:
         text = QObject::tr("Exterior cells must have a parent worldspace. (Wait, what? How did you get the Region Editor window to try and create an exterior cell?)");
         break;
      case cannot_sever_references_to_none_stub:
         text = QObject::tr("DovahKit needed to select a form ID to use for the new form. The chosen form ID is the target of one or more dangling references, and DovahKit does not know how to sever those references, so the form creation process could not continue.");
         break;
      case form_type_unavailable_in_current_game:
         text = QObject::tr("The desired form type doesn't exist in the version (e.g. Classic/Special) of Skyrim this file was created for. Try converting the file to the target Skyrim version first. (Wait, what? How did you get the Region Editor window to try to do that?)");
         break;
   }
   QMessageBox::critical(
      this,
      QObject::tr("Error", "create new form error"),
      QObject::tr("Unable to create a new region. %1").arg(text)
   );
}

void RegionsDialog::_commit_pending_changes() {
   if (!this->_current_region_edited)
      return;
   auto& data = this->_current_region;
   if (!data.stub)
      return;
   auto loaded = data.stub->load().ptr_cast<dovah::loaded_forms::Region>();
   if (!loaded)
      return;

   this->fragments.objects.commit();
   this->fragments.weather.commit();
   this->fragments.map.commit();
   this->fragments.landscape.commit();
   this->fragments.grass.commit();
   this->fragments.audio.commit();

   auto* worldspace = this->canvas->worldspace();

   auto& editor = DovahKitCore::get();
   this->_update_cells_in_region_areas();
   emit editor.formModificationImminent(data.stub);
   data.export_data(*loaded);
   data.stub->edit_record_flags(dovah::loaded_forms::Region::form_flag::border_region, data.is_border_region);
   data.stub->set_edited(true);
   emit editor.formModified(data.stub);
}
void RegionsDialog::_update_cells_in_region_areas() {
   auto& data      = this->_current_region;
   auto* worldspace = this->canvas->worldspace();
   
   //
   // This code is terrible, but it's not worth improving. Regions are rather 
   // low on my priority list, and no one outside of Bethesda ever uses them 
   // for anything but expanding border regions to add playable spaces.
   //

   data.bounds.areas = this->canvas->regionAreas();
   if (!data.bounds.areas.empty()) {
      data.bounds.worldspace = worldspace;
   }
   for (auto& area : data.bounds.areas) {
      area.edge_falloff = this->ui.edgeFalloff->value();
   }
   //
   // The optimal way to check what cells a region area overlaps would be to 
   // triangulate the region area and check for triangle/AABB intersections. 
   // Triangulation need not involve allocating or storing data; we could just 
   // examine the polygon's vertices three at a time.
   // 
   // Unfortunately, however, I'm extremely bad at math, and I can't find any 
   // legible and correct algorithms for 2D triangle/AABB intersections; only 
   // 3D intersections. I grasp the theory perfectly well -- use the separating 
   // axis theorem to early-out on checking the two -- but I can't stitch the 
   // formulae together in my head, and I can't find anything usable online; 
   // the 3D algorithms are full of completely unlabeled and unexplained vars. 
   // Skilled mathematicians can never seem to write anything legible, and 
   // unskilled mathematicians can never seem to write anything correct. So 
   // wasteful as it is, we shall outsource the math to Qt: we'll copy the 
   // vertex list into a QPolygon, and spawn a bunch of additional QPolygons to 
   // run polygon/polygon intersection tests. (Qt offers a convenience function 
   // for creating a QPolygon from a QRect, so they clearly understand how 
   // useful it is to do polygon/AABB intersection checks, but they didn't 
   // implement an API to run them directly.)
   //
   std::vector<QPolygonF> polygons;
   for (auto& area : data.bounds.areas) {
      auto& poly = polygons.emplace_back();
      for (auto& point : area.points)
         poly << QPointF{ point.x, point.y };
   }
   //
   // First, find cells that previously listed this region in CELL/XCLR, but 
   // which are no longer included in the region's areas.
   //
   for (auto& pair : data.stub->inbound) {
      auto* cell = pair.second.other;
      if (!cell || !cell->is_exterior_cell() || cell->get_parent_form() != worldspace)
         continue;

      bool is_in_area = false;
      QRect cell_rect;
      {
         int32_t gx;
         int32_t gy;
         if (cell->get_grid_coordinates(gx, gy)) {
            cell_rect.setLeft(gx * dovah::core_constants::exterior_cell_side_length);
            cell_rect.setTop(gy * dovah::core_constants::exterior_cell_side_length);
            cell_rect.setWidth(dovah::core_constants::exterior_cell_side_length);
            cell_rect.setHeight(dovah::core_constants::exterior_cell_side_length);
         }
      }
      if (!cell_rect.isNull()) {
         const auto cell_poly = QPolygon(cell_rect, false);
         for (const auto& regn_poly : polygons) {
            if (regn_poly.intersects(cell_poly)) {
               is_in_area = true;
               break;
            }
         }
      }

      if (!is_in_area) {
         //
         // Remove region from CELL/XCLR:
         //
         _remove_region_from_cell(*data.stub, *cell);
      }
   }
   //
   // Next, find all grid cells that this region overlaps. Add the region to 
   // the relevant cell forms' XCLR lists, creating cell forms as needed.
   //
   std::vector<QPoint> grid_coords; // basically a std::set, but QPoints can't go in those
   for (const auto& poly : polygons) {
      auto   bound = poly.boundingRect();
      QPoint min;
      QPoint max;
      {
         auto min_f = bound.topLeft()     / dovah::core_constants::exterior_cell_side_length;
         auto max_f = bound.bottomRight() / dovah::core_constants::exterior_cell_side_length;
         {
            auto x = (int)std::floor(min_f.x()); // casts suppress errors on narrowing conversion in braced init
            auto y = (int)std::floor(min_f.y());
            min = { x, y };
         }
         {
            auto x = (int)std::ceil(max_f.x()); // casts suppress errors on narrowing conversion in braced init
            auto y = (int)std::ceil(max_f.y());
            max = { x, y };
         }
      }
         
      for (int u = min.x(); u <= max.x(); ++u) {
         for (int v = min.y(); v < max.y(); ++v) {
            auto cell_rect = QRect(
               u * dovah::core_constants::exterior_cell_side_length,
               v * dovah::core_constants::exterior_cell_side_length,
               dovah::core_constants::exterior_cell_side_length,
               dovah::core_constants::exterior_cell_side_length
            );
            auto cell_poly = QPolygon(cell_rect, false);
            if (poly.intersects(cell_poly)) {
               QPoint point{ u, v };

               bool already = false;
               for (const auto& item : grid_coords) {
                  if (item == point) {
                     already = true;
                     break;
                  }
               }
               if (already)
                  continue;

               grid_coords.push_back(point);
            }
         }
      }
   }
   //
   // We gather the cell coords first, and then edit cells, so that we avoid 
   // loading and editing a single cell multiple times.
   //
   for (auto& grid_coord : grid_coords) {
      auto* cell = dovah::form_stub_helpers::get_worldspace_cell_by_grid(*worldspace, grid_coord.x(), grid_coord.y());
      if (!cell) {
         //
         // Create cell if non-existent:
         //
         auto& editor  = DovahKitCore::get();
         auto  request = editor.request_form_creation(dovah::form_type::cell);
         request.cell_grid_coordinates.emplace() = { grid_coord.x(), grid_coord.y() };
         request.set_parent_form(worldspace);
         try {
            cell = request.commit();
         } catch (...) {
            dovahkit::subsystems::message_log::core::get().addLogItem({
               tr("Failed to create cell (%1, %2) in worldspace %3 while updating areas for region %4.")
                  .arg(grid_coord.x())
                  .arg(grid_coord.y())
                  .arg(editor_helpers::form_identifiers_to_string(worldspace))
                  .arg(editor_helpers::form_identifiers_to_string(data.stub)),
               ui::types::log_item_type::warning,
               ui::types::log_item_context::unspecified
            });
         }
         if (!cell)
            continue;
      }
      _add_region_to_cell(*data.stub, *cell);
   }
}
void RegionsDialog::_add_region_to_cell(dovah::form_stub& region, dovah::form_stub& cell) {
   auto loaded = cell.load().ptr_cast<dovah::loaded_forms::Cell>();
   if (!loaded) {
      dovahkit::subsystems::message_log::core::get().addLogItem({
         tr("Failed to load cell %1 in worldspace %2 while updating areas for region %3. Can't add the region to the cell's region list.")
            .arg(editor_helpers::form_identifiers_to_string(&cell))
            .arg(editor_helpers::form_identifiers_to_string(cell.get_parent_form()))
            .arg(editor_helpers::form_identifiers_to_string(this->_current_region.stub)),
         ui::types::log_item_type::warning,
         ui::types::log_item_context::unspecified
      });
      return;
   }
   auto* extra   = loaded->extra_data.get_or_create<dovah::loaded_forms::components::extra_data_types::cell_region_list>();
   bool  already = false;
   for (auto& use : extra->regions) {
      if (use.get_form_stub() == &region) {
         already = true;
         break;
      }
   }
   if (!already) {
      auto& editor = DovahKitCore::get();
      editor.formModificationImminent(&cell);
      extra->regions.emplace_back().set(*loaded, &region);
      cell.set_edited(true);
      editor.formModified(&cell);
   }
}
void RegionsDialog::_remove_region_from_cell(dovah::form_stub& region, dovah::form_stub& cell) {
   auto loaded = cell.load().ptr_cast<dovah::loaded_forms::Cell>();
   if (!loaded) {
      dovahkit::subsystems::message_log::core::get().addLogItem({
         tr("Failed to load cell %1 in worldspace %2 while updating areas for region %3. Can't remove the region from the cell's region list.")
            .arg(editor_helpers::form_identifiers_to_string(&cell))
            .arg(editor_helpers::form_identifiers_to_string(cell.get_parent_form()))
            .arg(editor_helpers::form_identifiers_to_string(this->_current_region.stub)),
         ui::types::log_item_type::warning,
         ui::types::log_item_context::unspecified
      });
      return;
   }
   auto* extra = loaded->extra_data.get<dovah::loaded_forms::components::extra_data_types::cell_region_list>();
   if (!extra)
      return;
   bool present = false;
   for(size_t i = 0; i < extra->regions.size(); ++i) {
      auto& use = extra->regions[i];
      if (use.get_form_stub() == &region) {
         present = true;
         break;
      }
   }
   if (present) {
      emit DovahKitCore::get().formModificationImminent(&cell);
      dovah::remove_form_from_reference_list(extra->regions, region, *loaded);
      if (extra->regions.empty()) {
         extra = nullptr;
         loaded->extra_data.remove<dovah::loaded_forms::components::extra_data_types::cell_region_list>(*loaded);
      }
      cell.set_edited(true);
      emit DovahKitCore::get().formModified(&cell);
   }
}
void RegionsDialog::_set_selected_region(dovah::form_stub* region) {
   if (!region) {
      _commit_pending_changes();
      this->_current_region        = {};
      this->_current_region_edited = false;
      this->_pull_selected_region_to_ui();
      return;
   }
   auto loaded = region->load().ptr_cast<dovah::loaded_forms::Region>();
   if (!loaded) {
      _commit_pending_changes();
      this->_current_region        = {};
      this->_current_region_edited = false;
      this->_pull_selected_region_to_ui();
      return;
   }
   auto& data = this->_current_region;
   data = {};
   data.import_data(*loaded);
   data.import_record_flags(region->get_record_flags());
   this->_pull_selected_region_to_ui();
}

void RegionsDialog::_push_region_color_to_canvas() {
   auto* stub = this->_current_region.stub;
   if (!stub)
      return;

   auto color = this->ui.color->color();
   if (!this->ui.colorPresent->isChecked())
      color = QColor(0, 0, 0);

   this->canvas->forceRegionColor(*stub, color);
}
void RegionsDialog::_push_region_data_presence_to_canvas() {
   if (!this->_current_region.stub)
      return;
   RegionCanvasWidget::RegionDataPresence presence;
   presence.audio     = this->ui.soundEnable->isChecked();
   presence.grass     = this->ui.grassEnable->isChecked();
   presence.landscape = this->ui.landscapeEnable->isChecked();
   presence.map       = this->ui.mapEnable->isChecked();
   presence.objects   = this->ui.objectsEnable->isChecked();
   presence.weather   = this->ui.weatherEnable->isChecked();
   this->canvas->forceRegionDataPresence(*this->_current_region.stub, presence);
}
void RegionsDialog::_update_region_canvas_color_reqs() {
   RegionCanvasWidget::RegionColorRequirements req;
   req.audio     = this->ui.gridFilterSound->isChecked();
   req.grass     = this->ui.gridFilterGrass->isChecked();
   req.landscape = this->ui.gridFilterLand->isChecked();
   req.map       = this->ui.gridFilterMap->isChecked();
   req.objects   = this->ui.gridFilterObjects->isChecked();
   req.weather   = this->ui.gridFilterWeather->isChecked();
   req.empty     = this->ui.gridFilterNothing->isChecked();
   this->canvas->setColorRequirements(req);
}

void RegionsDialog::_pull_selected_region_to_ui() {
   this->fragments.objects.reload();
   this->fragments.weather.reload();
   this->fragments.map.reload();
   this->fragments.landscape.reload();
   this->fragments.grass.reload();
   this->fragments.audio.reload();

   const auto blockers = std::array{
      QSignalBlocker(this->ui.editorID),
      QSignalBlocker(this->ui.colorPresent),
      QSignalBlocker(this->ui.color),
      QSignalBlocker(this->ui.edgeFalloff),
   };

   auto& data = this->_current_region;
   if (!data.stub) {
      this->canvas->setNoRegion();
      this->ui.editorID->setText("");
      //
      // TODO: Clear fields
      //
      return;
   }

   #pragma region General
      this->ui.editorID->setText(data.stub->get_editor_id());
      if (data.map_color != QColor(0, 0, 0)) {
         this->ui.colorPresent->setChecked(true);
         this->ui.color->setEnabled(true);
         this->ui.color->setColor(data.map_color);
      } else {
         this->ui.colorPresent->setChecked(false);
         this->ui.color->setEnabled(false);
      }

      // Technically, each region area can have its own falloff, but Bethesda's 
      // UI design doesn't allow for this.
      this->ui.edgeFalloff->setValue(1024);
      if (!data.bounds.areas.empty()) {
         this->ui.edgeFalloff->setValue(data.bounds.areas[0].edge_falloff);
      }

      this->ui.flagBorder->setChecked(data.is_border_region);
   #pragma endregion

   this->canvas->setRegion(data);

   this->_set_form_ui_enable_state(true);
}
void RegionsDialog::_push_selected_region_to_form() {
   //
   // TODO
   //
}
void RegionsDialog::_set_form_ui_enable_state(bool v) {
   //
   // TODO
   //
}