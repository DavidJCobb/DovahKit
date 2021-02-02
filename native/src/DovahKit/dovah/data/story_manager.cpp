#include "story_manager.h"

namespace dovah {
   std::array<story_event_definition, 31> story_event_definition::list = {
      story_event_definition{
         story_event_code::actor_dialogue,
         {{
            story_event_definition::member{ 'L1', "Location", "Location" },
            story_event_definition::member{ 'R1', "Actor 1",  "Actor 2" },
            story_event_definition::member{ 'R2', "Actor 1",  "Actor 2" },
         }}
      },
      //
      story_event_definition{
         story_event_code::actor_hello,
         {{
            story_event_definition::member{ 'L1', "Location", "Location" },
            story_event_definition::member{ 'R1', "Actor 1",  "Actor 2" },
            story_event_definition::member{ 'R2', "Actor 1",  "Actor 2" },
         }}
      },
      //
      story_event_definition{
         story_event_code::arrest,
         {{
            story_event_definition::member{ 'L1', "Location",       "Location" },
            story_event_definition::member{ 'R1', "ArestingGuard",  "Arresting Guard" },
            story_event_definition::member{ 'R2', "Criminal",       "Criminal" },
            story_event_definition::member{ 'V1', "iCrimeType",     "Crime Type" },
         }}
      },
      //
      story_event_definition{
         story_event_code::assault,
         {{
            story_event_definition::member{ 'L1', "Location", "Location" },
            story_event_definition::member{ 'R1', "Victim",   "Victim" },
            story_event_definition::member{ 'R2', "Attacker", "Attacker" },
            story_event_definition::member{ 'V1', "Crime",    "Crime" },
         }}
      },
      //
      story_event_definition{
         story_event_code::bribe,
         {{
            story_event_definition::member{ 'R1', "Actor",      "Actor" },
            story_event_definition::member{ 'V1', "Gold Value", "Gold Value" },
         }}
      },
      //
      story_event_definition{
         story_event_code::cast_magic,
         {{
            story_event_definition::member{ 'R1', "CastingActor", "Caster" },
            story_event_definition::member{ 'R2', "SpellTarget",  "Target" },
            story_event_definition::member{ 'L1', "Location",     "Location" },
            story_event_definition::member{ 'F1', "SpellForm",    "Spell" },
         }}
      },
      //
      story_event_definition{
         story_event_code::change_location,
         {{
            story_event_definition::member{ 'R1', "Actor",        "Actor" },
            story_event_definition::member{ 'L1', "Old Location", "Old Location" },
            story_event_definition::member{ 'L2', "New Location", "New Location" },
         }}
      },
      //
      story_event_definition{
         story_event_code::change_relationship_rank,
         {{
            story_event_definition::member{ 'R1', "NPC 1",            "NPC 1" },
            story_event_definition::member{ 'R2', "NPC 2",            "NPC 2" },
            story_event_definition::member{ 'V1', "Old Relationship", "Old Relationship" },
            story_event_definition::member{ 'V2', "New Relationship", "New Relationship" },
         }}
      },
      //
      story_event_definition{
         story_event_code::craft_item,
         {{
            story_event_definition::member{ 'R1', "Workbench",      "Workbench" },
            story_event_definition::member{ 'L1', "Bench Location", "Bench Location" },
            story_event_definition::member{ 'O1', "CreatedObject",  "Created Object" },
         }}
      },
      //
      story_event_definition{
         story_event_code::crime_gold,
         {{
            story_event_definition::member{ 'R1', "Victim",        "Victim" },
            story_event_definition::member{ 'R2', "Criminal",      "Criminal" },
            story_event_definition::member{ 'F1', "Crime Faction", "Crime Faction" },
            story_event_definition::member{ 'V1', "Gold Value",    "Gold Value" },
            story_event_definition::member{ 'V2', "iCrimeType",    "Crime" },
         }}
      },
      //
      story_event_definition{
         story_event_code::dead_body,
         {{
            story_event_definition::member{ 'R1', "Actor",      "Actor" },
            story_event_definition::member{ 'R2', "Dead Actor", "Dead Actor" },
            story_event_definition::member{ 'L1', "Location",   "Location" },
         }}
      },
      //
      story_event_definition{
         story_event_code::escaped_jail,
         {{
            story_event_definition::member{ 'L1', "Location",    "Location" },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group" },
         }}
      },
      //
      story_event_definition{
         story_event_code::flatter,
         {{
            story_event_definition::member{ 'R1', "Actor", "Actor" },
         }}
      },
      //
      story_event_definition{
         story_event_code::level_up,
         {{
            story_event_definition::member{ 'V1', "New Level", "New Level" },
         }}
      },
      //
      story_event_definition{
         story_event_code::intimidate,
         {{
            story_event_definition::member{ 'R1', "Actor", "Actor" },
         }}
      },
      //
      story_event_definition{
         story_event_code::jail,
         {{
            story_event_definition::member{ 'R1', "hGuard",      "Guard" },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group" },
            story_event_definition::member{ 'L1', "Location",    "Location" },
            story_event_definition::member{ 'V1', "Crime Gold",  "Crime Gold" },
         }}
      },
      //
      story_event_definition{
         story_event_code::kill,
         {{
            story_event_definition::member{ 'R1', "Victim",       "Victim" },
            story_event_definition::member{ 'R2', "Killer",       "Killer" },
            story_event_definition::member{ 'L1', "Location",     "Location" },
            story_event_definition::member{ 'V1', "Crime Status", "Crime Status" },
            story_event_definition::member{ 'V1', "Relationship Rank to Killer before Death", "Relationship" },
         }}
      },
      //
      story_event_definition{
         story_event_code::lockpick,
         {{
            story_event_definition::member{ 'R1', "Actor",       "Actor" },
            story_event_definition::member{ 'R2', "Lock Object", "Locked Object" },
         }}
      },
      //
      story_event_definition{
         story_event_code::new_voice_power,
         {{
            story_event_definition::member{ 'R1', "Actor",       "Actor" },
            story_event_definition::member{ 'F1', "Voice Power", "Voice Power" },
         }}
      },
      //
      story_event_definition{
         story_event_code::pay_fine,
         {{
            story_event_definition::member{ 'R1', "hCriminal",   "Criminal" },
            story_event_definition::member{ 'R2', "hGuard",      "Guard" },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group" },
            story_event_definition::member{ 'V1', "Crime Gold",  "Crime Gold" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_activate_actor,
         {{
            story_event_definition::member{ 'L1', "Location", "Location" },
            story_event_definition::member{ 'R1', "Actor ",   "Actor" }, // the space in the Bethesda name is not a typo
         }}
      },
      //
      story_event_definition{
         story_event_code::player_add_item,
         {{
            story_event_definition::member{ 'R1', "OwnerRef",          "Owner" },
            story_event_definition::member{ 'R2', "OriginalContainer", "Container" },
            story_event_definition::member{ 'L1', "Location",          "Location" },
            story_event_definition::member{ 'F1', "ObjectForm",        "Item Base Form" },
            story_event_definition::member{ 'V1', "AquireType",        "Acquire Type" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_cured,
         {{
            story_event_definition::member{ 'F1', "Disease", "Disease" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_infected,
         {{
            story_event_definition::member{ 'R1', "Transmitting Actor", "Transmitting Actor" },
            story_event_definition::member{ 'F1', "Disease",            "Disease" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_receives_favor,
         {{
            story_event_definition::member{ 'R1', "Actor", "Actor" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_remove_item,
         {{
            story_event_definition::member{ 'R1', "OwnerRef",   "Owner" },
            story_event_definition::member{ 'R2', "ItemRef",    "Item In-World Ref" },
            story_event_definition::member{ 'L1', "Location",   "Location" },
            story_event_definition::member{ 'F1', "ObjectForm", "Item Base Form" },
            story_event_definition::member{ 'V1', "RemoveType", "Remove Type" },
         }}
      },
      //
      story_event_definition{
         story_event_code::quest_start,
         {{
            story_event_definition::member{ 'Q1', "Quest", "Quest" },
         }}
      },
      //
      story_event_definition{
         story_event_code::script,
         {{
            story_event_definition::member{ 'L1', "Location", "Location" },
            story_event_definition::member{ 'R1', "Ref 1",    "Ref 1" },
            story_event_definition::member{ 'R2', "Ref 2",    "Ref 2" },
            story_event_definition::member{ 'V1', "Value 1",  "Value 1" },
            story_event_definition::member{ 'V2', "Value 2",  "Value 2" },
         }}
      },
      //
      story_event_definition{
         story_event_code::served_time_in_jail,
         {{
            story_event_definition::member{ 'L1', "Location",    "Location" },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group" },
            story_event_definition::member{ 'V1', "Crime Gold",  "Crime Gold" },
            story_event_definition::member{ 'V2', "Days Jail",   "Days In Jail" },
         }}
      },
      //
      story_event_definition{
         story_event_code::skill_increase,
         {{
            story_event_definition::member{ 'V1', "Skill", "Skill" },
         }}
      },
      //
      story_event_definition{
         story_event_code::trespass,
         {{
            story_event_definition::member{ 'R1', "Victim",     "Victim" },
            story_event_definition::member{ 'R2', "Trespasser", "Trespasser" },
            story_event_definition::member{ 'L1', "Location",   "Location" },
            story_event_definition::member{ 'V1', "Crime",      "Crime" },
         }}
      },
      //
   };

   /*static*/ story_event_definition* story_event_definition::lookup(story_event_code_t code) {
      for (auto& entry : list)
         if (entry.signature == code)
            return &entry;
      return nullptr;
   }
}