#include "./RegionsDialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/forms/Region.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/use_info/entry_flags/region.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/open_window_for_form.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./RegionsAvailableInWorldModel.h"
#include "./RegionSoundsModel.h"

RegionsDialog::RegionsDialog(QWidget* parent) :
   QDialog(parent),
   fragments{
      .objects{*this},
      .weather{*this},
   }
{
   this->ui.setupUi(this);
   this->ui.tabWidget->setCurrentWidget(this->ui.tabGeneral);

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
      //
      // TODO: other fragments
      //
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

   auto& editor = DovahKitCore::get();
   emit editor.formModificationImminent(data.stub);
   data.export_data(*loaded);
   data.stub->edit_record_flags(dovah::loaded_forms::Region::form_flag::border_region, data.is_border_region);
   data.stub->set_edited(true);
   emit editor.formModified(data.stub);
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

void RegionsDialog::_pull_selected_region_to_ui() {
   this->fragments.objects.reload();
   this->fragments.weather.reload();
   // TODO: other fragments

   auto& data = this->_current_region;
   if (!data.stub) {
      //
      // TODO: Clear fields
      //
      return;
   }

   const auto blockers = std::array{
      QSignalBlocker(this->ui.editorID),
      QSignalBlocker(this->ui.colorPresent),
      QSignalBlocker(this->ui.color),
      QSignalBlocker(this->ui.edgeFalloff),
   };

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