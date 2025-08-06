#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "./Form.h"
#include "./_common.h"
#include "./components/bounds.h"
#include "./components/papyrus.h"
#include "helpers/vector3.h"
#include "./structs/navmesh_pathing_cell.h"

namespace dovah::loaded_forms {
   class Navmesh : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::navmesh;
         Navmesh(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               initially_disabled = 1 << 11,
               compressed         = 1 << 18,
               auto_generated     = 1 << 26,
               navmesh_gen_cell   = 1 << 31,
            };
         };

         struct door_link {
            int16_t  triangle = -1;
            uint32_t crc      =  0;
            form_reference_t door_ref;
         };

         struct edge_link {
            public:
               enum class type : uint32_t {
                  portal,
                  ledge_up,
                  ledge_down,
                  enable_disable_portal,
               };

            public:
               enum type        type = {};
               form_reference_t navmesh;
               int16_t          triangle = -1;
         };

         class cover_info {
            public:
               enum class edge {
                  a, edge_0_1 = a,
                  b, edge_1_2 = b,
               };
               enum class side {
                  left  = 0,
                  right = 1,
               };
               enum class type {
                  uncovered_open_edge,
                  uncovered_wall,
                  covered_ledge,
                  unused,
                  covered_wall,
               };

            public:
               uint16_t raw = 0;
               //
               // Bit pattern:
               // 
               //     | Edge B <- | -> Edge A
               // A.. |    RLHHHH | RLHHHH
               // 
               //    A = Auto-generated
               //    H = Type and height
               //    L = Left side
               //    R = Right side
               //    . = Unused bit
               //

            public:
               constexpr type get_type(edge e) const noexcept {
                  uint8_t v = (e == edge::b) ? (this->raw >> 6) : this->raw;
                  if (v > 0b0011)
                     return type::covered_wall;
                  return (type)v;
               }
               constexpr void set_type(edge e, type t) {
                  if (this->get_type(e) == t)
                     return;
                  uint8_t bits = (uint8_t)t;
                  uint8_t mask = 0b1111;
                  if (e == edge::b) {
                     bits <<= 6;
                     mask <<= 6;
                  }
                  this->raw &= ~mask;
                  this->raw |= bits;
               }

               constexpr uint8_t get_height(edge e) const noexcept {
                  uint8_t v = (e == edge::b) ? this->raw >> 6 : this->raw;
                  v &= 0b1111;
                  if (v < 0b0100)
                     return 0;
                  return (v - 0b0100) * 16 + 64;
               }
               constexpr void set_height(edge e, uint8_t h) {
                  h -= 64;
                  h /= 16;
                  h &= 0b1111;
                  h += 0b0100;
                  if (e == edge::a) {
                     this->raw &= ~0b1111;
                     this->raw |= h;
                  } else {
                     this->raw &= ~(0b1111 << 6);
                     this->raw |= h << 6;
                  }
               }

               constexpr bool is_covered_on_side(edge e, side s) const noexcept {
                  uint16_t mask = 1 << 4;
                  if (e == edge::b)
                     mask <<= 6;
                  if (s == side::right)
                     mask <<= 1;
                  return this->raw & mask;
               }
               constexpr void set_covered_on_side(edge e, side s, bool covered) {
                  uint16_t mask = 1 << 4;
                  if (e == edge::b)
                     mask <<= 6;
                  if (s == side::right)
                     mask <<= 1;

                  if (covered)
                     this->raw |= mask;
                  else
                     this->raw &= ~mask;
               }

               constexpr bool is_auto_generated() const noexcept {
                  return this->raw & (1 << 14);
               }
               constexpr bool set_auto_generated(bool v) noexcept {
                  if (v)
                     this->raw |= (1 << 14);
                  else
                     this->raw &= ~(1 << 14);
               }
         };

         struct triangle {
            public:
               struct flag {
                  enum type : uint16_t {
                     edge_0_1_link      = 1 <<  0,
                     edge_1_2_link      = 1 <<  1,
                     edge_2_0_link      = 1 <<  2,
                     deleted            = 1 <<  3,
                     no_large_creatures = 1 <<  4,
                     overlapping        = 1 <<  5,
                     preferred          = 1 <<  6,
                     is_water           = 1 <<  9,
                     has_load_door      = 1 << 10,
                     found              = 1 << 11,
                  };
               };
               using flags_t = std::underlying_type_t<flag::type>;

            public:
               std::array<uint16_t, 3> vertices;
               std::array<int16_t, 3> edges;
               flags_t    flags = 0;
               cover_info cover;
         };

      public:
         components::object_bounds bounds; // OBND
         components::papyrus_attachment_data script_data; // VMAD
         //
         struct {
            uint32_t version = 12;
            structs::navmesh_pathing_cell pathing_cell;
            std::vector<cobb::vector3<float>> vertices;
            std::vector<triangle>  triangles;
            std::vector<edge_link> edge_links;
            std::vector<door_link> door_links;
            std::vector<int16_t>   cover_triangles;
            struct {
               uint32_t divisor = 1;
               struct {
                  float x = 0;
                  float y = 0;
               } size;
               struct {
                  cobb::vector3<float> min;
                  cobb::vector3<float> max;
               } bounds;
               std::vector<std::vector<int16_t>> triangles_by_grid_cell;
            } navmesh_grid;
         } geometry; // NVNM
         std::vector<form_reference_t> base_objects; // ONAM[]
         std::vector<uint16_t> preferred_connectors; // PNAM[]
         std::vector<uint16_t> non_connectors;       // NNAM[]

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}