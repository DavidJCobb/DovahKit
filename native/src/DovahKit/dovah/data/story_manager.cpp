#include "story_manager.h"

namespace {
   constexpr std::initializer_list<dovah::form_type_t> reference_types = {};
}

namespace dovah {

   //
   // So Bethesda's system for defining the available form types for event members is kinda scuffed... 
   // They seem to do it based solely on the prefix:
   //
   //    F: Any type usable with the normal GetIsID condition function
   //    L: Locations
   //    O: Any type usable with the normal GetIsID condition function
   //    Q: Quests
   //    R: You cannot use this with GetEventData:GetIsID; this event member is not selectable.
   //    V: Any type usable with the normal GetIsID condition function (even though this isn't a form)
   //
   // As a consequence of this, the Creation Kit allows you to set up GetEventData conditions that 
   // don't make sense, like comparing the spell in a cast-magic event to a Door form. This also means 
   // that the form drop-down when configuring most GetEventData:GetIsID conditions ends up including 
   // a ton of irrelevant forms, which is both an impediment to usage and the cause of a performance 
   // hit.
   //
   // I've decided to instead provide explicit lists of sensible form types whenever possible.
   //

   const std::array<story_event_definition, 31> story_event_definition::list = {
      story_event_definition{
         story_event_code::actor_dialogue,
         {{
            story_event_definition::member{ 'L1', "Location", "Location", { form_type::location } },
            story_event_definition::member{ 'R1', "Actor 1",  "Actor 1",  { form_type::actor } },
            story_event_definition::member{ 'R2', "Actor 2",  "Actor 2",  { form_type::actor } },
         }}
      },
      //
      story_event_definition{
         story_event_code::actor_hello,
         {{
            story_event_definition::member{ 'L1', "Location", "Location", { form_type::location } },
            story_event_definition::member{ 'R1', "Actor 1",  "Actor 1",  { form_type::actor } },
            story_event_definition::member{ 'R2', "Actor 2",  "Actor 2",  { form_type::actor } },
         }}
      },
      //
      story_event_definition{
         story_event_code::arrest,
         {{
            story_event_definition::member{ 'L1', "Location",       "Location",        { form_type::location } },
            story_event_definition::member{ 'R1', "ArestingGuard",  "Arresting Guard", { form_type::actor } },
            story_event_definition::member{ 'R2', "Criminal",       "Criminal",        { form_type::actor } },
            story_event_definition::member{ 'V1', "iCrimeType",     "Crime Type" },
         }}
      },
      //
      story_event_definition{
         story_event_code::assault,
         {{
            story_event_definition::member{ 'L1', "Location", "Location", { form_type::location } },
            story_event_definition::member{ 'R1', "Victim",   "Victim",   { form_type::actor } },
            story_event_definition::member{ 'R2', "Attacker", "Attacker", { form_type::actor } },
            story_event_definition::member{ 'V1', "Crime",    "Crime" },
         }}
      },
      //
      story_event_definition{
         story_event_code::bribe,
         {{
            story_event_definition::member{ 'R1', "Actor",      "Actor", { form_type::actor } },
            story_event_definition::member{ 'V1', "Gold Value", "Gold Value" },
         }}
      },
      //
      story_event_definition{
         story_event_code::cast_magic,
         {{
            story_event_definition::member{ 'R1', "CastingActor", "Caster",   { form_type::actor } },
            story_event_definition::member{ 'R2', "SpellTarget",  "Target",   { form_type::actor } },
            story_event_definition::member{ 'L1', "Location",     "Location", { form_type::location } },
            story_event_definition::member{ 'F1', "SpellForm",    "Spell",    { form_type::spell } },
         }}
      },
      //
      story_event_definition{
         story_event_code::change_location,
         {{
            story_event_definition::member{ 'R1', "Actor",        "Actor",        { form_type::actor } },
            story_event_definition::member{ 'L1', "Old Location", "Old Location", { form_type::location } },
            story_event_definition::member{ 'L2', "New Location", "New Location", { form_type::location } },
         }}
      },
      //
      story_event_definition{
         story_event_code::change_relationship_rank,
         {{
            story_event_definition::member{ 'R1', "NPC 1",            "NPC 1", { form_type::actor } },
            story_event_definition::member{ 'R2', "NPC 2",            "NPC 2", { form_type::actor } },
            story_event_definition::member{ 'V1', "Old Relationship", "Old Relationship" },
            story_event_definition::member{ 'V2', "New Relationship", "New Relationship" },
         }}
      },
      //
      story_event_definition{
         story_event_code::craft_item,
         {{
            story_event_definition::member{ 'R1', "Workbench",      "Workbench",      { form_type::reference } },
            story_event_definition::member{ 'L1', "Bench Location", "Bench Location", { form_type::location } },
            story_event_definition::member{ 'O1', "CreatedObject",  "Created Object", { form_type::none } },
         }}
      },
      //
      story_event_definition{
         story_event_code::crime_gold,
         {{
            story_event_definition::member{ 'R1', "Victim",        "Victim",        { form_type::actor } },
            story_event_definition::member{ 'R2', "Criminal",      "Criminal",      { form_type::actor } },
            story_event_definition::member{ 'F1', "Crime Faction", "Crime Faction", { form_type::faction } },
            story_event_definition::member{ 'V1', "Gold Value",    "Gold Value" },
            story_event_definition::member{ 'V2', "iCrimeType",    "Crime" },
         }}
      },
      //
      story_event_definition{
         story_event_code::dead_body,
         {{
            story_event_definition::member{ 'R1', "Actor",      "Actor",      { form_type::actor } },
            story_event_definition::member{ 'R2', "Dead Actor", "Dead Actor", { form_type::actor } },
            story_event_definition::member{ 'L1', "Location",   "Location",   { form_type::location } },
         }}
      },
      //
      story_event_definition{
         story_event_code::escaped_jail,
         {{
            story_event_definition::member{ 'L1', "Location",    "Location",    { form_type::location } },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group", { form_type::none } }, // is this a faction?
         }}
      },
      //
      story_event_definition{
         story_event_code::flatter,
         {{
            story_event_definition::member{ 'R1', "Actor", "Actor", { form_type::actor } },
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
            story_event_definition::member{ 'R1', "Actor", "Actor", { form_type::actor } },
         }}
      },
      //
      story_event_definition{
         story_event_code::jail,
         {{
            story_event_definition::member{ 'R1', "hGuard",      "Guard",       { form_type::actor } },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group", { form_type::none } }, // is this a faction?
            story_event_definition::member{ 'L1', "Location",    "Location",    { form_type::location } },
            story_event_definition::member{ 'V1', "Crime Gold",  "Crime Gold" },
         }}
      },
      //
      story_event_definition{
         story_event_code::kill,
         {{
            story_event_definition::member{ 'R1', "Victim",       "Victim",   { form_type::actor } },
            story_event_definition::member{ 'R2', "Killer",       "Killer",   { form_type::actor } },
            story_event_definition::member{ 'L1', "Location",     "Location", { form_type::location } },
            story_event_definition::member{ 'V1', "Crime Status", "Crime Status" },
            story_event_definition::member{ 'V1', "Relationship Rank to Killer before Death", "Relationship" },
         }}
      },
      //
      story_event_definition{
         story_event_code::lockpick,
         {{
            story_event_definition::member{ 'R1', "Actor",       "Actor",         { form_type::actor } },
            story_event_definition::member{ 'R2', "Lock Object", "Locked Object", { form_type::reference } },
         }}
      },
      //
      story_event_definition{
         story_event_code::new_voice_power,
         {{
            story_event_definition::member{ 'R1', "Actor",       "Actor",       { form_type::actor } },
            story_event_definition::member{ 'F1', "Voice Power", "Voice Power", { form_type::shout } },
         }}
      },
      //
      story_event_definition{
         story_event_code::pay_fine,
         {{
            story_event_definition::member{ 'R1', "hCriminal",   "Criminal",    { form_type::actor } },
            story_event_definition::member{ 'R2', "hGuard",      "Guard",       { form_type::actor } },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group", { form_type::none } }, // is this a faction?
            story_event_definition::member{ 'V1', "Crime Gold",  "Crime Gold" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_activate_actor,
         {{
            story_event_definition::member{ 'L1', "Location", "Location", { form_type::location } },
            story_event_definition::member{ 'R1', "Actor ",   "Actor",    { form_type::actor } }, // the space in the Bethesda name is not a typo
         }}
      },
      //
      story_event_definition{
         story_event_code::player_add_item,
         {{
            story_event_definition::member{ 'R1', "OwnerRef",          "Owner",          { form_type::actor } },
            story_event_definition::member{ 'R2', "OriginalContainer", "Container",      { form_type::reference } },
            story_event_definition::member{ 'L1', "Location",          "Location",       { form_type::location } },
            story_event_definition::member{ 'F1', "ObjectForm",        "Item Base Form", { form_type::none } },
            story_event_definition::member{ 'V1', "AquireType",        "Acquire Type" },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_cured,
         {{
            story_event_definition::member{ 'F1', "Disease", "Disease", { form_type::spell } },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_infected,
         {{
            story_event_definition::member{ 'R1', "Transmitting Actor", "Transmitting Actor", { form_type::actor } },
            story_event_definition::member{ 'F1', "Disease",            "Disease",            { form_type::spell } },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_receives_favor,
         {{
            story_event_definition::member{ 'R1', "Actor", "Actor", { form_type::actor } },
         }}
      },
      //
      story_event_definition{
         story_event_code::player_remove_item,
         {{
            story_event_definition::member{ 'R1', "OwnerRef",   "Owner",             { form_type::actor } },
            story_event_definition::member{ 'R2', "ItemRef",    "Item In-World Ref", { form_type::reference } },
            story_event_definition::member{ 'L1', "Location",   "Location",          { form_type::location } },
            story_event_definition::member{ 'F1', "ObjectForm", "Item Base Form",    { form_type::none } },
            story_event_definition::member{ 'V1', "RemoveType", "Remove Type" },
         }}
      },
      //
      story_event_definition{
         story_event_code::quest_start,
         {{
            story_event_definition::member{ 'Q1', "Quest", "Quest", { form_type::quest } },
         }}
      },
      //
      story_event_definition{
         story_event_code::script,
         {{
            story_event_definition::member{ 'L1', "Location", "Location", { form_type::location } },
            story_event_definition::member{ 'R1', "Ref 1",    "Ref 1",    { form_type::actor } },
            story_event_definition::member{ 'R2', "Ref 2",    "Ref 2",    { form_type::actor } },
            story_event_definition::member{ 'V1', "Value 1",  "Value 1" },
            story_event_definition::member{ 'V2', "Value 2",  "Value 2" },
         }}
      },
      //
      story_event_definition{
         story_event_code::served_time_in_jail,
         {{
            story_event_definition::member{ 'L1', "Location",    "Location",    { form_type::location } },
            story_event_definition::member{ 'F1', "pCrimeGroup", "Crime Group", { form_type::none } }, // is this a faction?
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
            story_event_definition::member{ 'R1', "Victim",     "Victim",     { form_type::actor } },
            story_event_definition::member{ 'R2', "Trespasser", "Trespasser", { form_type::actor } },
            story_event_definition::member{ 'L1', "Location",   "Location",   { form_type::location } },
            story_event_definition::member{ 'V1', "Crime",      "Crime" },
         }}
      },
      //
   };

   const story_event_definition::member* story_event_definition::member_by_signature(uint16_t s) const {
      for (auto& entry : this->members)
         if (entry.signature == s)
            return &entry;
      return nullptr;
   }
   const story_event_definition::member* story_event_definition::member_by_wide_signature(uint32_t s) const {
      s = s >> 0x10; // 52 31 00 00 -> 52 31 -> 'R1'
      return this->member_by_signature(s);
   }

   /*static*/ const story_event_definition* story_event_definition::lookup(story_event_code_t code) {
      for (auto& entry : list)
         if (entry.signature == code)
            return &entry;
      return nullptr;
   }
}