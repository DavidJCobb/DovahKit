
## Info

### Crime

Most crimes have at least two lines of dialogue: one for if a criminal is caught in the act and held accountable for their crime; and another for if a criminal is caught in the act but the crime is forgiven (typically because the witness is a friend or faction ally). Werewolf transformations do not have a crime-forgiven line, and some crimes have additional dialogue for if the player is acting suspicious and the speaker suspects that a crime is about to occur.

### Detection

Detection lines are named in reference to the following states:

 - Normal:  The actor is at rest.
 - Alerted: The actor suspects that an enemy is nearby, and is searching for them.
 - Combat:  The actor has engaged an enemy in combat.
 - Lost:    The actor is searching for an enemy they were just in combat with.

Possible state progressions are:

| Progression | Info
| :- | :- |
| Normal -> Alerted -> Combat | An enemy tried to sneak, but was found.
| Normal -> Alerted -> Normal | An enemy tried to sneak, was sensed, but stayed hidden.
| Normal -> Combat            | An enemy was detected immediately.
| Combat -> Lost -> Combat    | Combat paused when an enemy tried and failed to hide.
| Combat -> Lost -> Normal    | Combat ended when an enemy successfully hid.
| Combat -> Normal            | Combat ended when an enemy was killed.

Detection states besides "normal" have both state-change lines and "idle" lines, the latter 
of which play periodically.

### Favors

The "favor" system is what allows the player to give orders to their follower from afar. Actors can accept orders, refuse them on moral grounds, or refuse them because they are impossible to follow.

The Dragonborn DLC added several additional favor topic types for ordering an airborne mount (i.e. a dragon).

## List

| Subtype signature | Subtype name | Engine Category | Semantic Category | Explanation |
| :-: | :- | :-: | :-: | :- |
| CUST | Custom | topic | System | Player-initiated dialogue tree content.
| PFGT | ForceGreet | topic | System | The speaker is forcegreeting the player.
| RUMO | Rumors | topic | System |
| FVDL | Custom | favor_dialogue | | FaVor DiaLogue? Called "Custom" in-engine.
| INTI | Intimidate | favor_dialogue | |
| FLAT | Flatter | favor_dialogue | |
| BRIB | Bribe | favor_dialogue | | Possibly an unused Oblivion leftover, originally used in the Persuasion minigame.
| ASKG | AskGift | favor_dialogue | |
| GIFF | Gift | favor_dialogue | |
| ASKF | AskFavor | favor_dialogue | |
| FAVO | Favor | favor_dialogue | |
| SHRE | ShowRelationships | favor_dialogue | |
| FOLL | Follow | favor_dialogue | |
| FRJT | Reject | favor_dialogue | |
| SCEN | Custom | scene | System | A line of dialogue that only exists for use in a Scene action.
| SHOW | Show | favors | Favors | The speaker is the player's follower, and the player has entered the favor state (i.e. begun giving orders). Likely named for the idea that the player is "showing" the speaker things to interact with.
| AGRE | Agree | favors | Favors | The speaker is the player's follower and is accepting an order.
| REFU | Refuse | favors | Favors | The speaker is the player's follower and is rejecting an order on the grounds that it is impossible.
| FEXT | ExitFavorState | favors | Favors | The speaker is the player's follower, and the player has exited the "giving orders" state.
| MREF | MoralRefusal | favors | Favors | The speaker is the player's follower and is refusing an order on moral grounds.
| FMLX | FlyingMountLand | favors | Favors (Flying) | The speaker is the player's flying mount, and is responding to a command to land somewhere.
| FMXL | FlyingMountCancelLand | favors | Favors (Flying) | The speaker is the player's flying mount, and is acknowledging that a previous order to land has been canceled.
| FMAT | FlyingMountAcceptTarget | favors | Favors (Flying) | The speaker is the player's flying mount, and is accepting an order to attack a nearby hostile.
| FMRT | FlyingMountRejectTarget | favors | Favors (Flying) | The speaker is the player's flying mount, and is unable to follow an order to attack a nearby hostile.
| FMNT | FlyingMountNoTarget | favors | Favors (Flying) | The speaker is the player's flying mount, and there are no nearby hostiles to attack.
| FMDR | FlyingMountDestinationReached | favors | Favors (Flying) | The speaker is the player's flying mount, and the two have reached their destination.
| ATCK | Attack | combat | Combat | The speaker is performing a normal (i.e. non-power) attack.
| POAT | PowerAttack | combat | Combat | The speaker is power-attacking.
| BASH | Bash | combat | Combat |
| HIT_ | Hit | combat | Combat | The speaker has been hit with an attack.
| FLEE | Flee | combat | Combat | The speaker is fleeing.
| BLED | BleedOut | combat | Combat | The speaker has just entered bleedout.
| AVTH | AvoidThreat | combat | Combat |
| DETH | Death | combat | Combat | The speaker is dying.
| GRST | GroupStrategy | combat | Combat |
| BLOC | Block | combat | Combat | The speaker has just blocked a melee attack.
| TAUT | Taunt | combat | Combat | The speaker is taunting an enemy during combat (e.g. "Skyrim belongs to the Nords!").
| ALKL | AllyKilled | combat | Combat |
| STEA | Steal | combat | Crime | the speaker just witnessed a theft and considers it a crime against their faction.
| YIEL | Yield | combat | Combat |
| ACYI | AcceptYield | combat | Combat | The speaker has accepted an enemy's surrender.
| PICC | PickpocketCombat | combat | Crime | The speaker just witnessed a failed pickpocket attempt and considers it a crime against their faction (test `Subject.IsActorAVictim` to determine if the speaker was the pickpocketing victim).
| ASSA | Assault | combat | Crime | The speaker just witnessed an assault and considers it a crime against their faction.
| MURD | Murder | combat | Crime | The speaker just witnessed a murder and considers it a crime against their faction.
| ASNC | AssaultNC | combat | Crime | The speaker is acknowledging an assault, but forgiving the crime ("I guess you had your reasons.").
| MUNC | MurderNC | combat | Crime | The speaker is acknowledging a murder, but forgiving the crime ("Guess they deserved it...").
| PICN | PickpocketNC | combat | Crime | The speaker is acknowledging a pickpocketing attempt, but forgiving the crime ("I guess I can look the other way, this time.").
| STFN | StealFromNC | combat | Crime | The speaker is acknowledging a theft, but forgiving the crime ("You could have asked before just taking it.").
| TRAN | TrespassAgainstNC | combat | Crime | The speaker is acknowledging a trespasser, but forgiving them for their crime ("You can stay, but you're not supposed to be in here.").
| TRES | Trespass | combat | Crime | The speaker is accosting a trespasser and demanding that they leave.
| WTCR | WereTransformCrime | combat | Crime | The speaker just witnessed someone transforming into a werewolf and considers that a crime against their faction.
| VPSS | VoicePowerStartShort | combat | Shouts | A one-word shout.
| VPSL | VoicePowerStartLong | combat | Shouts | The first word of a multi-word shout.
| VPES | VoicePowerEndShort | combat | Shouts | The last word of a two-word shout.
| VPEL | VoicePowerEndLong | combat | Shouts | The last two words of a three-word shout.
| ALIL | AlertIdle | detection | Detection | Plays periodically while the speaker is alerted.
| LOIL | LostIdle | detection | Detection | The speaker previously lost track of a target during combat, and will utter these lines occasionally during their search for that target.
| NOTA | NormalToAlert | detection | Detection | The speaker was not alerted or in combat, but has just been alerted to the possible presence of an enemy.
| ALTC | AlertToCombat | detection | Detection | Plays when an alerted speaker discovers their target and enters combat with them.
| NOTC | NormalToCombat | detection | Detection | The speaker was not alerted or in combat, but has just been engaged in combat.
| ALTN | AlertToNormal | detection | Detection | Plays when an alerted speaker abandons their search for their target.
| COTN | CombatToNormal | detection | Detection | The speaker was previously in combat, and no longer is, likely due to their target's death.
| COLO | CombatToLost | detection | Detection | The speaker was previously in combat with a target, but has now lost track of them.
| LOTN | LostToNormal | detection | Detection | The speaker previously lost track of a target during combat, and has now given up the search.
| LOTC | LostToCombat | detection | Detection | The speaker previously lost track of a target during combat, but has now found them again.
| DFDA | DetectFriendDie | detection | Detection |
| SERU | ServiceRefusal | service | | Possibly an unused Oblivion leftover.
| REPA | Repair | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player opens the Repair menu.
| TRAV | Travel | service | | Unused in Oblivion; apparently unused in Skyrim.
| TRAI | Training | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player opens the Training menu.
| BAEX | BarterExit | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player exits the Barter menu.
| REEX | RepairExit | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player exits the Repair menu.
| RECH | Recharge | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player opens the Recharge Weapon menu.
| RCEX | RechargeExit | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player exits the Recharge Weapon menu.
| TREX | TrainingExit | service | Leftovers | Possibly an unused Oblivion leftover, meant to play when the player exits the Training menu.
| OBCO | ObserveCombat | miscellaneous | Remarks | The speaker just observed other actors in combat but is not themselves in combat ("Those fools are actually fighting!").
| NOTI | NoticeCorpse | miscellaneous | Remarks | The speaker just passed close to a corpse.
| TITG | TimeToGo | miscellaneous | Crime | The speaker is ordering the player to leave, lest they become a trespasser? (In Oblivion, this meant the speaker wants to switch packages but can't because their package is flagged "Continue if PC Near.")
| GBYE | Goodbye | miscellaneous | | The player has exited dialogue with the speaker.
| HELO | Hello | miscellaneous | Remarks | The player has walked up to the speaker (or just stood too close to them...) and the speaker is acknowledging their presence.
| SWMW | SwingMeleeWeapon | miscellaneous | Remarks | Said to the player when they swing a melee weapon near the speaker while not in combat.
| FIWE | ShootBow | miscellaneous | Remarks | Said to the player when they fire a weapon near the speaker.
| ZKEY | ZKeyObject | miscellaneous | Remarks | Said to the player when they Z-key objects.
| JUMP | Jump | miscellaneous | |
| KNOO | KnockOverObject | miscellaneous | Remarks | Said to the player when they knock loose items over.
| DEOB | DestroyObject | miscellaneous | Remarks |
| STOF | StandonFurniture | miscellaneous | Remarks | Said to the player when they stand on furniture.
| LOOB | LockedObject | miscellaneous | Crime | Said to the player when they aim at a locked object, as if gearing up to lockpick it.
| PICT | PickpocketTopic | miscellaneous | Crime | Said to the player when they aim at the speaker while speaking, as if gearing up to pickpocket the speaker.
| PURS | PursueIdleTopic | miscellaneous | Crime | The speaker is a guard in pursuit of a criminal, wishing to make an arrest.
| IDAT | SharedInfo | miscellaneous | System |
| PCPS | PlayerCastProjectileSpell | miscellaneous | Remarks |
| PCSS | PlayerCastSelfSpell | miscellaneous | Remarks |
| PCSH | PlayerShout | miscellaneous | Remarks | The speaker just witnessed the player casting a Shout while not in combat.
| IDLE | Idle | miscellaneous | Remarks | The speaker is idle (uses for this include shopkeeper nagging, followers' location comments, and more).
| BREA | EnterSprintBreath | miscellaneous | Barks, Gasps, Etc. | Breathing while sprinting.
| ENBZ | EnterBowZoomBreath | miscellaneous | Barks, Gasps, Etc. | The speaker has entered ironsights (i.e. they're holding their breath to steady their aim).
| EXBZ | ExitBowZoomBreath | miscellaneous | Barks, Gasps, Etc. | The speaker has exited  ironsights (i.e. they're releasing a held breath).
| ACAC | ActorCollidewithActor | miscellaneous | Remarks | An actor has shoved past the speaker (check `Speaker.IsSmallBump == 0` to see if the actor was sprinting).
| PIRN | PlayerinIronSights | miscellaneous | Remarks | The player is in ironsights and aiming at the speaker while not in combat.
| OUTB | OutofBreath | miscellaneous | Barks, Gasps, Etc. | The speaker is out of breath (i.e. they are the player, were sprinting, and ran out of stamina).
| GRNT | CombatGrunt | miscellaneous | |
| LWBS | LeaveWaterBreath | miscellaneous | Barks, Gasps, Etc. | The speaker was diving and has just come up for air.
