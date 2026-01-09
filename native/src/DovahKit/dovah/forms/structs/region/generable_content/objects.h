#pragma once
#include <cstdint>
#include <vector>
#include "../../../../form_reference_t.h"

namespace dovah::loaded_forms::structs::region::generable_content {
   struct object_params {
      public:
         struct flag {
            flag() = delete;
            enum type : uint8_t {
               conform_to_slope     = 1 << 0,
               paint_vertices       = 1 << 1,
               size_variance_signed = 1 << 2,
               angle_x_range_signed = 1 << 3,
               angle_y_range_signed = 1 << 4,
               angle_z_range_signed = 1 << 5,
               tree                 = 1 << 6,
               huge_rock            = 1 << 7,
            };
         };

      public:
         float   density    = 30.0F; // valid range is [0.0F, 100.0F]
         uint8_t clustering =  0;
         struct {
            uint8_t min =  0; // must not be greater than 90. must not be lower than the parent object's min slope.
            uint8_t max = 90; // must not be greater than 90. must not be greater than the parent object's max slope.
         } slope;
         uint8_t  flags = 0;
         uint16_t radius_wrt_parent = 512; // 10
         uint16_t radius = 0;              // 12
         struct {
            float min = 0;    // 14
            float max = 2e05; // 18
         } height;
         struct {
            float base     = 0; // 1C
            float variance = 0; // 20
         } sink;
         float size_variance = 0; // 24
         struct {
            int16_t x = 0; // 28 // valid range is [-180, 180]
            int16_t y = 0; // 2A // valid range is [-180, 180]
            int16_t z = 0; // 2C // valid range is [-180, 180]
         } angle_variance;
         uint16_t unk2E; // 2E
         struct {
            struct {
               uint8_t r = 0; // 30
               uint8_t g = 0; // 31
               uint8_t b = 0; // 32
            } color;
            uint8_t radius_percent; // 33 // stored directly: 52 is 52% of the radius
         } paint_vertices;
   };

   class raw_object_collection {
      public:
         struct raw_object {
            form_reference_t form;
            int16_t          parent_index = -1;
            object_params    params;
         };

      public:
         std::vector<raw_object> objects;
         
      public:
         void clear(dovah::loaded_forms::Form& my_containing_form);
         void clone_from(dovah::loaded_forms::Form& my_containing_form, const raw_object_collection&);
         void sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub&);

         void copy_insert_from(dovah::loaded_forms::Form& my_containing_form, const raw_object_collection&);
   };
}