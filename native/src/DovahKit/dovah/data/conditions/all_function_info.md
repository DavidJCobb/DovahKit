
## All conditions

Prior to Skyrim, Bethesda RPGs used a proprietary scripting engine which defined condition and action functions within a single command table. Conditions within form data just borrowed from that table, as did console commands. Skyrim deprecated the old scripting engine but kept the commands. This means that the command table includes actions, not just conditions; however, as far as I know these actions aren't actually invocable via a condition list on a form.

Actions in the below table are indicated with their names in strikethrough. In some cases, I haven't bothered to actually check what their names would be, so they're listed as dashes with strikethrough.

| Index | Name | Description | Arg 1 | Arg 2 |
| -: | :- | :- | - | - |
| 0 | GetWantBlocking
| 1 | GetDistance | Returns the distance between this ref and another ref. | ObjectReference |
| 2 | ~~AddItem~~
| 3 | ~~SetEssential~~
| 4 | ~~Rotate~~
| 5 | GetLocked | Returns 1 if this ref is a locked door or container, or 0 otherwise.
| 6 | GetPos | Returns this ref's position along a given axis. | Axis
| 7 | ~~SetPos~~
| 8 | GetAngle | Returns this ref's rotation in degrees along a given axis. | Axis
| 9 | ~~SetAngle~~
| 10 | GetStartingPos | Returns this ref's starting position along the given axis. | Axis
| 11 | GetStartingAngle | Returns this ref's starting rotation along the given axis. | Axis
| 12 | GetSecondsPassed | 
| 13 | ~~Activate~~
| 14 | GetActorValue | Returns the current value of the specified ActorValue. | ActorValue
| 15 | ~~SetActorValue~~
| 16 | ~~ModActorValue~~
| 17 | ~~SetAtStart~~
| 18 | GetCurrentTime | 
| 19 | ~~PlayGroup~~
| 20 | ~~LoopGroup~~
| 21 | ~~SkipAnim~~
| 22 | ~~StartCombat~~
| 23 | ~~StopCombat~~
| 24 | GetScale | Returns this ref's scale. |
| 25 | IsMoving |
| 26 | IsTurning | Returns 1 if this actor is turning to the left, 2 if they are turning to the right, or 0 otherwise. |
| 27 | GetLineOfSight | | ObjectReference |
| 28 | ~~AddSpell~~
| 29 | ~~RemoveSpell~~
| 30 | ~~Cast~~
| 31 | ~~GetButtonPressed~~
| 32 | GetInSameCell | Returns 1 if this ref is in the same cell as the argument, or 0 otherwise. | ObjectReference |
| 33 | ~~Enable~~
| 34 | ~~Disable~~
| 35 | GetDisbaled | Returns 1 if this ref is disabled, or 0 otherwise. |
| 36 | MenuMode | | Integer |
| 37 | ~~PlaceAtMe~~
| 38 | ~~PlaySound~~
| 39 | GetDisease | Returns 1 if any of this actor's active magic effects come from a spell whose type was set to "Disease," or 0 otherwise. |
| 40 | ~~FailAllObjectives~~
| 41 | GetClothingValue |
| 42 | SameFaction | Returns 1 if this actor is in the same faction as the argument, or 0 otherwise. | Actor |
| 43 | SameRace  | Returns 1 if this actor is of the same race as the argument, or 0 otherwise. | Actor |
| 44 | SameSex | Returns 1 if this actor is of the same sex as the argument, or 0 otherwise. | Actor |
| 45 | GetDetected | | Actor |
| 46 | GetDead |
| 47 | GetItemCount | Returns how many of the specified form this ref has in its inventory. | InventoryItem |
| 48 | GetGold | Returns how much gold this ref has in its inventory. |
| 49 | GetSleeping |
| 50 | GetTalkedToPC |
| 51 | ~~Say~~
| 52 | ~~SayTo~~
| 53 | GetScriptVariable | | ObjectReference | String |
| 54 | ~~StartQuest~~
| 55 | ~~StopQuest~~
| 56 | GetQuestRunning | Returns 1 if the specified quest is running, or 0 otherwise. | Quest |
| 57 | ~~SetStage~~
| 58 | GetStage | Returns the specified quest's current stage number. | Quest |
| 59 | GetStageDone | Returns 1 if the specified quest stage is complete, or 0 otherwise. | Quest | QuestStage |
| 60 | GetFactionRankDifference | | Faction | Actor |
| 61 | GetAlarmed |
| 62 | IsRaining | Returns 1 if the current weather is rainy, or 0 otherwise. |
| 63 | GetAttacked |
| 64 | GetIsCreature |
| 65 | GetLockLevel |
| 66 | GetShouldAttack | | Actor |
| 67 | GetInCell | Returns 1 if this ref is in the argument cell, or 0 otherwise. | Cell |
| 68 | GetIsClass | Returns 1 if this actor is of the specified class, or 0 otherwise. | Class |
| 69 | GetIsRace | Returns 1 if this actor is of the specified race, or 0 otherwise. | Race |
| 70 | GetIsSex | Returns 1 if this actor is of the specified sex, or 0 otherwise. | Sex |
| 71 | GetFactionRank | | Faction |
| 72 | GetIsID | Returns 1 if this ref's base form is the specified form, or 0 otherwise. | BaseForm |
| 73 | GetFactionRank | | Faction |
| 74 | GetGlobalValue | Returns the value of the specified global. | Global |
| 75 | IsSnowing | Returns 1 if the current weather is snowy, or 0 otherwise. |
| 76 | ~~FastTravel~~
| 77 | GetRandomPercent | Returns a random number between 0 and 100, inclusive. |
| 78 | ~~RemoveMusic~~
| 79 | GetQuestVariable | | Quest | String |
| 80 | GetLevel | Returns this actor's level. |
| 81 | IsRotating |
| 82 | ~~RemoveItem~~
| 83 | ~~GetLeveledEncounterValue~~
| 84 | GetDeadCount | Returns the number of times any actor with the specified ActorBase has died. | ActorBase |
| 85 | ~~AddToMap~~
| 86 | ~~StartConversation~~
| 87 | ~~Drop~~
| 88 | ~~AddTopic~~
| 89 | ~~ShowMessage~~
| 90 | ~~SetAlert~~
| 91 | GetIsAlerted |
| 92 | ~~Look~~
| 93 | ~~StopLook~~
| 94 | ~~EvaluatePackage~~
| 95 | ~~SendAssaultAlarm~~
| 96 | ~~EnablePlayerControls~~
| 97 | ~~DisablePlayerControls~~
| 98 | GetPlayerControlsDisabled | | Integer | Integer |
| 99 | GetHeadingAngle | | ObjectReference |
| 100 | ~~PickIdle~~
| 101 | IsWeaponMagicOut |
| 102 | IsTorchOut |
| 103 | IsShieldOut |
| 104 | ~~CreateDetectionEvent~~
| 105 | ~~IsActionRef~~
| 106 | IsFacingUp |
| 107 | GetKnockedState |
| 108 | GetWeaponAnimType |
| 109 | IsWeaponSkillType | | ActorValue |
| 110 | GetCurrentAIPackage |
| 111 | IsWaiting |
| 112 | IsIdlePlaying |
| 113 | ~~CompleteQuest~~
| 114 | ~~Lock~~
| 115 | ~~Unlock~~
| 116 | IsIntimidatedByPlayer |
| 117 | IsPlayerInRegion | Returns 1 if this ref's parent cell belongs to or overlaps the specified region, or 0 otherwise. | Region |
| 118 | GetActorAggroRadiusViolated |
| 119 | ~~---~~
| 120 | ~~---~~
| 121 | ~~---~~
| 122 | GetCrime | | Actor | CrimeType |
| 123 | IsGreetingPlayer |
| 124 | ~~---~~
| 125 | IsGuard |
| 126 | ~~---~~
| 127 | HasBeenEaten | Returns 1 if this actor has been fed on by a cannibal or werewolf, or 0 otherwise. |
| 128 | GetStaminaPercentage |
| 129 | GetPCIsClass | Returns 1 if the player-character is of the specified class, or 0 otherwise. | Class |
| 130 | GetPCIsRace | Returns 1 if the player-character is of the specified race, or 0 otherwise. | Race |
| 131 | GetPCIsSex | Returns 1 if the player-character is of the specified sex, or 0 otherwise. | Sex |
| 132 | GetPCInFaction | Returns 1 if the player-character is in the specified faction, or 0 otherwise. | Faction |
| 133 | SameFactionAsPC |
| 134 | SameRaceAsPC | Returns 1 if this actor is of the same race as the player-character, or 0 otherwise. |
| 135 | SameSexAsPC | Returns 1 if this actor is of the same sex as the player-character, or 0 otherwise. |
| 136 | GetIsReference | Returns 1 if this ref is the specified ref, or 0 otherwise. | ObjectReference |
| 137 | ~~---~~
| 138 | ~~---~~
| 139 | ~~---~~
| 140 | ~~---~~
| 141 | IsTalking |
| 142 | GetWalkSpeed |
| 143 | GetCurrentAIProcedure |
| 144 | GetTrespassWarningLevel |
| 145 | IsTrespassing |
| 146 | IsInMyOwnedCell |
| 147 | GetWindSpeed |
| 148 | GetCurrentWeatherPercent |
| 149 | GetIsCurrentWeather | Returns 1 if the specified weather is the current weather, or 0 otherwise. | Weather |
| 150 | IsContinuingPackagePCNear |
| 151 | ~~---~~
| 152 | GetIsCrimeFaction | | Faction |
| 153 | CanHaveFlames |
| 154 | HasFlames |
| 155 | ~~---~~
| 156 | ~~---~~
| 157 | GetOpenState |
| 158 | ~~---~~
| 159 | GetSitting |
| 160 | ~~---~~
| 161 | GetIsCurrentPackage | | Package |
| 162 | IsCurrentFurnitureRef | | ObjectReference |
| 163 | IsCurrentFurnitureObj | | Furniture |
| 164 | ~~---~~
| 165 | ~~---~~
| 166 | ~~---~~
| 167 | ~~---~~
| 168 | ~~---~~
| 169 | ~~---~~
| 170 | GetDayOfWeek |
| 171 | ~~---~~
| 172 | GetTalkedToPCParam | | Actor |
| 173 | ~~---~~
| 174 | ~~---~~
| 175 | IsPCSleeping |
| 176 | IsPCAMurderer |
| 177 | ~~---~~
| 178 | ~~---~~
| 179 | ~~---~~
| 180 | HasSameEditorLocAsRef | | ObjectReference | Keyword |
| 181 | HasSameEditorLocAsRefAlias | | Alias | Keyword |
| 182 | GetEquipped | | InventoryItem |
| 183 | ~~---~~
| 184 | ~~---~~
| 185 | IsSwimming |
| 186 | ~~---~~
| 187 | ~~---~~
| 188 | ~~---~~
| 189 | ~~---~~
| 190 | GetAmountGoldStolen |
| 191 | ~~---~~
| 192 | GetIgnoreCrime |
| 193 | GetPCExpelled | | Faction |
| 194 | ~~---~~
| 195 | GetPCFactionMurder | | Faction |
| 196 | ~~---~~
| 197 | GetPCEnemyofFaction | | Faction |
| 198 | ~~---~~
| 199 | GetPCFactionAttack | | Faction |
| 200 | ~~---~~
| 201 | ~~---~~
| 202 | ~~---~~
| 203 | GetDestroyed | Returns 1 if this ref is destroyed, or 0 otherwise. |
| 204 | ~~---~~
| 205 | ~~---~~
| 206 | ~~---~~
| 207 | ~~---~~
| 208 | ~~---~~
| 209 | ~~---~~
| 210 | ~~---~~
| 211 | ~~---~~
| 212 | ~~---~~
| 213 | ~~---~~
| 214 | HasMagicEffect | | MagicEffect |
| 215 | GetDefaultOpen |
| 216 | ~~---~~
| 217 | ~~---~~
| 218 | ~~---~~
| 219 | GetAnimAction |
| 220 | ~~---~~
| 221 | ~~---~~
| 222 | ~~---~~
| 223 | IsSpellTarget | | Spell |
| 224 | GetVATSMode |
| 225 | GetPersuasionNumber |
| 226 | GetVampireFeed | Returns 1 if this actor is a vampire currently feeding on another actor, or 0 otherwise. |
| 227 | GetCannibal | Returns 1 if this actor is a cannibal currently feeding on another actor, or 0 otherwise. |
| 228 | GetIsClassDefault | | Class |
| 229 | GetClassDefaultMatch |
| 230 | GetInCellParam | Returns 1 if the specified ref is in the specified cell, or 0 otherwise. | Cell | ObjectReference |
| 231 | ~~---~~
| 232 | ~~---~~
| 233 | ~~---~~
| 234 | ~~---~~
| 235 | GetVatsTargetHeight |
| 236 | ~~---~~
| 237 | GetIsGhost |
| 238 | ~~---~~
| 239 | ~~---~~
| 240 | ~~---~~
| 241 | ~~---~~
| 242 | GetUnconscious | Returns 1 if this actor is unconscious, or 0 otherwise. |
| 243 | ~~---~~
| 244 | GetRestrained | Returns 1 if this actor is restrained, or 0 otherwise. |
| 245 | ~~---~~
| 246 | GetIsUsedItem | | BaseForm |
| 247 | GetIsUsedItemType | | FormType |
| 248 | IsScenePlaying | | Scene |
| 249 | IsInDialogueWithPlayer |
| 250 | GetLocationCleared | | Location |
| 251 | ~~---~~
| 252 | ~~---~~
| 253 | ~~---~~
| 254 | GetIsPlayableRace |
| 255 | GetOffersServicesNow | Returns 1 if this actor is currently available to barter with, or 0 otherwise. |
| 256 | ~~---~~
| 257 | ~~---~~
| 258 | HasAssociationType | | Actor | AssociationType |
| 259 | HasFamilyRelationship | | Actor |
| 260 | ~~---~~
| 261 | HasParentRelationship | | Actor |
| 262 | IsWarningAbout | | FormList |
| 263 | IsWeaponOut |
| 264 | HasSpell | | Spell |
| 265 | IsTimePassing |
| 266 | IsPleasant | Returns 1 if the current weather is pleasant, or 0 otherwise. |
| 267 | IsCloudy | Returns 1 if the current weather is cloudy, or 0 otherwise. |
| 268 | ~~---~~
| 269 | ~~---~~
| 270 | ~~---~~
| 271 | ~~---~~
| 272 | ~~---~~
| 273 | ~~---~~
| 274 | IsSmallBump |
| 275 | ~~---~~
| 276 | ~~---~~
| 277 | GetBaseActorValue | Returns the base value of the specified ActorValue on this ref. | ActorValue |
| 278 | IsOwner | | OwnerForm |
| 279 | ~~---~~
| 280 | IsCellOwner | Returns 1 if the specified actor or faction owns the specified cell, or 0 otherwise. | Cell | OwnerForm |
| 281 | ~~---~~
| 282 | IsHorseStolen |
| 283 | ~~---~~
| 284 | ~~---~~
| 285 | IsLeftUp |
| 286 | IsSneaking | Returns 1 if this actor is in sneak mode, or 0 otherwise. |
| 287 | IsRunning |
| 288 | GetFriendHit |
| 289 | IsInCombat | | Integer |
| 290 | ~~---~~
| 291 | ~~---~~
| 292 | ~~---~~
| 293 | ~~---~~
| 294 | ~~---~~
| 295 | ~~---~~
| 296 | ~~---~~
| 297 | ~~---~~
| 298 | ~~---~~
| 299 | ~~---~~
| 300 | IsInInterior | Returns 1 if this ref is in an interior cell, or 0 otherwise. |
| 301 | ~~---~~
| 302 | ~~---~~
| 303 | ~~---~~
| 304 | IsWaterObject |
| 305 | GetPlayerAction |
| 306 | IsActorUsingATorch |
| 307 | ~~---~~
| 308 | ~~---~~
| 309 | IsXBox |
| 310 | GetInWorldspace | | Worldspace |
| 311 | ~~---~~
| 312 | GetPCMisCStat | Returns the avlue of the specified misc stat. | MiscStat |
| 313 | GetPairedAnimation |
| 314 | IsActorAVictim |
| 315 | GetTotalPersuasionNumber |
| 316 | ~~---~~
| 317 | ~~---~~
| 318 | GetIdleDoneOnce |
| 319 | ~~---~~
| 320 | GetNoRumors |
| 321 | ~~---~~
| 322 | ~~---~~
| 323 | GetCombatState |
| 324 | ~~---~~
| 325 | GetWithinPackageLocation | | PackageData |
| 326 | ~~---~~
| 327 | IsRidingMount |
| 328 | ~~---~~
| 329 | IsFleeing |
| 330 | ~~---~~
| 331 | ~~---~~
| 332 | IsInDangerousWater | Returns 1 if this actor is standing or swimming in a body of water that has been flagged as dangerous, or 0 otherwise. |
| 333 | ~~---~~
| 334 | ~~---~~
| 335 | ~~---~~
| 336 | ~~---~~
| 337 | ~~---~~
| 338 | GetIgnoreFriendlyHits |
| 339 | IsPlayersLastRiddenMount |
| 340 | ~~---~~
| 341 | ~~---~~
| 342 | ~~---~~
| 343 | ~~---~~
| 344 | ~~---~~
| 345 | ~~---~~
| 346 | ~~---~~
| 347 | ~~---~~
| 348 | ~~---~~
| 349 | ~~---~~
| 350 | ~~---~~
| 351 | ~~---~~
| 352 | ~~---~~
| 353 | IsActor | Returns 1 if this ref is an actor, or 0 otherwise. |
| 354 | IsEssential | Returns 1 if this actor is flagged as essential, or 0 otherwise. |
| 355 | ~~---~~
| 356 | ~~---~~
| 357 | ~~---~~
| 358 | IsPlayerMovingIntoNewSpace |
| 359 | GetInCurrentLoc | | Location |
| 360 | GetInCurrentLocAlias | | Alias |
| 361 | GetTimeDead |
| 362 | HasLinkedRef | | Keyword |
| 363 | ~~---~~
| 364 | ~~---~~
| 365 | IsChild |
| 366 | GetStolenItemValueNoCrime | | Faction |
| 367 | GetPlayerLastAction |
| 368 | IsPlayerActionActive | | Integer*(?)* |
| 369 | ~~---~~
| 370 | IsTalkingActivatorActor | | Actor |
| 371 | ~~---~~
| 372 | IsInList | | FormList |
| 373 | GetStolenIteMValue | | Faction |
| 374 | ~~---~~
| 375 | GetCrimeGoldViolent | | Faction |
| 376 | GetCrimeGoldNonViolent | | Faction |
| 377 | ~~---~~
| 378 | HasShout | | Shout |
| 379 | ~~---~~
| 380 | ~~---~~
| 381 | GetHasNote | | Note |
| 382 | ~~---~~
| 383 | ~~---~~
| 384 | ~~---~~
| 385 | ~~---~~
| 386 | ~~---~~
| 387 | ~~---~~
| 388 | ~~---~~
| 389 | ~~---~~
| 390 | GetHitLocation |
| 391 | IsPC1stPerson |
| 392 | ~~---~~
| 393 | ~~---~~
| 394 | ~~---~~
| 395 | ~~---~~
| 396 | GetCauseOfDeath |
| 397 | IsLimbGone | | Integer |
| 398 | IsWeaponInList | | FormList |
| 399 | ~~---~~
| 400 | ~~---~~
| 401 | ~~---~~
| 402 | IsBribedByPlayer |
| 403 | GetRelationshipRank | | ObjectReference |
| 404 | ~~---~~
| 405 | ~~---~~
| 406 | ~~---~~
| 407 | GetVATSValue | | VATSValueFunction | VATSValue |
| 408 | IsKiller | | Actor |
| 409 | IsKillerObject | | FormList |
| 410 | GetFactionCombatReaction | | Faction | Faction |
| 411 | ~~---~~
| 412 | ~~---~~
| 413 | ~~---~~
| 414 | Exists | Returns 1 if this ref is the specified ref and if the specified ref exists (i.e. isn't None), or 0 otherwise. | ObjectReference |
| 415 | GetGroupMemberCount |
| 416 | GetGroupTargetCount |
| 417 | ~~---~~
| 418 | ~~---~~
| 419 | ~~---~~
| 420 | ~~---~~
| 421 | ~~---~~
| 422 | ~~---~~
| 423 | ~~---~~
| 424 | ~~---~~
| 425 | ~~---~~
| 426 | GetIsVoiceType | Returns 1 if this actor uses the specified voicetype, or 0 otherwise. | Voicetype |
| 427 | GetPlantedExplosive |
| 428 | ~~---~~
| 429 | IsScenePackageRunning |
| 430 | GetHealthPercentage |
| 431 | ~~---~~
| 432 | GetIsObjectType | | FormType |
| 433 | ~~---~~
| 434 | GetDialogueEmotion |
| 435 | GetDialogueEmotionValue |
| 436 | ~~---~~
| 437 | GetIsCreatureType | | Integer*(?)* |
| 438 | ~~---~~
| 439 | ~~---~~
| 440 | ~~---~~
| 441 | ~~---~~
| 442 | ~~---~~
| 443 | ~~---~~
| 444 | GetInCurrentLocFormList | | FormList |
| 445 | GetInZone | | EncounterZone |
| 446 | GetVelocity | | Axis |
| 447 | GetGraphVariableFloat | | String |
| 448 | HasPerk | | Perk | *unknown* |
| 449 | GetFactionRelation | | Actor |
| 450 | IsLastIdlePlayed | | Idle |
| 451 | ~~---~~
| 452 | ~~---~~
| 453 | GetPlayerTeammate |
| 454 | GetPlayerTeammateCount |
| 455 | ~~---~~
| 456 | ~~---~~
| 457 | ~~---~~
| 458 | GetActorCrimePlayerEnemy |
| 459 | GetCrimeGold | | Faction |
| 460 | ~~---~~
| 461 | ~~---~~
| 462 | ~~---~~
| 463 | IsPlayerGrabbedRef | Returns 1 if the player is Z-keying the specified object, or 0 otherwise. | ObjectReference |
| 464 | ~~---~~
| 465 | GetKeywordItemCount | | Keyword |
| 466 | ~~---~~
| 467 | ~~---~~
| 468 | ~~---~~
| 469 | ~~---~~
| 470 | GetDestructionStage |
| 471 | ~~---~~
| 472 | ~~---~~
| 475 | GetIsAlignment | A Fallout leftover. Returns 1 if this actor has the specified karma level, or 0 otherwise. | Alignment |
| 474 | ~~---~~
| 475 | ~~---~~
| 476 | IsProtected | Returns 1 if this actor is flagged as protected, or 0 otherwise. |
| 477 | GetThreatRatio | | Actor |
| 478 | ~~---~~
| 479 | GetIsUsedItemEquipType | | EquipType |
| 480 | ~~---~~
| 481 | ~~---~~
| 482 | ~~---~~
| 483 | ~~---~~
| 484 | ~~---~~
| 485 | ~~---~~
| 486 | ~~---~~
| 487 | IsCarryable |
| 488 | IsConcusssed |
| 489 | ~~---~~
| 490 | ~~---~~
| 491 | GetMapMarkerVisible
| 492 | ~~---~~
| 493 | PlayerKnows | | KnowableForm |
| 494 | GetPermanentActorValue | Returns the "permanent modifier" value of the specified ActorValue. | ActorValue |
| 495 | GetKillingBlowLimb |
| 496 | ~~---~~
| 497 | CanPayCrimeGold |
| 498 | ~~---~~
| 499 | GetDaysInJail |
| 500 | EPAlchemyGetMakingPoison |
| 501 | EPAlchemyEffectHasKeyword | | Keyword |
| 502 | ~~---~~
| 503 | GetAllowWorldInteractions |
| 504 | ~~---~~
| 505 | ~~---~~
| 506 | ~~---~~
| 507 | ~~---~~
| 508 | GetLastHitCritical |
| 509 | ~~---~~
| 510 | ~~---~~
| 511 | ~~---~~
| 512 | ~~---~~
| 513 | IsCombatTarget | | Actor |
| 514 | ~~---~~
| 515 | GetVATSRightAreaFree | | ObjectReference |
| 516 | GetVATSLeftAreaFree | | ObjectReference |
| 517 | GetVATSBackAreaFree | | ObjectReference |
| 518 | GetVATSFrontAreaFree | | ObjectReference |
| 519 | GetLockIsBroken |
| 520 | IsPS3 |
| 521 | IsWin32 |
| 522 | GetVATSRightTargetVisible | | ObjectReference |
| 523 | GetVATSLeftTargetVisible | | ObjectReference |
| 524 | GetVATSBackTargetVisible | | ObjectReference |
| 525 | GetVATSFrontTargetVisible | | ObjectReference |
| 526 | ~~---~~
| 527 | ~~---~~
| 528 | IsInCriticalStage | | CriticalStage |
| 529 | ~~---~~
| 530 | GetXPForNextLevel |
| 531 | ~~---~~
| 532 | ~~---~~
| 533 | GetInfamy | | Faction |
| 534 | GetInfamyViolent | | Faction |
| 535 | GetInfamyNonViolent | | Faction |
| 536 | ~~---~~
| 537 | ~~---~~
| 538 | ~~---~~
| 539 | ~~---~~
| 540 | ~~---~~
| 541 | ~~---~~
| 542 | ~~---~~
| 543 | GetQuestCompleted | | Quest |
| 544 | ~~---~~
| 545 | ~~---~~
| 546 | ~~---~~
| 547 | IsGoreDisabled |
| 548 | ~~---~~
| 549 | ~~---~~
| 550 | IsSceneActionComplete | *(The integer is an action index.)* | Scene | Integer |
| 551 | ~~---~~
| 552 | GetSpellUsageNum | | Spell |
| 553 | ~~---~~
| 554 | GetActorsInHigh | Returns the number of actors currently in "high" AI processing. |
| 555 | HasLoaded3D | Returns 1 if this ref has any 3D loaded, or 0 otherwise. |
| 556 | ~~---~~
| 557 | ~~---~~
| 558 | ~~---~~
| 559 | ~~---~~
| 560 | HasKeyword | Returns 1 if this ref's base form has the specified keyword, or 0 otherwise. | Keyword |
| 561 | HasRefType | | LocRefType |
| 562 | LocationHasKeyword | Returns 1 if this location has the specified keyword, or 0 otherwise. | Keyword |
| 563 | LocationHasRefType | | LocRefType |
| 564 | ~~---~~
| 565 | GetIsEditorLocation | | Location |
| 566 | GetIsAliasRef | | Alias |
| 567 | GetIsEditorLocAlias | | Alias |
| 568 | IsSprinting | Returns 1 if this actor is sprinting, or 0 otherwise. |
| 569 | IsBlocking | Returns 1 if this actor is blocking, or 0 otherwise. |
| 570 | HasEquippedSpell | | CastingSource |
| 571 | GetCurrentCastingType | | CastingSource |
| 572 | GetCurrentDeliveryType | | CastingSource |
| 573 | ~~---~~
| 574 | GetAttackState |
| 575 | ~~---~~
| 576 | GetEventData | *(Function uses event data.)* |
| 577 | IsCloserToAThanB | Returns 1 if this ref is closer to the first argument than it is to the second argument, or 0 otherwise. | ObjectReference | ObjectReference |
| 578 | ~~---~~
| 579 | GetEquippedShout | | Shout |
| 580 | IsBleedingOut |
| 581 | ~~---~~
| 582 | ~~---~~
| 583 | ~~---~~
| 584 | GetRelativeAngle | | ObjectReference | Axis |
| 585 | ~~---~~
| 586 | ~~---~~
| 587 | ~~---~~
| 588 | ~~---~~
| 589 | GetMovementDirection |
| 590 | IsInScene |
| 591 | GetRefTypeDeadCount | | Location | LocRefType |
| 592 | GetRefTypeAliveCount | | Location | LocRefType |
| 593 | ~~---~~
| 594 | GetIsFlying |
| 595 | IsCurrentSpell | | Spell | CastingSource |
| 596 | SpellHasKeyword | | CastingSource | Keyword |
| 597 | GetEquippedItemType | | CastingSource |
| 598 | GetLocationAliasCleared | | Alias |
| 599 | ~~---~~
| 600 | GetLocAliasRefTypeDeadCount | | Alias | LocRefType |
| 601 | GetLocAliasRefTypeAliveCount | | Alias | LocRefType |
| 602 | IsWardState | | WardState |
| 603 | IsInSameCurrentLocAsRef | | ObjectReference | Keyword |
| 604 | IsInSameCurrentLocAsRefAlias | | Alias | Keyword |
| 605 | LocAliasIsLocation | | Alias | Location |
| 606 | GetKeywordDataForLocation | | Location | Keyword |
| 607 | ~~---~~
| 608 | GetKeywordDataForAlias | | Alias | Keyword |
| 609 | ~~---~~
| 610 | LocAliasHasKeyword | | Alias | Keyword
| 611 | IsNullPackageData | | PackageData |
| 612 | GetNumericPackageData | | Integer*(?)* |
| 613 | IsFurnitureAnimType | | FurnitureAnim |
| 614 | IsFurnitureEntryType | | FurnitureEntry |
| 615 | GetHighestRelationshipRank |
| 616 | GetLowestRelationshipRank |
| 617 | HasAssociationTypeAny | | AssociationType |
| 618 | HasFamilyRelationshipAny |
| 619 | GetPathingTargetOffset | | Axis |
| 620 | GetPathingTargetAngleOffset | | Axis |
| 621 | GetPathingTargetSpeed |
| 622 | GetPathingTargetSpeedAngle | | Axis |
| 623 | GetMovementSpeed |
| 624 | GetInContainer | | ObjectReference |
| 625 | IsLocationLoaded | | Location |
| 626 | IsLocAliasLoaded | | Alias |
| 627 | IsDualCasting |
| 628 | ~~---~~
| 629 | GetVMQuestVariable | | Quest | String |
| 630 | GetVMScriptVariable | | ObjectReference | String |
| 631 | IsEnteringInteractionQuick |
| 632 | IsCasting |
| 633 | GetFlyingState |
| 634 | ~~---~~
| 635 | IsInFavorState |
| 636 | HasTwoHandedWeaponEquipped |
| 637 | IsExitingInstant |
| 638 | IsInFriendStateWithPlayer |
| 639 | GetWithinDistance | | ObjectReference | Float |
| 640 | GetActorValuePercent | | ActorValue |
| 641 | IsUnique |
| 642 | GetLastBumpDirection |
| 643 | ~~---~~
| 644 | IsInFurnitureState | | FurnitureAnim |
| 645 | GetIsInjured |
| 646 | GetIsCrashLandRequest |
| 647 | GetIsHastyLandRequest |
| 648 | ~~---~~
| 649 | ~~---~~
| 650 | IsLinkedTo | | ObjectReference | Keyword |
| 651 | GetKeywordDataForCurrentLocation | | Keyword |
| 652 | GetInSharedCrimeFaction | | ObjectReference |
| 653 | ~~---~~
| 654 | GetBribeSuccess |
| 655 | GetIntimidateSuccess |
| 656 | GetArrestedState |
| 657 | GetArrestingActor |
| 658 | ~~---~~
| 659 | EPTemperingItemIsEnchanted |
| 660 | EPTemperingItemHasKeyword |
| 661 | ~~---~~
| 662 | ~~---~~
| 663 | ~~---~~
| 664 | GetReplacedItemType | | CastingSource |
| 665 | ~~---~~
| 666 | ~~---~~
| 667 | ~~---~~
| 668 | ~~---~~
| 669 | ~~---~~
| 670 | ~~---~~
| 671 | ~~---~~
| 672 | IsAttacking |
| 673 | IsPowerAttacking |
| 674 | IsLastHostileActor |
| 675 | GetGraphVariableInt | | String |
| 676 | GetCurrentShoutVariation |
| 677 | ~~---~~
| 678 | ShouldAttackKill | | Actor |
| 679 | ~~---~~
| 680 | ~~---~~
| 681 | EPMagic_IsAdvanceSkill | | ActorValue |
| 682 | WornHasKeyword | | Keyword |
| 683 | GetPathingCurrentSpeed |
| 684 | GetPathingCurrentSpeedAngle | | Axis |
| 685 | ~~---~~
| 686 | ~~---~~
| 687 | ~~---~~
| 688 | ~~---~~
| 689 | ~~---~~
| 690 | ~~---~~
| 691 | EPModSkillUsage_AdvanceObjectHasKeyword | | Keyword |
| 692 | EPModSkillUsage_IsAdvanceAction | | AdvanceAction |
| 693 | EPMagic_SpellHasKeyword | | Keyword |
| 694 | GetNoBleedoutRecovery |
| 695 | ~~---~~
| 696 | EPMagic_SpellHasSkill | | ActorValue |
| 697 | IsAttackType | | Keyword |
| 698 | IsAllowedToFly |
| 699 | HasMagicEffectKeyword | | Keyword |
| 700 | IsCommandedActor |
| 701 | IsStaggered |
| 702 | IsRecoiling |
| 703 | IsExitingInteractionQuick |
| 704 | IsPathing |
| 705 | GetShouldHelp | | Actor |
| 706 | HasBoundWeaponEquipped | | CastingSource |
| 707 | GetCombatTargetHasKeyword | | Keyword |
| 708 | ~~---~~
| 709 | GetCombatGroupMemberCount |
| 710 | IsIgnoringCombat |
| 711 | GetLightLevel |
| 712 | ~~---~~
| 713 | SpellHasCastingPerk |
| 714 | IsBeingRidden |
| 715 | IsUndead |
| 716 | GetRealHoursPassed |
| 717 | ~~---~~
| 718 | IsUnlockedDoor |
| 719 | IsHostileToActor | | Actor |
| 720 | GetTargetHeight | | ObjectReference |
| 721 | IsPoison |
| 722 | WornApparelHasKeywordCount | | Keyword |
| 723 | GetItemHealthPercent |
| 724 | EffectWasDualCast |
| 725 | GetKnockedStateEnum |
| 726 | DoesNotExist |
| 727 | ~~---~~
| 728 | ~~---~~
| 729 | ~~---~~
| 730 | IsOnFlyingMount |
| 731 | CanFlyHere |
| 732 | IsFlyingMountPatrolQueued |
| 733 | IsFlyingMountFastTravelling |
| **SSE** 734 | IsOverencumbered |
| **SSE** 735 | GetActorWarmth |
| **SKSE** 1024 | GetSKSEVersion |
| **SKSE** 1025 | GetSKSEVersionMinor |
| **SKSE** 1026 | GetSKSEVersionBeta |
| **SKSE** 1027 | GetSKSERelease |
| **SKSE** 1028 | ~~ClearInvalidRegistrations~~ |