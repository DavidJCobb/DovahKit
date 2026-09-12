#include "./worldspace.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

/*
   Most form-editing dialogs at this point use the "working copy" system. 
   We deliberately avoid doing that here in order to avoid the overhead 
   of copying WRLD/RNAM (the "large ref" data), since there's so MUCH of 
   it on [WRLD:0000003C]Tamriel.
*/

#define FOR_EACH_WORLDSPACE_PARENT_FLAG(X) \
   X(use_parent_climate) \
   X(use_parent_land) \
   X(use_parent_lod) \
   X(use_parent_map) \
   X(use_parent_sky_cell) \
   X(use_parent_water)

#define FOR_EACH_WORLDSPACE_FLAG(X) \
   X(fixed_dimensions) \
   X(no_fast_travel) \
   X(no_grass) \
   X(no_land) \
   X(no_lod_water) \
   X(no_sky) \
   X(small_world)

#define FOR_EACH_WORLDSPACE_FORM_FLAG(X) \
   X(cant_wait)

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

   #pragma region Pull form data to UI
   {
      auto& src = *this->form;
      auto& dst = this->working;
      this->working = {
         .flags = {},
         //
         .default_heights = {
            .land  = src.land_data.default_land_height,
            .water = src.land_data.default_water_height,
         },
         .distant_lod_multiplier = src.distant_lod_multiplier,
         .encounter_zone         = src.encounter_zone.get_form_stub(),
         .hd_lod_textures        = {
            .diffuse = src.hd_lod_diffuse_texture,
            .normal  = src.hd_lod_normal_texture,
         },
         .lighting_template = src.lighting_template.get_form_stub(),
         .location          = src.location.get_form_stub(),
         .lod_water         = {
            .height = src.lod_water_height,
            .type   = src.water_type_lod.get_form_stub(),
         },
         .map = {
            .bounds = {
               .x = src.map_data.usable_dimensions.x,
               .y = src.map_data.usable_dimensions.y,
            },
            .camera = {
               .height = {
                  .min = src.map_data.camera.height_min,
                  .max = src.map_data.camera.height_max,
               },
               .pitch = src.map_data.camera.initial_pitch,
            },
            .cell_coords = {
               .nw = {
                  .x = src.map_data.coordinates.northwest.x,
                  .y = src.map_data.coordinates.northwest.y
               },
               .se = {
                  .x = src.map_data.coordinates.southeast.x,
                  .y = src.map_data.coordinates.southeast.y
               },
            },
            // cloud_model
            .offset_data = {
               .offset = src.map_offset_data.offset,
               .scale  = src.map_offset_data.scale,
            },
         },
         .music_type         = src.music.get_form_stub(),
         .parent             = src.parent.form.get_form_stub(),
         .small_world_center = {
            .x = src.center_cell_coordinates.x,
            .y = src.center_cell_coordinates.y,
         },
         .tree_canopy_shadow    = src.tree_canopy_shadow,
         .water_environment_map = src.water_environment_map,
      };
      //
      dst.map.cloud_model.initializeFrom(src.cloud_model);
      #define X(_flag_name, ...) dst.flags._flag_name = !!(src.parent.flags & loaded_form_type::parent_flag::_flag_name);
      FOR_EACH_WORLDSPACE_PARENT_FLAG(X)
      #undef X
      //
      #define X(_flag_name, ...) dst.flags._flag_name = !!(src.world_flags & loaded_form_type::world_flag::_flag_name);
      FOR_EACH_WORLDSPACE_FLAG(X)
      #undef X
      //
      #define X(_flag_name, ...) dst.flags._flag_name = src.stub.test_record_flags(loaded_form_type::form_flag::_flag_name);
      FOR_EACH_WORLDSPACE_FORM_FLAG(X)
      #undef X
   }
   #pragma endregion

   this->filters.exclude_self->set_exclusion(&this->form->stub);

   auto& working = this->working;
   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.parent, working.parent);

   ui::bind(this->ui.flagUseSkyCell, working.flags.use_parent_sky_cell);
   ui::bind_inverse(this->ui.doNotInheritClimate,   working.flags.use_parent_climate);
   ui::bind_inverse(this->ui.doNotInheritWaterType, working.flags.use_parent_water);
   ui::bind_inverse(this->ui.doNotInheritDefaultHeights, working.flags.use_parent_land);
   ui::bind_inverse(this->ui.doNotInheritLODWater, working.flags.use_parent_lod);
   ui::bind_inverse(this->ui.doNotInheritMapData, working.flags.use_parent_map);

   ui::bind(this->ui.defaultHeightLand,  working.default_heights.land);
   ui::bind(this->ui.defaultHeightWater, working.default_heights.water);
   ui::bind(this->ui.lodWaterHeight, working.lod_water.height);
   ui::bind(this->ui.lodWaterType, working.lod_water.type);
   #pragma region Map Data
      this->ui.cloudModel->setValue(working.map.cloud_model);
      ui::bind(this->ui.mapBoundsX, working.map.bounds.x);
      ui::bind(this->ui.mapBoundsY, working.map.bounds.y);
      ui::bind(this->ui.cameraMinHeight, working.map.camera.height.min);
      ui::bind(this->ui.cameraMaxHeight, working.map.camera.height.max);
      ui::bind(this->ui.cameraPitch, working.map.camera.pitch);
      ui::bind(this->ui.mapCellCoordsStartX, working.map.cell_coords.nw.x);
      ui::bind(this->ui.mapCellCoordsStartY, working.map.cell_coords.nw.y);
      ui::bind(this->ui.mapCellCoordsEndX,   working.map.cell_coords.se.x);
      ui::bind(this->ui.mapCellCoordsEndY,   working.map.cell_coords.se.y);
   #pragma endregion
   #pragma region World Map Offset Data
      ui::bind(this->ui.mapCellOffsetX, working.map.offset_data.offset.x);
      ui::bind(this->ui.mapCellOffsetY, working.map.offset_data.offset.y);
      ui::bind(this->ui.mapCellOffsetZ, working.map.offset_data.offset.z);
      ui::bind(this->ui.mapScale, working.map.offset_data.scale);
   #pragma endregion
   #pragma region Small World
      ui::bind(this->ui.flagSmallWorld,      working.flags.small_world);
      ui::bind(this->ui.flagFixedDimensions, working.flags.fixed_dimensions);
      ui::bind(this->ui.smallGridCenterX,    working.small_world_center.x);
      ui::bind(this->ui.smallGridCenterY,    working.small_world_center.y);
   #pragma endregion
   #pragma region Flags
      ui::bind(this->ui.flagCantFastTravel, working.flags.no_fast_travel);
      ui::bind(this->ui.flagCantWait,       working.flags.cant_wait);
      ui::bind(this->ui.flagNoGrass,        working.flags.no_grass);
      ui::bind(this->ui.flagNoLand,         working.flags.no_land);
      ui::bind(this->ui.flagNoLODWater,     working.flags.no_lod_water);
      ui::bind(this->ui.flagNoSky,          working.flags.no_sky);
   #pragma endregion
   ui::bind(this->ui.encounterZone, working.encounter_zone);
   ui::bind(this->ui.lightingTemplate, working.lighting_template);
   ui::bind(this->ui.location, working.location);
   ui::bind(this->ui.musicType, working.music_type);
   ui::bind(this->ui.canopyShadow, working.tree_canopy_shadow);
   ui::bind(this->ui.distantLODMultSpinbox, working.distant_lod_multiplier);
   ui::bind(this->ui.hdLODDiffuse, working.hd_lod_textures.diffuse);
   ui::bind(this->ui.hdLODNormal,  working.hd_lod_textures.normal);
   ui::bind(this->ui.waterEnvMap,  working.water_environment_map);
}
void FormDialogWorldspace::_save_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = this->working;
   //
   // Copy stuff that wasn't ui::bind'd to the working data.
   //
   #pragma region Map Data
      working.map.cloud_model = this->ui.cloudModel->value();
   #pragma endregion
   if (!working.parent) {
      #define X(_flag_name, ...) working.flags._flag_name = false;
      FOR_EACH_WORLDSPACE_PARENT_FLAG(X)
      #undef X
   }
   if (!(working.flags.small_world)) {
      working.flags.fixed_dimensions = false;
   }
   
   #pragma region Push UI data to form
   {
      const auto& src = working;
      auto& dst = *this->form;
      
      #pragma region flags
         {
            decltype(dst.parent.flags) v = 0;
            #define X(_flag_name, ...) if (src.flags._flag_name) v |= loaded_form_type::parent_flag::_flag_name;
            FOR_EACH_WORLDSPACE_PARENT_FLAG(X)
            #undef X
            dst.parent.flags = v;
         }
         {
            decltype(dst.world_flags) v = 0;
            #define X(_flag_name, ...) if (src.flags._flag_name) v |= loaded_form_type::world_flag::_flag_name;
            FOR_EACH_WORLDSPACE_FLAG(X)
            #undef X
            dst.world_flags = v;
         }
         #define X(_flag_name, ...) dst.stub.edit_record_flags(loaded_form_type::form_flag::_flag_name, src.flags._flag_name);
         FOR_EACH_WORLDSPACE_FORM_FLAG(X)
         #undef X
      #pragma endregion

      src.map.cloud_model.commitTo(dst.cloud_model, dst);

      dst.land_data.default_land_height  = src.default_heights.land;
      dst.land_data.default_water_height = src.default_heights.water;
      dst.distant_lod_multiplier         = src.distant_lod_multiplier;
      dst.encounter_zone.set(dst, src.encounter_zone);
      dst.hd_lod_diffuse_texture = src.hd_lod_textures.diffuse;
      dst.hd_lod_normal_texture  = src.hd_lod_textures.normal;
      dst.lighting_template.set(dst, src.lighting_template);
      dst.location.set(dst, src.location);
      dst.lod_water_height = src.lod_water.height;
      dst.water_type_lod.set(dst, src.lod_water.type);
      dst.map_data = {
         .usable_dimensions = {
            src.map.bounds.x,
            src.map.bounds.y,
         },
         .coordinates = {
            .northwest = {
               .x = src.map.cell_coords.nw.x,
               .y = src.map.cell_coords.nw.y,
            },
            .southeast = {
               .x = src.map.cell_coords.se.x,
               .y = src.map.cell_coords.se.y,
            },
         },
         .camera = {
            .height_min    = src.map.camera.height.min,
            .height_max    = src.map.camera.height.max,
            .initial_pitch = src.map.camera.pitch,
         },
      };
      dst.map_offset_data = {
         .scale  = src.map.offset_data.scale,
         .offset = src.map.offset_data.offset,
      };
      dst.music.set(dst, src.music_type);
      dst.parent.form.set(dst, src.parent);
      dst.center_cell_coordinates = {
         .x = src.small_world_center.x,
         .y = src.small_world_center.y,
      };
      dst.tree_canopy_shadow    = src.tree_canopy_shadow;
      dst.water_environment_map = src.water_environment_map;
   }
   #pragma endregion
}

void FormDialogWorldspace::_on_has_parent_changed(bool force) {
   bool has_parent = this->ui.parent->formStub() != nullptr;
   
   // We hook this function up to the parent-picker's "changed" signal 
   // before we bind the form data, which means that we run first. This 
   // means we can easily check what the previous value was without having 
   // to manually track it ourselves.
   if (!force) {
      bool previously_had_parent = this->working.parent != nullptr;
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