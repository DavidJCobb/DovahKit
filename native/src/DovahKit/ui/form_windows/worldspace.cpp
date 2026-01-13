#include "./worldspace.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

FormDialogWorldspace::FormDialogWorldspace(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);

   {
      auto* widget = this->ui.parent;
      widget->setAllowedFormType(dovah::form_type::worldspace);
      widget->setCustomFilter(this->filters.exclude_self);
      QObject::connect(widget, &DKFormPicker::formChanged, this, [this]() { this->_on_has_parent_changed(); });
      _on_has_parent_changed(true);
   }
   {
      auto* checkbox = this->ui.doNotInheritClimate;
      auto* picker   = this->ui.climate;
      picker->setAllowedFormType(dovah::form_type::climate);
      QObject::connect(checkbox, &QCheckBox::toggled, picker, &QWidget::setEnabled);
   }
   {
      auto* checkbox = this->ui.doNotInheritWaterType;
      auto* picker   = this->ui.waterType;
      picker->setAllowedFormType(dovah::form_type::water_type);
      QObject::connect(checkbox, &QCheckBox::toggled, picker, &QWidget::setEnabled);
   }
   ui::set_range<float>(this->ui.defaultHeightLand);
   ui::set_range<float>(this->ui.defaultHeightWater);
   ui::set_range<float>(this->ui.lodWaterHeight);
   this->ui.lodWaterType->setAllowedFormType(dovah::form_type::water_type);
   ui::set_range<int32_t>(this->ui.mapBoundsX);
   ui::set_range<int32_t>(this->ui.mapBoundsY);
   ui::set_range<float>(this->ui.cameraMinHeight);
   ui::set_range<float>(this->ui.cameraMaxHeight);
   this->ui.cameraPitch->setRange(0, 360.0F);
   for (auto* spinbox : std::array{
      this->ui.mapCellCoordsStartX,
      this->ui.mapCellCoordsStartY,
      this->ui.mapCellCoordsEndX,
      this->ui.mapCellCoordsEndY,
   }) {
      ui::set_range<int16_t>(spinbox);
   }
   for (auto* spinbox : std::array{
      this->ui.mapCellOffsetX,
      this->ui.mapCellOffsetY,
      this->ui.mapCellOffsetZ,
      this->ui.mapScale,
   }) {
      ui::set_range<float>(spinbox);
   }

   QObject::connect(this->ui.flagFixedDimensions, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.smallGridCenterX->setEnabled(checked);
      this->ui.smallGridCenterY->setEnabled(checked);
   });
   this->ui.smallGridCenterX->setEnabled(false);
   this->ui.smallGridCenterY->setEnabled(false);
   for (auto* spinbox : std::array{
      this->ui.smallGridCenterX,
      this->ui.smallGridCenterY,
   }) {
      ui::set_range<int16_t>(spinbox);
   }

   this->ui.encounterZone->setAllowedFormType(dovah::form_type::encounter_zone);
   this->ui.lightingTemplate->setAllowedFormType(dovah::form_type::lighting_template);
   this->ui.location->setAllowedFormType(dovah::form_type::location);
   this->ui.musicType->setAllowedFormType(dovah::form_type::music_type);
   {
      auto* slider  = this->ui.distantLODMultSlider;
      auto* spinbox = this->ui.distantLODMultSpinbox;
      QObject::connect(spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), slider, [slider](double v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
      QObject::connect(slider, &DKFloatSlider::valueChanged, spinbox, [spinbox](double v) {
         spinbox->setValue(v);
      });
   }

   this->load(); // this creates the working copy.
}
void FormDialogWorldspace::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->filters.exclude_self->set_exclusion(&working.stub);

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.parent, working.parent.form, working);

   ui::bind(this->ui.flagUseSkyCell, working.parent.flags, loaded_form_type::parent_flag::use_parent_sky_cell);
   ui::bind_inverse(this->ui.doNotInheritClimate,   working.parent.flags, loaded_form_type::parent_flag::use_parent_climate);
   ui::bind_inverse(this->ui.doNotInheritWaterType, working.parent.flags, loaded_form_type::parent_flag::use_parent_water);
   ui::bind_inverse(this->ui.doNotInheritDefaultHeights, working.parent.flags, loaded_form_type::parent_flag::use_parent_land);
   ui::bind_inverse(this->ui.doNotInheritLODWater, working.parent.flags, loaded_form_type::parent_flag::use_parent_lod);
   ui::bind_inverse(this->ui.doNotInheritMapData, working.parent.flags, loaded_form_type::parent_flag::use_parent_map);

   ui::bind(this->ui.defaultHeightLand,  working.land_data.default_land_height);
   ui::bind(this->ui.defaultHeightWater, working.land_data.default_water_height);
   ui::bind(this->ui.lodWaterHeight, working.lod_water_height);
   ui::bind(this->ui.lodWaterType, working.water_type_lod, working);
   #pragma region Map Data
      this->ui.cloudModel->initializeFrom(working.cloud_model);
      ui::bind(this->ui.mapBoundsX, working.map_data.usable_dimensions.x);
      ui::bind(this->ui.mapBoundsY, working.map_data.usable_dimensions.y);
      ui::bind(this->ui.cameraMinHeight, working.map_data.camera.height_min);
      ui::bind(this->ui.cameraMaxHeight, working.map_data.camera.height_max);
      ui::bind(this->ui.cameraPitch, working.map_data.camera.initial_pitch);
      ui::bind(this->ui.mapCellCoordsStartX, working.map_data.coordinates.northwest.x);
      ui::bind(this->ui.mapCellCoordsStartY, working.map_data.coordinates.northwest.y);
      ui::bind(this->ui.mapCellCoordsEndX,   working.map_data.coordinates.southeast.x);
      ui::bind(this->ui.mapCellCoordsEndY,   working.map_data.coordinates.southeast.y);
   #pragma endregion
   #pragma region World Map Offset Data
      ui::bind(this->ui.mapCellOffsetX, working.map_offset_data.offset.x);
      ui::bind(this->ui.mapCellOffsetY, working.map_offset_data.offset.y);
      ui::bind(this->ui.mapCellOffsetZ, working.map_offset_data.offset.z);
      ui::bind(this->ui.mapScale, working.map_offset_data.scale);
   #pragma endregion
   #pragma region Small World
      ui::bind(this->ui.flagSmallWorld,      working.world_flags, loaded_form_type::world_flag::small_world);
      ui::bind(this->ui.flagFixedDimensions, working.world_flags, loaded_form_type::world_flag::fixed_dimensions);
      ui::bind(this->ui.smallGridCenterX,    working.center_cell_coordinates.x);
      ui::bind(this->ui.smallGridCenterY,    working.center_cell_coordinates.y);
   #pragma endregion
   #pragma region Flags
      ui::bind(this->ui.flagCantFastTravel, working.world_flags, loaded_form_type::world_flag::no_fast_travel);
      ui::bind(this->ui.flagCantWait,       record_flags(),      loaded_form_type::form_flag::cant_wait);
      ui::bind(this->ui.flagNoGrass,        working.world_flags, loaded_form_type::world_flag::no_grass);
      ui::bind(this->ui.flagNoLand,         working.world_flags, loaded_form_type::world_flag::no_land);
      ui::bind(this->ui.flagNoLODWater,     working.world_flags, loaded_form_type::world_flag::no_lod_water);
      ui::bind(this->ui.flagNoSky,          working.world_flags, loaded_form_type::world_flag::no_sky);
   #pragma endregion
   ui::bind(this->ui.encounterZone, working.encounter_zone, working);
   ui::bind(this->ui.lightingTemplate, working.lighting_template, working);
   ui::bind(this->ui.location, working.location, working);
   ui::bind(this->ui.musicType, working.music, working);
   ui::bind(this->ui.canopyShadow, working.tree_canopy_shadow);
   ui::bind(this->ui.distantLODMultSpinbox, working.distant_lod_multiplier);
   ui::bind(this->ui.hdLODDiffuse, working.hd_lod_diffuse_texture);
   ui::bind(this->ui.hdLODNormal,  working.hd_lod_normal_texture);
   ui::bind(this->ui.waterEnvMap,  working.water_environment_map);
}
void FormDialogWorldspace::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   #pragma region Map Data
      this->ui.cloudModel->commitTo(working.cloud_model, working);
   #pragma endregion
   if (!working.parent.form) {
      working.parent.flags = 0;
   }
   if (!(working.world_flags & loaded_form_type::world_flag::small_world)) {
      working.world_flags &= ~loaded_form_type::world_flag::fixed_dimensions;
   }
}

void FormDialogWorldspace::_on_has_parent_changed(bool force) {
   bool has_parent = this->ui.parent->formStub() != nullptr;
   
   // We hook this function up to the parent-picker's "changed" signal 
   // before we bind the form data, which means that we run first. This 
   // means we can easily check what the previous value was without having 
   // to manually track it ourselves.
   if (!force) {
      bool previously_had_parent = this->form->parent.form.get_form_stub() != nullptr;
      if (has_parent == previously_had_parent)
         return;
   }

   if (has_parent) {
      this->ui.flagUseSkyCell->setEnabled(true);
      this->ui.doNotInheritClimate->setEnabled(true);
      this->ui.doNotInheritWaterType->setEnabled(true);
   } else {
      this->ui.flagUseSkyCell->setChecked(false);
      this->ui.flagUseSkyCell->setEnabled(false);

      this->ui.doNotInheritClimate->setChecked(true);
      this->ui.doNotInheritClimate->setEnabled(false);
      this->ui.doNotInheritWaterType->setChecked(true);
      this->ui.doNotInheritWaterType->setEnabled(false);
   }
   {
      auto* groupbox = this->ui.doNotInheritDefaultHeights;
      if (has_parent) {
         groupbox->setCheckable(true);
         groupbox->setTitle(tr("Set Own Default Heights"));
      } else {
         groupbox->setCheckable(false);
         groupbox->setTitle(tr("Default Heights"));
      }
   }
   {
      auto* groupbox = this->ui.doNotInheritLODWater;
      if (has_parent) {
         groupbox->setCheckable(true);
         groupbox->setTitle(tr("Set Own Water LOD"));
      } else {
         groupbox->setCheckable(false);
         groupbox->setTitle(tr("Water LOD"));
      }
   }
   {
      auto* groupbox = this->ui.doNotInheritMapData;
      if (has_parent) {
         groupbox->setCheckable(true);
         groupbox->setTitle(tr("Set Own Map Data"));
      } else {
         groupbox->setCheckable(false);
         groupbox->setTitle(tr("Map Data"));
      }
   }
}