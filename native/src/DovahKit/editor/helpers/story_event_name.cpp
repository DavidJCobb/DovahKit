#include "./story_event_name.h"
#include <QCoreApplication>

namespace editor_helpers {
   extern QString story_event_name(dovah::story_event_code::type event) {
      switch (event) {
         case dovah::story_event_code::none:
            return QCoreApplication::translate("all story event names", "NONE");
         case dovah::story_event_code::undefined:
            return QCoreApplication::translate("all story event names", "NONE");
            
         case dovah::story_event_code::actor_dialogue:
            return QCoreApplication::translate("all story event names", "Actor Dialogue");
         case dovah::story_event_code::actor_hello:
            return QCoreApplication::translate("all story event names", "Actor Hello");
         case dovah::story_event_code::arrest:
            return QCoreApplication::translate("all story event names", "Arrest");
         case dovah::story_event_code::assault:
            return QCoreApplication::translate("all story event names", "Assault");
         case dovah::story_event_code::bribe:
            return QCoreApplication::translate("all story event names", "Bribe");
         case dovah::story_event_code::cast_magic:
            return QCoreApplication::translate("all story event names", "Cast Magic");
         case dovah::story_event_code::change_relationship_rank:
            return QCoreApplication::translate("all story event names", "Change Relationship Rank");
         case dovah::story_event_code::change_location:
            return QCoreApplication::translate("all story event names", "Change Location");
         case dovah::story_event_code::craft_item:
            return QCoreApplication::translate("all story event names", "Craft Item");
         case dovah::story_event_code::crime_gold:
            return QCoreApplication::translate("all story event names", "Crime Gold");
         case dovah::story_event_code::dead_body:
            return QCoreApplication::translate("all story event names", "Dead Body");
         case dovah::story_event_code::escaped_jail:
            return QCoreApplication::translate("all story event names", "Escape Jail");
         case dovah::story_event_code::flatter:
            return QCoreApplication::translate("all story event names", "Flatter");
         case dovah::story_event_code::level_up:
            return QCoreApplication::translate("all story event names", "Increase Level");
         case dovah::story_event_code::intimidate:
            return QCoreApplication::translate("all story event names", "Intimidate");
         case dovah::story_event_code::jail:
            return QCoreApplication::translate("all story event names", "Jail");
         case dovah::story_event_code::kill:
            return QCoreApplication::translate("all story event names", "Kill");
         case dovah::story_event_code::lockpick:
            return QCoreApplication::translate("all story event names", "Lockpick");
         case dovah::story_event_code::new_voice_power:
            return QCoreApplication::translate("all story event names", "New Voice Power");
         case dovah::story_event_code::pay_fine:
            return QCoreApplication::translate("all story event names", "Pay Fine");
         case dovah::story_event_code::player_activate_actor:
            return QCoreApplication::translate("all story event names", "Player Activate Actor");
         case dovah::story_event_code::player_add_item:
            return QCoreApplication::translate("all story event names", "Player Add Item");
         case dovah::story_event_code::player_cured:
            return QCoreApplication::translate("all story event names", "Player Cured");
         case dovah::story_event_code::player_infected:
            return QCoreApplication::translate("all story event names", "Player Infected");
         case dovah::story_event_code::player_receives_favor:
            return QCoreApplication::translate("all story event names", "Player Receives Favor");
         case dovah::story_event_code::player_remove_item:
            return QCoreApplication::translate("all story event names", "Player Remove Item");
         case dovah::story_event_code::quest_start:
            return QCoreApplication::translate("all story event names", "Quest Start");
         case dovah::story_event_code::script:
            return QCoreApplication::translate("all story event names", "Script");
         case dovah::story_event_code::skill_increase:
            return QCoreApplication::translate("all story event names", "Skill Increase");
         case dovah::story_event_code::served_time_in_jail:
            return QCoreApplication::translate("all story event names", "Served Time in Jail");
         case dovah::story_event_code::trespass:
            return QCoreApplication::translate("all story event names", "Trespass");
      }
      return "";
   }
}