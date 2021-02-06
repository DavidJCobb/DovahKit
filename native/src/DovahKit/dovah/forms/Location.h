#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/keyword_list.h"

class TESPluginRecord;

namespace dovah::loaded_forms {
   class Location : public Form {
      public:
         static constexpr form_type_t form_type = form_type::location;
         Location(const constructor_params& c) : Form(form_type, c) {};

         struct subrecord_flag {
            subrecord_flag() = delete;
            enum type : uint8_t {
               population_is_a = 0x01, // ACPR instead of LCPR?
               unique_is_a     = 0x02, // ACUN instead of LCUN?
               static_is_a     = 0x04, // ACSR instead of LCSR?
               encounter_is_a  = 0x08, // ACEC instead of LCEC?
               enable_is_a     = 0x10, // ACEP instead of LCEP?
               cid_is_a        = 0x20, // ACID instead of LCID?
            };
         };
         using subrecord_flags_t = std::underlying_type_t<subrecord_flag::type>;

         struct PopulationRef {
            form_reference_t actor; // ACHR
            form_reference_t cell_or_world;
            int16_t          grid_y;
            int16_t          grid_x;
         };
         struct UniqueRef {
            form_reference_t actor_base;
            form_reference_t actor;
            form_reference_t location; // usually self
         };
         struct StaticRef {
            form_reference_t ref_type; // LocRefType
            form_reference_t reference;
            form_reference_t cell_or_world;
            uint16_t         unk0C;
            uint16_t         unk0E;
         };
         struct Encounter { // unverified
            form_reference_t worldID;
            std::vector<uint32_t> ints; // actually pairs of (u?)int16_t
         };
         struct EnablePoint { // cell enable point
            form_reference_t actor;
            form_reference_t reference;
            int16_t          grid_y;
            int16_t          grid_x;
         };

         components::keyword_list keywords;
         subrecord_flags_t subrecordFlags = 0; // whether we're using Axxx subrecords or Lxxx subrecords
         std::vector<PopulationRef> population; // ACPR/LCPR
         std::vector<form_reference_t> populationActors; // RCPR // Dawnguard only?
         std::vector<UniqueRef> uniques; // ACUN/LCUN
         std::vector<form_reference_t> uniques_R; // RCUN
         std::vector<StaticRef> statics; // ACSR/LCSR
         std::vector<Encounter> encounters; // ACEC/LCEC
         std::vector<EnablePoint> enablePoints; // ACEP/LCEP
         std::vector<form_reference_t> acid; // ACID/LCID - xEdit says "cell marker reference?"
         localized_string name; // FULL
         form_reference_t parent_location; // PNAM
         form_reference_t music; // NAM1
         form_reference_t unreported_crime_faction; // FNAM
         form_reference_t marker; // MNAM
         float            radius; // RNAM
         form_reference_t horse_marker; // NAM0
         struct { // VERIFY ME
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t alpha; // unused
         } color; // CNAM

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
   };
}