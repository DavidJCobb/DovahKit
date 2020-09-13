#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class MagicEffect : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         MagicEffect() : Form(form_type::magic_effect) {};

         void load(tes_record_reader&); // TODO: FINISH ME
         static void generateUseInfo(tes_record_reader&, form_stub*);

         enum class archetype_t : uint32_t {
            value_modifier       =  0,
            script               =  1,
            dispel               =  2,
            cure_disease         =  3,
            absorb               =  4,
            dual_value_modifier  =  5,
            calm                 =  6,
            demoralize           =  7,
            frenzy               =  8,
            disarm               =  9,
            command_summoned     = 10,
            invisibility         = 11,
            light                = 12,
            lock                 = 15,
            open                 = 16,
            bound_weapon         = 17,
            summon_creature      = 18,
            detect_life          = 19,
            telekinesis          = 20,
            paralysis            = 21,
            reanimate            = 22,
            soul_trap            = 23,
            turn_undead          = 24,
            guide                = 25,
            werewolf_feed        = 26,
            cure_paralysis       = 27,
            cure_addiction       = 28,
            cure_poison          = 29,
            concussion           = 30,
            value_and_parts      = 31,
            accumulate_magnitude = 32,
            stagger              = 33,
            peak_value_modifier  = 34,
            cloak                = 35,
            werewolf             = 36,
            slow_time            = 37,
            rally                = 38,
            enchant_weapon       = 39,
            spawn_hazard         = 40,
            etherealize          = 41,
            banish               = 42,
            spawn_scripted_ref   = 43,
            disguise             = 44,
            grab_actor           = 45,
            vampire_lord         = 46
         };
   };
}