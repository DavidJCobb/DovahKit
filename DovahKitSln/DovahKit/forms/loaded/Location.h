#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "../components.h"

class TESPluginRecord;

SCOPE_ENUM(location_subrecord_flags, enum location_subrecord_flags : uint8_t {
   population_is_a = 1, SCOPED_ENUM_COMMENT("ACPR instead of LCPR?")
   unique_is_a     = 2, SCOPED_ENUM_COMMENT("ACUN instead of LCUN?")
   static_is_a     = 4, SCOPED_ENUM_COMMENT("ACSR instead of LCSR?")
   encounter_is_a  = 8, SCOPED_ENUM_COMMENT("ACEC instead of LCEC?")
   enable_is_a     = 16, SCOPED_ENUM_COMMENT("ACEP instead of LCEP?")
   cid_is_a        = 32, SCOPED_ENUM_COMMENT("ACID instead of LCID?")
});
namespace LoadedForms {
   class Location : public Form {
      public:
         Location() : Form(FormType::Location) {};

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

         uint16_t subrecordFlags = 0; // whether we're using Axxx subrecords or Lxxx subrecords
         std::vector<PopulationRef> population; // ACPR/LCPR
         std::vector<form_id_t> populationActors; // RCPR // Dawnguard only?
         std::vector<UniqueRef> uniques; // ACUN/LCUN
         std::vector<form_id_t> uniques_R; // RCUN
         std::vector<StaticRef> statics; // ACSR/LCSR
         std::vector<Encounter> encounters; // ACEC/LCEC
         std::vector<EnablePoint> enablePoints; // ACEP/LCEP
         std::vector<form_id_t> acid; // ACID/LCID - xEdit says "cell marker reference?"
         LStringRef name; // FULL
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

         void load(TESPluginRecord&);
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}