#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

class TESPluginRecord;

namespace dovah::loaded_forms {
   class Location : public Form {
      public:
         static constexpr form_type_t form_type = form_type::location;
         Location() : Form(form_type) {};

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
            form_id_t actorRefID;
            form_id_t cellOrWorldID;
            int16_t   gridY;
            int16_t   gridX;
         };
         struct UniqueRef {
            form_id_t actorBaseID;
            form_id_t actorRefID;
            form_id_t locationID; // usually self
         };
         struct StaticRef {
            form_id_t locRefTypeID;
            form_id_t refID;
            form_id_t cellOrWorldID;
            uint16_t  unk0C;
            uint16_t  unk0E;
         };
         struct Encounter { // unverified
            form_id_t worldID;
            std::vector<uint32_t> ints; // actually pairs of (u?)int16_t
         };
         struct EnablePoint { // cell enable point
            form_id_t actorID;
            form_id_t refID;
            int16_t   gridY;
            int16_t   gridX;
         };

         subrecord_flags_t subrecordFlags = 0; // whether we're using Axxx subrecords or Lxxx subrecords
         std::vector<PopulationRef> population; // ACPR/LCPR
         std::vector<form_id_t> populationActors; // RCPR // Dawnguard only?
         std::vector<UniqueRef> uniques; // ACUN/LCUN
         std::vector<form_id_t> uniques_R; // RCUN
         std::vector<StaticRef> statics; // ACSR/LCSR
         std::vector<Encounter> encounters; // ACEC/LCEC
         std::vector<EnablePoint> enablePoints; // ACEP/LCEP
         std::vector<form_id_t> acid; // ACID/LCID - xEdit says "cell marker reference?"
         localized_string name; // FULL
         std::vector<form_id_t> keywords; // KSIZ+KWDA[]
         form_id_t parentLocationID; // PNAM
         form_id_t musicID; // NAM1
         form_id_t unreportedCrimeFactionID; // FNAM
         form_id_t marker; // MNAM
         float radius; // RNAM
         form_id_t horseMarkerRefID; // NAM0
         struct { // VERIFY ME
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t alpha; // unused
         } color; // CNAM

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}