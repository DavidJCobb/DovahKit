#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "../../helpers/vector3.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Worldspace : public Form {
      public:
         static constexpr form_type_t form_type = form_type::worldspace;
         Worldspace() : Form(form_type) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               cant_wait = 0x00080000,
            };
         };

         struct world_flag {
            world_flag() = delete;
            enum : uint8_t {
               small_world      = 0x01,
               no_fast_travel   = 0x02,
               //
               no_lod_water     = 0x08,
               no_land          = 0x10,
               no_sky           = 0x20,
               fixed_dimensions = 0x40,
               no_grass         = 0x80,
            };
         };

         struct parent_flag {
            parent_flag() = delete;
            enum : uint8_t {
               use_parent_land       = 0x01,
               use_parent_lod        = 0x02,
               use_parent_map        = 0x04,
               use_parent_water      = 0x08,
               use_parent_climate    = 0x10,
               use_parent_imagespace = 0x20, // unused
               use_parent_sky_cell   = 0x40,
            };
         };

         struct large_reference_t { // RNAM (SSE-only); one subrecord per entry
            struct ref {
               form_reference_t form;
               int16_t y;
               int16_t x;
            };
            struct entry {
               int16_t y;
               int16_t x;
               std::vector<ref> refs;
            };
            //
            std::vector<entry> entries;
            //
            void clone_from(const large_reference_t& original, form_stub& my_owner) noexcept;
            void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
         };

         struct max_height_data_t {
            struct quad_heights {
               int8_t sw;
               int8_t se;
               int8_t nw;
               int8_t ne;
            };
            //
            struct {
               int16_t x;
               int16_t y;
            } min;
            struct {
               int16_t x;
               int16_t y;
            } max;
            bool present = false;
            std::vector<quad_heights> cells;
         };

         struct offset_data_t {
            //
            // OFST is skipped if it is zero-length. Otherwise, it's a two-dimensional array of 
            // uint32_ts with indices [y][x], where the range for X is defined by (bounds) i.e. 
            // the X-value in NAM9 minus the X-value in NAM0.
            //
            // Skyrim.esm doesn't seem to use this, and a forum post by zilav describes either 
            // OFST or xEdit functionality for working with OFST (his wording is ambiguous) as 
            // a "debugging option." In all honesty we could probably get away with not even 
            // serializing it at all.
            //
            bool present = false;
            std::vector<uint32_t> offsets; // single-dimensional array
         };

         large_reference_t large_references;
         localized_string  name; // FULL
         max_height_data_t max_height_data; // MHDT
         struct {
            int16_t x = 0;
            int16_t y = 0;
         } center_cell_coordinates; // WCTR // for fixed-dimension worldspaces only?
         form_reference_t climate;           // CNAM
         form_reference_t lighting_template; // LTMP
         form_reference_t encounter_zone;    // XEZN // Uses the same signature as an extra-data type, but isn't loaded as extra-data.
         form_reference_t location;          // XLCN // Uses the same signature as an extra-data type, but isn't loaded as extra-data.
         form_reference_t music;             // ZNAM
         form_reference_t water_type;        // NAM2
         form_reference_t water_type_lod;    // NAM3
         float            lod_water_height;  // NAM4
         struct {
            float default_land_height;
            float default_water_height;
         } land_data; // DNAM
         struct {
            form_reference_t form; // WNAM
            uint16_t flags = 0; // PNAM
         } parent;
         std::string map_icon; // ICON
         components::model cloud_model; // MODL and friends
         struct {
            struct {
               int32_t x = 0;
               int32_t y = 0;
            } usable_dimensions;
            struct {
               struct {
                  int16_t x = 0;
                  int16_t y = 0;
               } northwest;
               struct {
                  int16_t x = 0;
                  int16_t y = 0;
               } southeast;
            } coordinates;
            struct {
               float height_min    = 50000.0F;
               float height_max    = 80000.0F;
               float initial_pitch =    50.0F;
            } camera;
         } map_data; // MNAM
         struct {
            //
            // Indicates where and how map markers are displayed relative to 
            // the parent worldspace.
            //
            float scale; // world map scale
            cobb::vector3<float> offset; // offset, measured in world units (i.e. cell grid * 4096)
         } map_offset_data; // ONAM
         float   distant_lod_multiplier = 1.0F; // NAMA
         uint8_t world_flags = 0; // DATA
         struct { // NOTE: huge values can cause performance hits; xEdit warns if any value is outside of +/- 256
            struct {
               float x;
               float y;
            } min; // NAM0
            struct {
               float x;
               float y;
            } max; // NAM9
         } bounds;
         std::string tree_canopy_shadow; // NNAM // unused
         std::string water_noise_texture; // XNAM
         std::string hd_lod_diffuse_texture; // TNAM
         std::string hd_lod_normal_texture; // UNAM
         std::string water_environment_map; // XWEM // Uses the same signature as an extra-data type, but isn't loaded as extra-data.
         offset_data_t offset_data; // OFST
         //
         // Additional content that the game is theoretically capable of identifying, but 
         // that is never generated by official tools:
         //
         bool has_object_bounds = false;
         components::object_bounds object_bounds; // OBND. recognized, but discarded at run-time.
         components::papyrus_attachment_data script_data; // VMAD

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
         //
         virtual void setup(const file_load_order&) noexcept override;
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}