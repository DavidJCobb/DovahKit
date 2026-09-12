#pragma once
#include <cstdint>
#include <string>
#include "./_base.h"
#include "dovah/forms/Worldspace.h"
#include "ui_worldspace.h"
#include "ui/types/nif_for_form.h"
class DKFormPickerExcludeSingleFormFilter;

class FormDialogWorldspace :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Worldspace, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogWorldspace(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogWorldspace ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
      } filters;
      struct {
         struct {
            bool cant_wait           = false;
            bool fixed_dimensions    = false;
            bool no_fast_travel      = false;
            bool no_grass            = false;
            bool no_land             = false;
            bool no_lod_water        = false;
            bool no_sky              = false;
            bool small_world         = false;
            bool use_parent_climate  = false;
            bool use_parent_land     = false;
            bool use_parent_lod      = false;
            bool use_parent_map      = false;
            bool use_parent_sky_cell = false;
            bool use_parent_water    = false;
         } flags;
         //
         struct {
            float land  = 0;
            float water = 0;
         } default_heights;
         float distant_lod_multiplier = 0;
         dovah::form_stub* encounter_zone = nullptr;
         struct {
            std::string diffuse;
            std::string normal;
         } hd_lod_textures;
         dovah::form_stub* lighting_template = nullptr;
         dovah::form_stub* location          = nullptr;
         struct {
            float             height = 0;
            dovah::form_stub* type   = nullptr;
         } lod_water;
         struct {
            struct {
               int32_t x = 0;
               int32_t y = 0;
            } bounds;
            struct {
               struct {
                  float min = 0;
                  float max = 0;
               } height;
               float pitch = 0;
            } camera;
            struct {
               struct {
                  int16_t x = 0;
                  int16_t y = 0;
               } nw;
               struct {
                  int16_t x = 0;
                  int16_t y = 0;
               } se;
            } cell_coords;
            ui::types::nif_for_form cloud_model;
            struct {
               cobb::vector3<float> offset = { 0, 0, 0 };
               float scale = 1.0F;
            } offset_data;
         } map;
         dovah::form_stub* music_type = nullptr;
         dovah::form_stub* parent     = nullptr;
         struct {
            int16_t x = 0;
            int16_t y = 0;
         } small_world_center;
         std::string tree_canopy_shadow;
         std::string water_environment_map;
      } working;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _on_has_parent_changed(bool force = false);
};
