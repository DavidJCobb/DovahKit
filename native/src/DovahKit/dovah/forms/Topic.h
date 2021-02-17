#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Topic : public Form {
      public:
         static constexpr form_type_t form_type = form_type::topic;
         Topic(const constructor_params& c) : Form(form_type, c) {};

         struct dialogue_flag {
            dialogue_flag() = delete;
            enum type : uint8_t {
               do_all_before_repeating = 0x01,
            };
         };
         using dialogue_flags_t = std::underlying_type_t<dialogue_flag::type>;

         enum class category : uint8_t {
            topic,
            favor, // DA14 only, apparently
            scene,
            combat,
            favors,
            detection,
            service,
            miscellaneous
         };

         //
         // Subtype indices aren't actually reliable because they're sequentially numbered, but Bethesda 
         // added six values into the middle of the list for the Dragonborn DLC's flying mount feature. 
         // This means that values at or after the point where those were added are unreliable; for 
         // example, Skyrim.esm has tons of topics that now have the right subtype signature but the 
         // wrong subtype index.
         //
         enum class subtype_index : uint16_t {
            custom = 0x00,
            forcegreet,
            rumors,
            unknown_03,
            intimidate,
            flatter,
            bribe,
            ask_gift,
            gift = 0x08,
            ask_favor,
            favor,
            show_relationships,
            follow,
            reject,
            scene,
            show,
            agree = 0x10,
            refuse,
            exit_favor_state,
            moral_refusal,
            flying_mount_land,
            flying_mount_cancel_land,
            flying_mount_accept_target,
            flying_mount_reject_target,
            flying_mount_no_target,
            flying_mount_destination_reached,
            attack,
            power_attack,
            bash,
            hit,
            flee,
            bleedout,
            avoid_threat = 0x20,
            death,
            group_strategy,
            block,
            taunt,
            ally_killed,
            steal,
            yield,
            accept_yield,
            pickpocket_combat,
            assault,
            murder,
            assault_no_crime,
            murder_no_crime,
            pickpocket_no_crime,
            steal_from_no_crime,
            trespass_against_no_crime = 0x30,
            trespass,
            werewolf_transform_crime,
            voice_power_start_short,
            voice_power_start_long,
            voice_power_end_short,
            voice_power_end_long,
            alert_idle,
            lost_idle,
            normal_to_alert,
            alert_to_combat,
            normal_to_combat,
            alert_to_normal,
            combat_to_normal,
            combat_to_lost,
            lost_to_normal,
            lost_to_combat = 0x40,
            detect_friend_die,
            service_refusal,
            repair,
            travel,
            training,
            barter_exit,
            repair_exit,
            recharge,
            recharge_exit,
            training_exit,
            observe_combat,
            notice_corpse,
            time_to_go,
            goodbye,
            hello,
            swing_melee_weapon = 0x50,
            shoot_bow,
            z_key_object,
            jump,
            knock_over_object,
            destroy_object,
            stand_on_furniture,
            locked_object,
            pickpocket_topic,
            pursue_idle_topic,
            sharedinfo,
            player_cast_projectile_spell,
            player_cast_self_spell,
            player_shout,
            idle,
            enter_sprint_breath,
            enter_bow_zoom_breath = 0x60,
            exit_bow_zoom_breath,
            actor_collide_with_actor,
            player_in_iron_sights,
            out_of_breath,
            combat_grunt,
            leave_water_breath,
         };

         struct subtype_signature {
            subtype_signature() = delete;
            //
            // Combat lines are named in reference to the following states:
            //
            //  - Normal:  The actor is at rest.
            //  - Alerted: The actor suspects that an enemy is nearby, and is searching for them.
            //  - Combat:  The actor has engaged an enemy in combat.
            //  - Lost:    The actor is searching for an enemy they were just in combat with.
            //
            // Possible state progressions are:
            //
            //  - Normal -> Alerted -> Combat | An enemy tried to sneak, but was found.
            //  - Normal -> Alerted -> Normal | An enemy tried to sneak, was sensed, but stayed hidden.
            //  - Normal -> Combat            | An enemy was detected immediately.
            //  - Combat -> Lost -> Combat    | Combat paused when an enemy tried and failed to hide.
            //  - Combat -> Lost -> Normal    | Combat ended when an enemy successfully hid.
            //  - Combat -> Normal            | Combat ended when an enemy was killed.
            //
            // Combat states besides "normal" have both state-change lines and "idle" lines, the latter 
            // of which play periodically.
            //
            enum type : uint32_t {
               #pragma region Combat
               accept_yield                     = 'ACYI', // the speaker has accepted an enemy's surrender
               ally_killed                      = 'ALKL', // 
               attack                           = 'ATCK', // the speaker is performing a normal (i.e. non-power) attack
               bash                             = 'BASH', // 
               bleedout                         = 'BLED', // the speaker has just entered bleedout
               block                            = 'BLOC', // the speaker has just blocked a melee attack
               death                            = 'DETH', // the speaker is dying
               detect_friend_die                = 'DFDA', // 
               flee                             = 'FLEE', // the speaker is fleeing
               hit                              = 'HIT_', // the speaker has been hit with an attack
               power_attack                     = 'POAT', // the speaker is power-attacking
               taunt                            = 'TAUT', // the speaker is taunting an enemy during combat (e.g. "Skyrim belongs to the Nords!")
               #pragma endregion
               #pragma region Crime
               assault                          = 'ASSA', // the speaker just witnessed an assault and considers it a crime against their faction
               assault_no_crime                 = 'ASNC', // the speaker is acknowledging an assault, but forgiving the crime ("I guess you had your reasons.")
               locked_object                    = 'LOOB', // said to the player when they aim at a locked object, as if gearing up to lockpick it
               murder                           = 'MURD', // the speaker just witnessed a murder and considers it a crime against their faction
               murder_no_crime                  = 'MUNC', // the speaker is acknowledging a murder, but forgiving the crime ("Guess they deserved it...")
               pickpocket_combat                = 'PICC', // the speaker just witnessed a failed pickpocket attempt and considers it a crime against their faction (test Subject.IsActorAVictim to determine if the speaker was the pickpocketing victim)
               pickpocket_no_crime              = 'PICN', // the speaker is acknowledging a pickpocketing attempt, but forgiving the crime ("I guess I can look the other way, this time.")
               pickpocket_topic                 = 'PICT', // said to the player when they aim at the speaker while speaking, as if gearing up to pickpocket the speaker
               pursue_idle_topic                = 'PURS', // the speaker is a guard in pursuit of a criminal, wishing to make an arrest
               steal                            = 'STEA', // the speaker just witnessed a theft and considers it a crime against their faction
               steal_from_no_crime              = 'STFN', // the speaker is acknowledging a theft, but forgiving the crime ("You could have asked before just taking it.")
               trespass                         = 'TRES', // the speaker is accosting a trespasser and demanding that they leave
               trespass_against_no_crime        = 'TRAN', // the speaker is acknowledging a trespasser, but forgiving them for their crime ("You can stay, but you're not supposed to be in here.")
               werewolf_transform_crime         = 'WTCR', // the speaker just witnessed someone transforming into a werewolf and considers that a crime against their faction
               #pragma endregion
               #pragma region Detection
               alert_idle                       = 'ALIL', // plays periodically while the speaker is alerted
               alert_to_combat                  = 'ALTC', // plays when an alerted speaker discovers their target and enters combat with them
               alert_to_normal                  = 'ALTN', // plays when an alerted speaker abandons their search for their target
               combat_to_lost                   = 'COLO', // the speaker was previously in combat with a target, but has now lost track of them
               combat_to_normal                 = 'COTN', // the speaker was previously in combat, and no longer is, likely due to their target's death
               lost_idle                        = 'LOIL', // the speaker previously lost track of a target during combat, and will utter these lines occasionally during their search for that target
               lost_to_combat                   = 'LOTC', // the speaker previously lost track of a target during combat, but has now found them again
               lost_to_normal                   = 'LOTN', // the speaker previously lost track of a target during combat, and has now given up the search
               normal_to_alert                  = 'NOTA', // the speaker was not alerted or in combat, but has just been alerted to the possible presence of an enemy
               normal_to_combat                 = 'NOTC', // the speaker was not alerted or in combat, but has just been engaged in combat
               #pragma endregion
               #pragma region Flying mount
               flying_mount_accept_target       = 'FMAT', // the speaker is the player's flying mount, and is accepting an order to attack a nearby hostile
               flying_mount_destination_reached = 'FMDR', // the speaker is the player's flying mount, and the two have reached their destination
               flying_mount_land                = 'FMLX', // the speaker is the player's flying mount, and is responding to a command to land somewhere
               flying_mount_no_target           = 'FMNT', // the speaker is the player's flying mount, and there are no nearby hostiles to attack
               flying_mount_reject_target       = 'FMRT', // the speaker is the player's flying mount, and is unable to follow an order to attack a nearby hostile
               flying_mount_cancel_land         = 'FMXL', // the speaker is the player's flying mount, and is acknowledging that a previous order to land has been canceled
               #pragma endregion
               #pragma region Follower
               agree                            = 'AGRE', // the speaker is the player's follower and is accepting an order
               exit_favor_state                 = 'FEXT', // the speaker is the player's follower, and the player has exited the "giving orders" state
               moral_refusal                    = 'MREF', // the speaker is the player's follower and is refusing an order on moral grounds
               refuse                           = 'REFU', // the speaker is the player's follower and is rejecting an order on the grounds that it is impossible
               show                             = 'SHOW', // the speaker is the player's follower, and the player has entered the favor state (i.e. begun giving orders) // guessing Bethesda's logic was, you're "showing" them things to interact with
               #pragma endregion
               #pragma region Grunts and similar
               combat_grunt                     = 'GRNT', // 
               enter_bow_zoom_breath            = 'ENBZ', // the speaker has entered ironsights (i.e. they're holding their breath to steady their aim)
               enter_sprint_breath              = 'BREA', // breathing while sprinting
               exit_bow_zoom_breath             = 'EXBZ', // the speaker has exited  ironsights (i.e. they're releasing a held breath)
               leave_water_breath               = 'LWBS', // the speaker was diving and has just come up for air
               out_of_breath                    = 'OUTB', // the speaker is out of breath (i.e. they are the player, were sprinting, and ran out of stamina)
               #pragma endregion
               #pragma region Reactions to other actors
               actor_collide_with_actor         = 'ACAC', // an actor has shoved past the speaker (check Speaker.IsSmallBump == 0 to see if the actor was sprinting)
               destroy_object                   = 'DEOB',
               fire_weapon                      = 'FIWE', // said to the player when they fire a weapon near the speaker // listed as "Shoot Bow" in the CK?
               knock_over_object                = 'KNOO', // said to the player when they knock loose items over
               notice_corpse                    = 'NOTI', // the speaker just passed close to a corpse
               observe_combat                   = 'OBCO', // the speaker just observed other actors in combat but is not themselves in combat ("Those fools are actually fighting!")
               player_cast_projectile_spell     = 'PCPS',
               player_cast_self_spell           = 'PCSS',
               player_cast_shout                = 'PCSH', // the speaker just witnessed the player casting a Shout while not in combat
               player_in_iron_sights            = 'PIRN', // the player is in ironsights and aiming at the speaker while not in combat
               stand_on_furniture               = 'STOF', // said to the player when they stand on furniture
               swing_melee_weapon               = 'SWMW', // said to the player when they swing a melee weapon near the speaker while not in combat
               z_key_object                     = 'ZKEY', // said to the player when they Z-key objects
               #pragma endregion
               //
               ask_favor                        = 'ASKF',
               ask_gift                         = 'ASKG',
               avoid_threat                     = 'AVTH', // 
               barter_exit                      = 'BAEX', // possibly an unused Oblivion leftover, meant to play when the player exits the Barter menu
               bribe                            = 'BRIB', // possibly an unused Oblivion leftover, originally used in the Persuasion minigame
               custom                           = 'CUST', // player-initiated dialogue tree content
               favor                            = 'FAVO',
               flatter                          = 'FLAT',
               follow                           = 'FOLL',
               reject                           = 'FRJT',
               fvdl                             = 'FVDL', // Follower Voice Dialogue?
               gift                             = 'GIFF',
               goodbye                          = 'GBYE', // the player has exited dialogue with the speaker
               group_strategy                   = 'GRST', // 
               hello                            = 'HELO', // the player has walked up to the speaker (or just stood too close to them...) and the speaker is acknowledging their presence
               idle                             = 'IDLE', // the speaker is idle (uses for this include shopkeeper nagging, followers' location comments, and more)
               intimidate                       = 'INTI',
               jump                             = 'JUMP',
               forcegreet                       = 'PFGT', // the speaker is forcegreeting the player
               rumors                           = 'RUMO', // 
               scene_dialogue_action            = 'SCEN', // a line of dialogue that only exists for use in a Scene action
               service_refusal                  = 'SERU', // possibly an unused Oblivion leftover
               sharedinfo                       = 'IDAT', // infos in this topic can be used as SharedInfos
               show_relationships               = 'SHRE',
               time_to_go                       = 'TITG', // the speaker is ordering the player to leave, lest they become a trespasser? (in Oblivion, this meant the speaker wants to switch packages but can't because their package is flagged "Continue if PC Near")
               voice_power_end_long             = 'VPEL', // the last two words of a three-word shout
               voice_power_end_short            = 'VPES', // the last word of a two-word shout
               voice_power_start_long           = 'VSPL', // the first word of a multi-word shout
               voice_power_start_short          = 'VPSS', // a one-word shout
               yield                            = 'YIEL', // 
               //
               #pragma region Unused
               recharge                         = 'RECH', // possibly an unused Oblivion leftover, meant to play when the player opens the Recharge Weapon menu
               recharge_exit                    = 'RCEX', // possibly an unused Oblivion leftover, meant to play when the player exits the Recharge Weapon menu
               repair                           = 'REPA', // possibly an unused Oblivion leftover, meant to play when the player opens the Repair menu
               repair_exit                      = 'REEX', // possibly an unused Oblivion leftover, meant to play when the player exits the Repair menu
               training                         = 'TRAI', // possibly an unused Oblivion leftover, meant to play when the player opens the Training menu
               training_exit                    = 'TREX', // possibly an unused Oblivion leftover, meant to play when the player exits the Training menu
               travel                           = 'TRAV', // unused in Oblivion; apparently unused in Skyrim
               #pragma endregion
            };
         };

         struct {
            dialogue_branch_reference_t branch;
            dialogue_quest_reference_t  quest;
         } owning_forms;
         localized_string text; // FULL // player's dialogue
         struct {
            dialogue_flags_t flags   = 0;
            category         dialogue_tab = category::topic;
            subtype_index    subtype = subtype_index::custom; // unused?
         } data; // DATA
         float    priority = 50.0F;  // PNAM
         uint32_t subtype  = 'CUST'; // SNAM // in-game, the game uses whichever subtype between DATA and SNAM was loaded last
         components::object_bounds object_bounds; // OBND. recognized, but probably discarded at run-time.
         components::papyrus_attachment_data script_data; // VMAD

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}