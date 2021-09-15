/*

   This file contains data mined from The Elder Scrolls: Skyrim, including text strings.
   What pieces of text are long and distinctive enough to be copyrightable are the sole
   intellectual property of Bethesda Game Studios. DovahKit's author is operating under
   the good-faith belief that the project's use of this data-mined text falls under fair
   use -- that DovahKit as a whole is sufficiently transformative.

   The portions of this document that do not consist of data mined from Skyrim are code
   provided under the Creative Commons 0 License; they are public domain or the closest
   legal equivalent.
   <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
   <https://creativecommons.org/publicdomain/zero/1.0/>

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "ini_settings.h"
#include <cassert>

namespace dovah::game_ini {
   #pragma region Class internals
   extern setting_type get_setting_type_from_name(const char* name) {
      if (!name)
         return setting_type::none;
      switch (name[0]) {
         case 'b':
         case 'B':
            return setting_type::boolean;
         case 'f':
         case 'F':
            return setting_type::float32;
         case 'i':
         case 'I':
            return setting_type::integer;
         case 's':
         case 'S':
            return setting_type::string;
      }
      return setting_type::none;
   }

   setting_definition::setting_definition(const char* n, bool value) : name(n), type(setting_type::boolean) {
      this->default_value.b = value;
   }
   setting_definition::setting_definition(const char* n, float value) : name(n), type(setting_type::float32) {
      this->default_value.f = value;
   }
   setting_definition::setting_definition(const char* n, int32_t value) : name(n), type(setting_type::integer) {
      this->default_value.i = value;
   }
   setting_definition::setting_definition(const char* n, const char* value) : name(n), type(setting_type::string) {
      this->default_value.s = value;
   }

   void setting_definition::_set_games(std::initializer_list<game>& g) {
      this->games.skyrim_classic = false;
      this->games.skyrim_special = false;
      for (auto v : g) {
         switch (v) {
            case game::skyrim_classic: this->games.skyrim_classic = true; break;
            case game::skyrim_special: this->games.skyrim_special = true; break;
            default:
               assert(false && "dovah::game_ini::setting_definition: Unrecognized game passed to constructor!");
               //
               // Add a member to (setting_definition::games), and then reset it to (false) at the start of 
               // this function and add a case for it to this switch. Be sure to update the (exists_in_game) 
               // member function, too!
               //
         }
      }
   }

   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, bool value) : setting_definition(n, value) {
      this->_set_games(g);
   }
   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, float value) : setting_definition(n, value) {
      this->_set_games(g);
   }
   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, int32_t value) : setting_definition(n, value) {
      this->_set_games(g);
   }
   setting_definition::setting_definition(std::initializer_list<game> g, const char* n, const char* value) : setting_definition(n, value) {
      this->_set_games(g);
   }

   bool setting_definition::exists_in_game(game g) const noexcept {
      switch (g) {
         case game::skyrim_classic: return this->games.skyrim_classic; break;
         case game::skyrim_special: return this->games.skyrim_special; break;
      }
      return false;
   }

   section_definition::section_definition(const char* name, std::initializer_list<setting_definition> settings) : name(name), settings(settings) {
   }

   file_definition::file_definition(const char* name, std::initializer_list<section_definition> sections) : filename(name), sections(sections) {
   }
   const setting_definition* file_definition::lookup(const char* section, const char* setting) const noexcept {
      for (auto& a : this->sections) {
         if (stricmp(a.name.c_str(), section) != 0)
            continue;
         for (auto& b : a.settings) {
            if (stricmp(b.name, setting) == 0)
               return &b;
         }
         return nullptr;
      }
      return nullptr;
   }
   #pragma endregion

   namespace files {
      /*//
      extern const file_definition skyrim = file_definition("Skyrim.ini", {
         section_definition("Landscape", {
            { "sDefaultLandDiffuseTexture", "Dirt02.dds" },
            { "sDefaultLandNormalTexture",  "Dirt02_N.dds" },
         }),
      });
      //
      // TODO: Dump all INI settings from the executable, along with their program-level defaults.
      //
      //*/
      #pragma region Skyrim.ini

      static_assert(false, "TODO: When we generate these definitions, don't group settings within a type into alphabetized regions (i.e. category > type > letter) if there are fewer than 8 settings for that type");
      static_assert(false, "TODO: Investigate placing SSE-only settings at the bottom of each category.");
      static_assert(false, "TODO: Investigate giving setting_definition a constexpr constructor; will that prevent warning C6262 (too much stack space used)?");
      static_assert(false, "TODO: The constructor form that takes a game list can be made constexpr, with compile-time validity checks, if we use a bitfield on the setting for games and use templating to map between game constants and bit indices.");


      extern const file_definition skyrim = file_definition("Skyrim", {
         section_definition("", {
            #pragma region Booleans
               #pragma region E
                  { "bEnableLipLookup", true },
               #pragma endregion
               #pragma region P
                  { "bPrimitivesOn", false },
               #pragma endregion
               #pragma region U
                  { "bUseWaterHDR", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region F
                  { { game::skyrim_special }, "fFadingFracStart", 0.25F },
               #pragma endregion
               #pragma region K
                  { "fKeyboardRepeatDelay", 0.30000001192092896F },
                  { "fKeyboardRepeatRate", 0.05000000074505806F },
               #pragma endregion
               #pragma region L
                  { "fLowPerfCombatantVoiceDistance", 1000.0F },
               #pragma endregion
               #pragma region M
                  { "fMapWorldTargetTransitionTime", 0.5F },
               #pragma endregion
               #pragma region S
                  { { game::skyrim_special }, "fSnowSSSDarkColorIntensity", 0.10000000149011612F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region D
                  { "iDetectionHighNumPicks", 40 },
               #pragma endregion
               #pragma region L
                  { "iLastHDRSetting", -1 },
               #pragma endregion
               #pragma region M
                  { "iMaxQuestObjectives", 3000 },
               #pragma endregion
               #pragma region S
                  { { game::skyrim_special }, "iSnowNoiseTextureSize", 512 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region C
                  { "sControlsDefinitionFile", "Interface/Controls/PC/ControlMap.txt" },
                  { "sControlsRemapFile", "ControlMap_Custom.txt" },
               #pragma endregion
               #pragma region G
                  { "sGamepadDefinitionFile", "Interface/Controls/PC/Gamepad.txt" },
               #pragma endregion
               #pragma region K
                  { "sKeyboardDefinitionFile", "Interface/Controls/PC/Keyboard_" },
               #pragma endregion
               #pragma region M
                  { "sMouseDefinitionFile", "Interface/Controls/PC/Mouse.txt" },
               #pragma endregion
               #pragma region S
                  { "sSaveGameScreenshotName", "BGSSaveLoadHeader_Screenshot" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Actor", {
            { "bUseNavMeshForMovement", true },
            { "fNotVisibleNavmeshMoveDist", 2048.0F },
            { "fVisibleNavmeshMoveDist", 4096.0F },
         }),
         section_definition("Animation", {
            #pragma region Booleans
               #pragma region A
                  { "bAlwaysDriveRagdoll", false },
                  { "bAlwaysSaveAllInfo", true },
                  { "bAnimInterpEnable", true },
                  { "bApplyPitchToExtractedMotion", true },
               #pragma endregion
               #pragma region D
                  { "bDisplayMarkWarning", false },
                  { { game::skyrim_special }, "bDrawAnimPoseInVDB", false },
                  { "bDriveRagdollWithGraph", true },
               #pragma endregion
               #pragma region E
                  { "bEnableHavokHit", false },
               #pragma endregion
               #pragma region F
                  { "bFeedbackToGraphFromCharacterController", true },
                  { "bFootIK", true },
                  { "bFootIKFeedback", true },
               #pragma endregion
               #pragma region H
                  { "bHumanoidFootIKEnable", true },
               #pragma endregion
               #pragma region I
                  { "bInitiallyLoadAllClips", false },
               #pragma endregion
               #pragma region L
                  { "bLoadCollatedAnimTextData", true },
               #pragma endregion
               #pragma region M
                  { "bMultiThreadBoneUpdate", true },
               #pragma endregion
               #pragma region R
                  { "bRandomizeGraphSeed", true },
               #pragma endregion
               #pragma region S
                  { "bSendNonVisibleBehaviorGraphsToSPU", true },
                  { "bShouldProcessRequests", true },
               #pragma endregion
               #pragma region U
                  { "bUseSPUGenerate", false },
                  { "bUseSpeedSampler", true },
                  { "bUseVariableCache", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fAnimInterpFarDist", 800.0F },
                  { "fAnimInterpMaxTime", 0.25F },
                  { "fAnimInterpMinTime", 0.07999999821186066F },
                  { "fAnimInterpNearDist", 400.0F },
                  { "fAnimInterpSlop", 0.25F },
               #pragma endregion
               #pragma region B
                  { "fBoneLODDistanceScale", 1.0F },
               #pragma endregion
               #pragma region H
                  { "fHavokHitImpulseMult", 50.0F },
                  { "fHavokHitTimeSec", 3.0F },
               #pragma endregion
               #pragma region I
                  { "fIdleChangeClearTime", 1.0F },
               #pragma endregion
               #pragma region M
                  { "fMaxFrameCounterDifferenceToConsiderVisible", 0.06666667014360428F },
                  { "fMaxTimeToMarkSec", 3.0F },
                  { "fMotionFeedbackMinAngleDelta", 20.0F },
                  { "fMotionFeedbackMinSpeed", 5.0F },
                  { "fMotionFeedbackMinSpeedDelta", 25.0F },
                  { "fMotionFeedbackMinTime", 0.5F },
                  { "fMountDismountTimeout", 8.0F },
               #pragma endregion
               #pragma region P
                  { "fPlayerCharacterAttackComboStartFraction", 0.5F },
                  { "fPlayerCharacterAttackIntroLength", 0.0F },
                  { "fPlayerCharacterDrawSheatheTimeout", 3.0F },
                  { "fPlayerCharacterPowerAttackStartTime", 0.36666667461395264F },
               #pragma endregion
               #pragma region S
                  { "fSpecialIdlePickTime", 250.0F },
               #pragma endregion
               #pragma region W
                  { "fWeaponChangeClearTime", 0.5F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region M
                  { "iMinBonesToGenerateWhileSitting", 5 },
               #pragma endregion
               #pragma region P
                  { "iPlayerCharacterImagespaceModifierAnimCount", 2 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region H
                  { "sHkxDBNameContextPrefix", "meshes\\" },
               #pragma endregion
               #pragma region T
                  { "strPlayerCharacterBehavior1stPGraph", "Actors\\Character\\_1stPerson\\FirstPerson.hkx" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Archive", {
            #pragma region Booleans
               #pragma region C
                  { "bCheckRuntimeCollisions", false },
               #pragma endregion
               #pragma region F
                  { { game::skyrim_special }, "bForceAsync", false },
               #pragma endregion
               #pragma region I
                  { "bInvalidateOlderFiles", true },
               #pragma endregion
               #pragma region L
                  { { game::skyrim_special }, "bLoadArchiveInMemory", false },
                  { { game::skyrim_special }, "bLoadEsmInMemory", true },
               #pragma endregion
               #pragma region T
                  { "bTrackFileLoading", false },
               #pragma endregion
               #pragma region U
                  { "bUseArchives", true },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region R
                  { "iRetainDirectoryStringTable", 1 },
                  { "iRetainFilenameOffsetTable", 1 },
                  { "iRetainFilenameStringTable", 1 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region A
                  { "sArchiveList", "Skyrim - Textures.bsa, Skyrim - Meshes.bsa, Skyrim - Voices.bsa" },
                  { { game::skyrim_special }, "sArchiveToLoadInMemoryList", "Skyrim - Animations.bsa, Skyrim - Interface.bsa, Skyrim - Misc.bsa, Skyrim - Sounds.bsa" },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "sEsmToLoadInMemoryList", "Skyrim.esm, Update.esm, Dawnguard.esm, HearthFires.esm, Dragonborn.esm" },
               #pragma endregion
               #pragma region I
                  { "sInvalidationFile", "ArchiveInvalidation.txt" },
               #pragma endregion
               #pragma region R
                  { "sResourceArchiveList", "SKYRIM - MISC.BSA, SKYRIM - SHADERS.BSA, SKYRIM - TEXTURES.BSA, SKYRIM - MESHES.BSA, SKYRIM - ANIMATIONS.BSA, SKYRIM - VOICES.BSA, SKYRIM - VOICES2.BSA, SKYRIM - INTERFACE.BSA, SKYRIM - SOUNDS.BSA" },
                  { "sResourceArchiveList2", "Not enough arguments..." },
                  { "sResourceArchiveListBeta", "Not enough arguments..." },
                  { "sResourceArchiveRetainFileNameList", "Not enough arguments..." },
                  { "sResourcePrefixList", "TEXTURES\\, MESHES\\, FACEGEN\\, INTERFACE\\ , MUSIC\\, SOUND\\, SCRIPTS\\, MAXHEIGHTS\\, VIS\\, GRASS\\, STRINGS\\" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Audio", {
            #pragma region Booleans
               #pragma region E
                  { "bEnableAudio", true },
                  { "bEnableAudioCache", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fASFadeInTime", 3.0F },
                  { "fASFadeOutTime", 8.0F },
                  { { game::skyrim_special }, "fAudioRumbleBigDeadZone", 0.0F },
                  { "fAudioRumbleBigExponent", 0.5F },
                  { { game::skyrim_special }, "fAudioRumbleBigLerpMax", 1.0F },
                  { { game::skyrim_special }, "fAudioRumbleBigLerpMin", 0.0F },
                  { "fAudioRumblePowerAttackAdj", 0.15000000596046448F },
                  { { game::skyrim_special }, "fAudioRumbleSmallDeadZone", 0.0F },
                  { "fAudioRumbleSmallExponent", 0.4000000059604645F },
                  { { game::skyrim_special }, "fAudioRumbleSmallLerpMax", 1.0F },
                  { { game::skyrim_special }, "fAudioRumbleSmallLerpMin", 0.0F },
               #pragma endregion
               #pragma region C
                  { "fCollisionSoundHeavyThreshold", 160.0F },
               #pragma endregion
               #pragma region D
                  { "fDefaultMasterVolume", 1.0F },
                  { "fDialogueHeadPitchExaggeration", 2.0F },
                  { "fDialogueHeadRollExaggeration", 2.0F },
                  { "fDialogueHeadYawExaggeration", 2.0F },
               #pragma endregion
               #pragma region H
                  { "fHardLandingDamageThreshold", 500.0F },
                  { "fHighlightSpeechOverlap", 0.5F },
               #pragma endregion
               #pragma region M
                  { "fMaxHighlightRadius", 250.0F },
                  { "fMenuModeFadeInTime", 0.5F },
                  { "fMenuModeFadeOutTime", 0.5F },
                  { "fMinSoundVel", 60.0F },
                  { "fMusicDuckingSeconds", 1.0F },
                  { "fMusicFinaleCrossFadeTimeIn", 3.0F },
                  { "fMusicFinaleCrossFadeTimeOut", 6.0F },
                  { "fMusicUnDuckingSeconds", 4.0F },
               #pragma endregion
               #pragma region N
                  { "fNonDialogVoiceDucking", 9.0F },
                  { "fNonDialogVoiceDuckingFadeIn", 1.0F },
                  { "fNonDialogVoiceDuckingFadeOut", 1.5F },
                  { "fNonHighlightSpeechAtten", 12.0F },
               #pragma endregion
               #pragma region R
                  { "fRegionLoopFadeInTime", 6.0F },
                  { "fRegionLoopFadeOutTime", 8.0F },
                  { "fRegionRandomSoundPlacementBase", 100.0F },
                  { "fRegionSoundPlacementRandomOffset", 650.0F },
                  { "fRegionSoundPlacementZOffset", 256.0F },
                  { "fReverbTransitionTime", 0.5F },
               #pragma endregion
               #pragma region W
                  { "fWaterAudioFadeInSeconds", 3.0F },
                  { "fWaterAudioFadeOutSeconds", 5.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region C
                  { "iCollisionSoundTimeDelta", 150 },
               #pragma endregion
               #pragma region H
                  { "iHighlightSpeechOverlap", 500 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region A
                  { "sAudioAPI", "XAudio2" },
               #pragma endregion
               #pragma region D
                  { "sDeathCameraEffect", "MAGShoutSlowTimeActiveLP" },
               #pragma endregion
               #pragma region M
                  { "sMissingAssetSoundFile", "Not enough arguments..." },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("BackgroundLoad", {
            { "bBackgroundCellLoads", true },
            { "bBackgroundLoadLipFiles", false },
            { "bLoadBackgroundFaceGen", false },
            { "bLoadHelmetsInBackground", true },
            { "bSelectivePurgeUnusedOnFastTravel", false },
            { "bUseBackgroundFileLoader", false },
            { "bUseMultiThreadedFaceGen", true },
            { "bUseMultiThreadedTrees", true },
            { "iPostProcessMilliseconds", 5 },
            { "iPostProcessMillisecondsEditor", 50 },
            { "iPostProcessMillisecondsLoadingQueuedPriority", 20 },
            { "iPostProcessTaskWarningMilliseconds", 20 },
         }),
         section_definition("Bethesda.net", {
            #pragma region Booleans
               #pragma region A
                  { { game::skyrim_special }, "bAutoSkipMainMenuLogin", true },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "bEnableLocalLogging", false },
                  { { game::skyrim_special }, "bEnablePlatform", true },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "bModShowDownloadInsteadOfInstallLimit", false },
               #pragma endregion
               #pragma region T
                  { { game::skyrim_special }, "bTESTGameDataWait", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region E
                  { { game::skyrim_special }, "fEULATimeOutSeconds", 30.0F },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "fModIntroWarningDisplayTime", 5.0F },
                  { { game::skyrim_special }, "fModLimitVisibilityThreshold", 0.8500000238418579F },
               #pragma endregion
               #pragma region S
                  { { game::skyrim_special }, "fSteamPollDuration", 90.0F },
                  { { game::skyrim_special }, "fSteamPollInterval", 10.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region B
                  { { game::skyrim_special }, "iBethesdaEntitlementsProductId", 0 },
                  { { game::skyrim_special }, "iBethesdaLoggingProductId", 0 },
                  { { game::skyrim_special }, "iBethesdaMotdProductId", 0 },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "iEnableEventLogging", -1 },
                  { { game::skyrim_special }, "iEnableLogging", -1 },
               #pragma endregion
               #pragma region I
                  { { game::skyrim_special }, "iIsLiveEventLogging", -1 },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "iMinHttpResonseStatusToLog", 0 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region A
                  { { game::skyrim_special }, "sAccountsURL", "Not enough arguments..." },
               #pragma endregion
               #pragma region B
                  { { game::skyrim_special }, "sBethesdaBeamKey", "Not enough arguments..." },
                  { { game::skyrim_special }, "sBethesdaLoggingKey", "Not enough arguments..." },
                  { { game::skyrim_special }, "sBethesdaOAuthId", "Not enough arguments..." },
               #pragma endregion
               #pragma region C
                  { { game::skyrim_special }, "sCreationsDownloadDirectory", "Creations" },
                  { { game::skyrim_special }, "sCreationsInstallDirectory", "Not enough arguments..." },
               #pragma endregion
               #pragma region D
                  { { game::skyrim_special }, "sDeveloperEmail", "Not enough arguments..." },
                  { { game::skyrim_special }, "sDeveloperUsername", "Not enough arguments..." },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "sEnvironment", "Auto" },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "sModsDownloadDirectory", "Mods" },
                  { { game::skyrim_special }, "sModsInstallDirectory", "Not enough arguments..." },
                  { { game::skyrim_special }, "sModsURL", "Not enough arguments..." },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("bLightAttenuation", {
            { "bOutQuadInLin", false },
            { "bUseConstant", false },
            { "bUseLinear", false },
            { "bUseQuadratic", true },
            { "fConstantValue", 0.0F },
            { "fLinearRadiusMult", 1.0F },
            { "fLinearValue", 3.0F },
            { "fQuadraticRadiusMult", 1.0F },
            { "fQuadraticValue", 16.0F },
         }),
         section_definition("BSPathing", {
            { "fFindClosestEdgesRadius", 512.0F },
         }),
         section_definition("BudgetCaps", {
            { "fMaxMsUsagePerFrame", 28.0F },
            { "fMsActiveRefCount", 0.05000000074505806F },
            { "fMsActorRefCount", 0.24500000476837158F },
            { "fMsAnimatedObjectsCount", 0.05000000074505806F },
            { "fMsDecalCount", 0.0010000000474974513F },
            { "fMsEmittersCount", 0.009999999776482582F },
            { "fMsGeometryCount", 0.009999999776482582F },
            { "fMsHavokTriCount", 0.0010000000474974513F },
            { "fMsLightCount", 0.009999999776482582F },
            { "fMsLightExcessGeometry", 0.009999999776482582F },
            { "fMsParticlesCount", 0.0010000000474974513F },
            { "fMsRefCount", 0.020999999716877937F },
            { "fMsTriangleCount", 0.00009999999747378752F },
            { "fMsWaterCount", 0.10000000149011612F },
         }),
         section_definition("Camera", {
            #pragma region Booleans
               #pragma region D
                  { "bDisableAutoVanityMode", false },
                  { "bDragonCameraTargetPlayer", true },
               #pragma endregion
               #pragma region F
                  { { game::skyrim_special }, "bForceAutoVanityMode", false },
               #pragma endregion
               #pragma region R
                  { "bReturnTo1stPersonFromVanity", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region 1
                  { "f1st3rdSwitchDelay", 0.25F },
                  { { game::skyrim_special }, "f1stHorzDampeningSpringConstant", 0.00009999999747378752F },
                  { { game::skyrim_special }, "f1stHorzDampeningVelocityDampening", 0.4000000059604645F },
                  { "f1stPitchOffsetMouseFollowSpeed", 15.0F },
                  { "f1stPitchOffsetMouseMaxLag", 4.0F },
                  { "f1stPitchOffsetMultOffAccel", 1.0F },
                  { "f1stPitchOffsetMultOffMaxSpeed", 1.0F },
                  { "f1stPitchOffsetMultOnAccel", 0.5F },
                  { "f1stPitchOffsetMultOnMaxSpeed", 0.6000000238418579F },
                  { "f1stPitchOffsetTarget", 0.75F },
                  { { game::skyrim_special }, "f1stVertDampeningSpringConstant", 0.0010000000474974513F },
                  { { game::skyrim_special }, "f1stVertDampeningVelocityDampening", 0.6000000238418579F },
               #pragma endregion
               #pragma region A
                  { "fActorFadeOutLimit", 30.0F },
                  { "fAutoVanityIncrement", 0.009999999776482582F },
                  { "fAutoVanityModeDelay", 120.0F },
               #pragma endregion
               #pragma region C
                  { "fCameraCasterBleedOutSize", 5.0F },
                  { "fCameraCasterSize", 15.0F },
                  { "fCameraCasterTargetSize", 20.0F },
                  { "fCharControllerCheckHeightOffset", 124.0F },
                  { "fChaseCameraMaxAngle", 30.0F },
                  { "fChaseCameraSpeed", 10.0F },
                  { "fCollisionRecoveryMinDist", 4000.0F },
                  { "fCollisionRecoverySpeed", 3.0F },
               #pragma endregion
               #pragma region D
                  { "fDefaultAutoVanityZoom", 300.0F },
                  { "fDragonCameraDollyTime", 0.75F },
                  { "fDragonMaxAngleBeforeTurn", 90.0F },
               #pragma endregion
               #pragma region F
                  { "fFirstPersonDisablePOVLerpDPS", 2.0F },
                  { "fFirstPersonSittingAngleLimit", 1.5707963705062866F },
                  { "fFirstPersonSittingRotationSpeed", 0.10000000149011612F },
                  { "fFreeCameraRotationSpeed", 3.0F },
                  { "fFreeCameraRunSpeed", 2.0F },
                  { "fFreeCameraTranslationSpeed", 20.0F },
                  { "fFreeCameraTriggerDeadzone", 0.10000000149011612F },
                  { "fFreeRotationSpeed", 3.0F },
                  { "fFurnitureCameraAngle", 0.39269909262657166F },
                  { "fFurnitureCameraZoom", 250.0F },
               #pragma endregion
               #pragma region H
                  { "fHorseDismountYawCorrection", 0.3199999928474426F },
                  { "fHorseMaxAngleBeforeTurn", 90.0F },
               #pragma endregion
               #pragma region L
                  { "fLookingSpeed", 0.10000000149011612F },
               #pragma endregion
               #pragma region M
                  { "fMinCurrentZoom", -0.20000000298023224F },
                  { "fMouseWheelZoomIncrement", 0.07500000298023224F },
                  { "fMouseWheelZoomMinDelta", 0.004999999888241291F },
                  { "fMouseWheelZoomSpeed", 0.800000011920929F },
               #pragma endregion
               #pragma region O
                  { "fOverShoulderCombatAddY", -100.0F },
                  { "fOverShoulderCombatPosX", 0.0F },
                  { "fOverShoulderCombatPosZ", 20.0F },
                  { "fOverShoulderDragonAddY", -600.0F },
                  { "fOverShoulderDragonPosX", 0.0F },
                  { "fOverShoulderDragonPosZ", 0.0F },
                  { "fOverShoulderHorseAddY", -300.0F },
                  { "fOverShoulderHorsePosX", 0.0F },
                  { "fOverShoulderHorsePosZ", 0.0F },
                  { "fOverShoulderPosX", 30.0F },
                  { "fOverShoulderPosZ", -10.0F },
               #pragma endregion
               #pragma region P
                  { "fPitchZeroBlendTime", 0.75F },
                  { "fPitchZoomOutMaxDist", 100.0F },
               #pragma endregion
               #pragma region S
                  { "fShoulderDollySpeed", 3.0F },
                  { { game::skyrim_special }, "fSlowVanityIncrement", 0.019999999552965164F },
               #pragma endregion
               #pragma region T
                  { "fThumbstickZoomSpeed", 0.05000000074505806F },
                  { "fTweenCamRotAngle", 0.05000000074505806F },
                  { "fTweenCamRotClosingSpeed", 10.0F },
                  { "fTweenCamRotSpeed", 4.0F },
                  { "fTweenCamZoomFOVMod", 10.0F },
                  { "fTweenCamZoomSpeed", 25.0F },
               #pragma endregion
               #pragma region V
                  { "fVanityModeMaxDist", 600.0F },
                  { "fVanityModeMaxDistFlyMult", 4.0F },
                  { "fVanityModeMinDist", 155.0F },
                  { "fVanityModeMinDistFly", 1200.0F },
               #pragma endregion
               #pragma region W
                  { "fWorkbenchCameraPitch", 0.5F },
                  { "fWorkbenchCameraTranslateX", 0.0F },
                  { "fWorkbenchCameraTranslateY", 50.0F },
                  { "fWorkbenchCameraTranslateZ", -50.0F },
                  { "fWorkbenchCameraYaw", -1.0F },
                  { "fWorkbenchCameraZoom", 100.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region A
                  { "iAnimatedTransitionMillis", 1000 },
               #pragma endregion
               #pragma region B
                  { "iBleedoutTransitionMillis", 500 },
               #pragma endregion
               #pragma region H
                  { "iHorseTransitionMillis", 500 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("CameraPath", {
            { "bRecord", false },
            { "bStart", false },
            { "iFPS", 30 },
            { "iTake", 0 },
            { "sDirectoryName", "TestCameraPath" },
            { "sNif", "Cameras\\CameraTest.nif" },
            { "sOffsetID", "Not enough arguments..." },
         }),
         section_definition("Cart", {
            #pragma region Floats
               #pragma region C
                  { "fCartLimitMax", 0.75F },
                  { "fCartLimitMin", -0.75F },
                  { "fCartPivotX", 0.0F },
                  { "fCartPivotY", 3.0F },
                  { "fCartPivotZ", 0.699999988079071F },
                  { "fCartRot1", 10.0F },
                  { "fCartRot2", 10.0F },
               #pragma endregion
               #pragma region F
                  { "fFriction", 100.0F },
               #pragma endregion
               #pragma region G
                  { "fGravMult", 3.5F },
               #pragma endregion
               #pragma region H
                  { "fHorseOffsetX", 0.0F },
                  { "fHorseOffsetY", 200.0F },
                  { "fHorseOffsetZ", 0.0F },
                  { "fHorsePivotX", 0.0F },
                  { "fHorsePivotY", 0.0F },
                  { "fHorsePivotZ", 0.0F },
               #pragma endregion
               #pragma region M
                  { "fMass", 130.0F },
               #pragma endregion
               #pragma region P
                  { "fPoleZOffset", -20.0F },
               #pragma endregion
               #pragma region T
                  { "fTetherOffsetX", 50.0F },
                  { "fTetherOffsetY", 163.0F },
                  { "fTetherOffsetZ", 13.0F },
                  { "fTipImpulse", 500.0F },
               #pragma endregion
               #pragma region W
                  { "fWheelAngDamp", 0.009999999776482582F },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region H
                  { "sHarnessBoneCart", "FrontHorseCarriage01" },
                  { "sHarnessBoneLeft", "HarnessLeftBone" },
                  { "sHarnessBoneRight", "HarnessRightBone" },
                  { "sHorseConnect", "HorseSpine2" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Clouds", {
            { "fCloudAtlasWidth", 1.0F },
            { "sCloudInstanceMesh", "Meshes\\CloudInstance.NIF" },
         }),
         section_definition("Combat", {
            #pragma region Booleans
               #pragma region A
                  { "bAimSights", false },
               #pragma endregion
               #pragma region C
                  { "bChainExplosionDebug", false },
                  { "bCombatPathSmoothing", true },
                  { "bCombatPathSprinting", true },
               #pragma endregion
               #pragma region D
                  { "bDebugCombat", false },
                  { "bDebugCombatAimLocations", false },
                  { "bDebugCombatArea", false },
                  { "bDebugCombatAttackRange", false },
                  { "bDebugCombatCoverReservations", false },
                  { "bDebugCombatDetection", false },
                  { "bDebugCombatGroups", false },
                  { "bDebugCombatGroups2", false },
                  { "bDebugCombatProjectileLOS", false },
                  { "bDebugCombatSearch", false },
                  { "bDebugCombatTargetLocations", false },
                  { "bDebugCombatTargets", false },
                  { "bDebugCombatTextColorDark", false },
                  { "bDebugCombatThreats", false },
                  { "bDebugCombatUnreachableLocations", false },
                  { "bDisableCombatDialogue", false },
                  { "bDisableNPCAttacks", false },
                  { "bDismemberOneLimb", false },
               #pragma endregion
               #pragma region E
                  { "bEncounterZoneTargetRestrict", true },
               #pragma endregion
               #pragma region F
                  { "bForceNPCsUseAmmo", false },
               #pragma endregion
               #pragma region H
                  { "bHazardDebug", false },
               #pragma endregion
               #pragma region I
                  { "bIronSightsZoomEnable", true },
               #pragma endregion
               #pragma region L
                  { "bLaserSights", false },
               #pragma endregion
               #pragma region M
                  { "bMagicDebug", false },
               #pragma endregion
               #pragma region P
                  { "bPlayHitLocationIdles", true },
                  { "bPlayStaggers", true },
                  { "bPlayerAlwaysStaggered", true },
                  { "bProjectileDebug", false },
               #pragma endregion
               #pragma region V
                  { "bVATSProjectileDebug", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region 1
                  { "f1PArrowTiltUpAngle", 2.0F },
                  { "f1PBoltTiltUpAngle", 1.0F },
               #pragma endregion
               #pragma region 3
                  { "f3PArrowTiltUpAngle", 2.5F },
               #pragma endregion
               #pragma region A
                  { "fAimChaseLookingMult", 3.0F },
                  { "fAimDownDegrees", 90.0F },
                  { "fAimUpDegrees", 90.0F },
               #pragma endregion
               #pragma region D
                  { "fDeathForceCleared", 1.0F },
                  { "fDebugCombatProjectileLOSTime", 5.0F },
                  { "fDebugCombatTextSize", 0.5F },
                  { "fDecapInitialSpeed", 250.0F },
               #pragma endregion
               #pragma region H
                  { "fHitEffectThresholdMod", 0.03999999910593033F },
                  { "fHitEffectThresholdSevere", 0.0430000014603138F },
                  { "fHitVectorDelay", 0.4000000059604645F },
               #pragma endregion
               #pragma region I
                  { "fIronSightsZoomDefault", 50.0F },
               #pragma endregion
               #pragma region M
                  { "fMagnetismHeadingMult", 1.0F },
                  { "fMagnetismLookingMult", 10.0F },
                  { "fMagnetismObjHeadingMult", 0.5F },
                  { "fMagnetismObjLookingMult", 0.5F },
                  { "fMagnetismObjStrafeHeadingMult", 0.0F },
                  { "fMagnetismStrafeBaseSpeed", 350.0F },
                  { "fMagnetismStrafeHeadingMult", 0.5F },
                  { "fMagnetismStrafeMaxDistance", 750.0F },
                  { "fMinBloodDamage", 1.0F },
                  { "fMostCommonProjectileCollisionRadius1", 0.0F },
                  { "fMostCommonProjectileCollisionRadius2", 0.5F },
                  { "fMostCommonProjectileCollisionRadius3", 10.0F },
                  { "fMountedAttackRange", 135.0F },
               #pragma endregion
               #pragma region P
                  { "fProjectileDebugDuration", 5.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region M
                  { "iMaxHiPerfCombatCount", 4 },
               #pragma endregion
               #pragma region S
                  { "iShowHitVector", 0 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Controls", {
            #pragma region Booleans
               #pragma region B
                  { "bBackgroundMouse", false },
               #pragma endregion
               #pragma region D
                  { "bDampenPlayerControls", true },
               #pragma endregion
               #pragma region F
                  { "bFreezeDirectionOnLargeDelta", true },
               #pragma endregion
               #pragma region G
                  { { game::skyrim_special }, "bGamepadLookApplyMaxedOutAcceleration", true },
                  { { game::skyrim_special }, "bGamepadLookApplySensitivityThreshold", false },
                  { { game::skyrim_special }, "bGamepadLookApplySnapToAxis", true },
                  { { game::skyrim_special }, "bGamepadLookTimeNormalizeInputs", true },
               #pragma endregion
               #pragma region I
                  { "bInvertMovementThumbstick", false },
               #pragma endregion
               #pragma region P
                  { "bPlayerGraphFeedback", false },
               #pragma endregion
               #pragma region S
                  { "bShowKinectDebugInfo", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region C
                  { "fControllerBufferDepth", 0.14000000059604645F },
                  { "fControllerDampenTime", 0.18000000715255737F },
                  { "fControllerSampleThreshold", 0.10000000149011612F },
               #pragma endregion
               #pragma region D
                  { "fDialogueHardStopAngle1P", 45.0F },
                  { "fDialogueHardStopAngle3P", 55.0F },
                  { "fDialogueSoftStopAngle1P", 20.0F },
                  { "fDialogueSoftStopAngle3P", 25.0F },
                  { "fDirectionalDeadzone", 0.5F },
                  { "fDualCastChordTime", 0.05000000074505806F },
               #pragma endregion
               #pragma region F
                  { "fFreezeDirectionDefaultAngleThreshold", 60.0F },
                  { "fFreezeDirectionDefaultSpeedThreshold", 100.0F },
               #pragma endregion
               #pragma region G
                  { { game::skyrim_special }, "fGamepadHeadingSensitivityDefault", 0.6667199730873108F },
                  { "fGamepadHeadingSensitivityMax", 3.549999952316284F },
                  { "fGamepadHeadingSensitivityMin", 0.25F },
                  { "fGamepadHeadingXScale", 0.8999999761581421F },
                  { "fGamepadHeadingYScale", 23.0F },
                  { { game::skyrim_special }, "fGamepadLookAccelPitchMult", 2.5F },
                  { { game::skyrim_special }, "fGamepadLookAccelSec", 0.9200000166893005F },
                  { { game::skyrim_special }, "fGamepadLookAccelYawMult", 2.5F },
                  { { game::skyrim_special }, "fGamepadLookMultExponent", 0.0F },
               #pragma endregion
               #pragma region H
                  { "fHeadingAxisDeadzone", 0.15000000596046448F },
                  { "fHorseClampAngle", 10.0F },
                  { "fHorseControlsDampenTime", 1.0F },
                  { "fHorseHeadingMovementMult", 0.75F },
                  { "fHotKeyDelay", 0.25F },
               #pragma endregion
               #pragma region I
                  { "fInitialPowerAttackDelay", 0.30000001192092896F },
                  { "fInitialPowerBashDelay", 0.30000001192092896F },
               #pragma endregion
               #pragma region L
                  { "fLThumbDeadzone", 0.23999999463558197F },
                  { { game::skyrim_special }, "fLThumbDeadzoneMax", 0.9700000286102295F },
                  { { game::skyrim_special }, "fLookCurveSensitivityThreshold", 0.10000000149011612F },
                  { { game::skyrim_special }, "fLookGraphCoefficient1", 0.11514099687337875F },
                  { { game::skyrim_special }, "fLookGraphCoefficient2", -0.3826659917831421F },
                  { { game::skyrim_special }, "fLookGraphCoefficient3", 2.0286500453948975F },
                  { { game::skyrim_special }, "fLookGraphCoefficient4", -0.7544990181922913F },
                  { "fLookGraphX1", 0.4000000059604645F },
                  { "fLookGraphX2", 0.6000000238418579F },
                  { "fLookGraphX3", 0.800000011920929F },
                  { "fLookGraphX4", 0.8999999761581421F },
                  { "fLookGraphY1", 0.10000000149011612F },
                  { "fLookGraphY2", 0.20000000298023224F },
                  { "fLookGraphY3", 0.30000001192092896F },
                  { "fLookGraphY4", 0.6000000238418579F },
                  { { game::skyrim_special }, "fLookSnapToAxisStrength", 0.15000000596046448F },
                  { { game::skyrim_special }, "fLookTimeNormalizingFloorFramerate", 20.0F },
                  { { game::skyrim_special }, "fLookTimeNormalizingTargetFramerate", 30.0F },
               #pragma endregion
               #pragma region M
                  { "fMaxLookRampUpDelta", 0.12999999523162842F },
                  { "fMaxMoveRampDownDelta", 500.0F },
                  { "fMouseHeadingSensitivityMax", 0.05000000074505806F },
                  { "fMouseHeadingSensitivityMin", 0.009999999776482582F },
                  { "fMouseHeadingXScale", 0.019999999552965164F },
                  { "fMouseHeadingYScale", 0.8500000238418579F },
                  { "fMoveGraphX1", 0.20000000298023224F },
                  { "fMoveGraphX2", 0.699999988079071F },
                  { "fMoveGraphX3", 0.8999999761581421F },
                  { "fMoveGraphY1", 0.10000000149011612F },
                  { "fMoveGraphY2", 0.5F },
                  { "fMoveGraphY3", 0.8999999761581421F },
                  { "fMovementAxisDeadzone", 0.15000000596046448F },
               #pragma endregion
               #pragma region O
                  { "fOutsideDialogueAngleRotationDampen", 0.33000001311302185F },
               #pragma endregion
               #pragma region P
                  { "fPCDialogueLookSpeed", 10.0F },
                  { "fPCDialogueLookStart", 25.0F },
                  { "fPlayerThirdPersonDampenTime", 0.25F },
               #pragma endregion
               #pragma region R
                  { "fRThumbDeadzone", 0.26499998569488525F },
                  { { game::skyrim_special }, "fRThumbDeadzoneMax", 0.9700000286102295F },
                  { "fReverseDirThreshold", 0.30000001192092896F },
               #pragma endregion
               #pragma region S
                  { "fSprintStopThreshold", 0.5F },
                  { "fSubsequentPowerAttackDelay", 2.0F },
                  { "fSubsequentPowerBashDelay", 2.0F },
               #pragma endregion
               #pragma region T
                  { "fTogglePOVDelay", 0.0F },
                  { "fTriggerDeadzone", 0.30000001192092896F },
               #pragma endregion
               #pragma region Z
                  { "fZKeyDelay", 0.20000000298023224F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region N
                  { "iNumHotkeys", 37 },
                  { "iNumLookGraphSettings", 4 },
                  { "iNumMoveGraphSettings", 3 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("CopyProtectionStrings", {
            { "sCopyProtectionMessage", "Unable to find a CD-ROM/DVD drive on this computer." },
            { "sCopyProtectionMessage2", "Insert the Skyrim Disc." },
            { "sCopyProtectionTitle", "CD-ROM Drive Not Found" },
            { "sCopyProtectionTitle2", "Skyrim Disc Not Found" },
         }),
         section_definition("Debug", {
            { "bDebugFaceGenCriticalSection", false },
            { "bDebugFaceGenMultithreading", false },
            { "bDebugFlyingMountLeash", false },
            { { game::skyrim_special }, "bWriteTACOutputToFile", false },
         }),
         section_definition("Decals", {
            { "bAllowDecalsOnAlpha", true },
            { "bBackgroundInitializeGeometryDecals", true },
            { "bDecalMultithreaded", false },
            { "bDecalOcclusionQuery", true },
            { "bDecals", true },
            { "bForceAllDecals", false },
            { "bSkinnedDecals", true },
            { "fDebrisDecalTimer", 0.004999999888241291F },
         }),
         section_definition("Dialogue", {
            { "fDialogueRotationPitchOffset", 0.17000000178813934F },
            { "fDialogueRotationSecs", 1.0F },
         }),
         section_definition("Display", {
            #pragma region Booleans
               #pragma region A
                  { "bActorSelfShadowing", false },
                  { "bAllow20HairShader", true },
                  { "bAllow30Shaders", false },
                  { "bAllowPartialPrecision", true },
                  { "bAllowScreenShot", false },
                  { { game::skyrim_special }, "bAllowShaderCache", true },
                  { { game::skyrim_special }, "bAssertOnShaderCompileAtRuntime", true },
                  { "bAutoViewDistance", false },
               #pragma endregion
               #pragma region B
                  { { game::skyrim_special }, "bBreakOnValidationError", true },
                  { { game::skyrim_special }, "bBreakOnValidationWarning", false },
               #pragma endregion
               #pragma region C
                  { { game::skyrim_special }, "bCharacterLighting", true },
                  { { game::skyrim_special }, "bCompensateUnstableFrameTime", true },
                  { "bCompileOnRender", true },
                  { { game::skyrim_special }, "bCreateShadowRenderTarget", false },
               #pragma endregion
               #pragma region D
                  { { game::skyrim_special }, "bDOFApplyCenterWeight", true },
                  { { game::skyrim_special }, "bDOFBilateralBlur", true },
                  { { game::skyrim_special }, "bDeactivateAOOnSnow", true },
                  { "bDecalsOnSkinnedGeometry", true },
                  { { game::skyrim_special }, "bDirShadowMapFullViewPort", true },
                  { { game::skyrim_special }, "bDisableHighTreeShadow", true },
                  { { game::skyrim_special }, "bDisableShadowJumps", true },
                  { { game::skyrim_special }, "bDisableZPrepassOutput", false },
                  { "bDo30VFog", true },
                  { "bDoAmbientPass", true },
                  { "bDoDiffusePass", true },
                  { "bDoSpecularPass", true },
                  { "bDoTallGrassEffect", true },
                  { "bDoTestHDR", false },
                  { "bDoTexturePass", true },
                  { { game::skyrim_special }, "bDownSampleNormalSSR", true },
                  { { game::skyrim_special }, "bDynamicDOF", true },
                  { "bDynamicWindowReflections", true },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "bEnableAutoDynamicResolution", false },
                  { { game::skyrim_special }, "bEnableDownsampleComputeShader", true },
                  { { game::skyrim_special }, "bEnableFrontToBackPrepass", false },
                  { { game::skyrim_special }, "bEnableLandFade", true },
                  { { game::skyrim_special }, "bEnableParallaxOcclusion", false },
                  { { game::skyrim_special }, "bEnableProjecteUVDiffuseNormalsOnCubemap", false },
                  { { game::skyrim_special }, "bEnableSnowMask", true },
                  { { game::skyrim_special }, "bEnableSnowRimLighting", true },
                  { { game::skyrim_special }, "bEnableStippleFade", true },
                  { { game::skyrim_special }, "bEnableVolumetricLighting", false },
                  { "bEquippedTorchesCastShadows", false },
               #pragma endregion
               #pragma region F
                  { "bForce1XShaders", false },
                  { "bForceMultiPass", true },
                  { "bForcePow2Textures", false },
               #pragma endregion
               #pragma region I
                  { "bIgnoreResolutionCheck", false },
                  { "bImageSpaceEffects", true },
                  { { game::skyrim_special }, "bIndDownscaled", false },
                  { { game::skyrim_special }, "bIndNormalMap", true },
               #pragma endregion
               #pragma region L
                  { "bLODNoiseAniso", true },
                  { "bLoadMarkers", true },
                  { { game::skyrim_special }, "bLockFramerate", true },
                  { { game::skyrim_special }, "bLodZPrepass", true },
                  { "bLowHealthIModEnabled", true },
               #pragma endregion
               #pragma region M
                  { "bMTRendering", false },
               #pragma endregion
               #pragma region O
                  { { game::skyrim_special }, "bOutputMissingTexture", true },
               #pragma endregion
               #pragma region P
                  { { game::skyrim_special }, "bProjectileOnReflection", false },
               #pragma endregion
               #pragma region R
                  { "bReportBadTangentSpace", false },
               #pragma endregion
               #pragma region S
                  { { game::skyrim_special }, "bSAOApplyFog", true },
                  { { game::skyrim_special }, "bSAODownscaled", false },
                  { { game::skyrim_special }, "bSAONormalMap", true },
                  { { game::skyrim_special }, "bSAO_CS_Downscaled", false },
                  { { game::skyrim_special }, "bShadowsOnGrass", true },
                  { "bShowMarkers", false },
                  { "bShowMenuTextureUse", true },
                  { "bSimpleLighting", false },
                  { { game::skyrim_special }, "bSparklesOnly", false },
                  { "bStaticMenuBackground", true },
               #pragma endregion
               #pragma region T
                  { { game::skyrim_special }, "bTAAWater", false },
               #pragma endregion
               #pragma region U
                  { "bUse Shaders", true },
                  { { game::skyrim_special }, "bUse16BitsDepthTarget", true },
                  { { game::skyrim_special }, "bUseDeviceDebug", false },
                  { "bUseFakeFullScreenMotionBlur", false },
                  { { game::skyrim_special }, "bUseFilmicCurve", false },
                  { { game::skyrim_special }, "bUseMultipleLuminanceReferences", true },
                  { { game::skyrim_special }, "bUsePrecomputedNoise", false },
                  { "bUseRefractionShader", true },
                  { "bUseSunbeams", false },
               #pragma endregion
               #pragma region V
                  { { game::skyrim_special }, "bValidateRenderTargets", true },
                  { { game::skyrim_special }, "bVolumetricLightingDisableInterior", true },
                  { { game::skyrim_special }, "bVolumetricLightingEnableTemporalAccumulation", true },
                  { { game::skyrim_special }, "bVolumetricLightingUpdateWeather", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region 1
                  { { game::skyrim_special }, "f1stPersonFarDepthRange", 0.009999999776482582F },
                  { { game::skyrim_special }, "f1stPersonFarDepthRangeControlDriven", 0.10000000149011612F },
               #pragma endregion
               #pragma region A
                  { { game::skyrim_special }, "fAlphaWeight", 1.0F },
               #pragma endregion
               #pragma region C
                  { { game::skyrim_special }, "fCharacterLightLumMax", 0.75F },
                  { { game::skyrim_special }, "fCharacterLightLumScale", 2.0F },
                  { { game::skyrim_special }, "fCharacterLightPrimaryLightIntensity", 0.75F },
                  { { game::skyrim_special }, "fCharacterLightSecondaryLightIntensity", 0.30000001192092896F },
                  { { game::skyrim_special }, "fClampScale", 0.4000000059604645F },
                  { { game::skyrim_special }, "fConstHDRAdaptTimerForMenu", 0.30000001192092896F },
               #pragma endregion
               #pragma region D
                  { { game::skyrim_special }, "fDDOFAngleThreshold", 2.0F },
                  { { game::skyrim_special }, "fDDOFFocusCenterweightExt", 1.600000023841858F },
                  { { game::skyrim_special }, "fDDOFFocusCenterweightInt", 1.0F },
                  { { game::skyrim_special }, "fDDOFFocusDelay", 500.0F },
                  { { game::skyrim_special }, "fDDOFFocusDuration", 1000.0F },
                  { { game::skyrim_special }, "fDDOFPositionThreshold", 5.0F },
                  { { game::skyrim_special }, "fDOFCenterWeight", 0.6000000238418579F },
                  { { game::skyrim_special }, "fDOFMaxDepthParticipation", 50000.0F },
                  { { game::skyrim_special }, "fDRClampOffset", 0.0020000000949949026F },
                  { { game::skyrim_special }, "fDRClampOffsetNeo", 0.0005000000237487257F },
                  { "fDecalLOD0", 800.0F },
                  { "fDecalLifetime", 30.0F },
                  { { game::skyrim_special }, "fDecreaseDRMilliseconds", 32.0F },
                  { "fDefault1stPersonFOV", 65.0F },
                  { "fDefaultFOV", 65.0F },
                  { "fDefaultWorldFOV", 65.0F },
                  { { game::skyrim_special }, "fDynamicDOFBlurMultiplierMax", 1.0F },
                  { { game::skyrim_special }, "fDynamicDOFBlurMultiplierMin", 0.0F },
                  { { game::skyrim_special }, "fDynamicDOFFarBlur", 0.699999988079071F },
                  { { game::skyrim_special }, "fDynamicDOFFarDist", 1000.0F },
                  { { game::skyrim_special }, "fDynamicDOFFarRange", 10000.0F },
                  { { game::skyrim_special }, "fDynamicDOFNearBlur", 1.0F },
                  { { game::skyrim_special }, "fDynamicDOFNearDist", 100.0F },
                  { { game::skyrim_special }, "fDynamicDOFNearRange", 100.0F },
               #pragma endregion
               #pragma region E
                  { "fEnvMapLOD1", 1500.0F },
                  { "fEnvMapLOD2", 1800.0F },
                  { { game::skyrim_special }, "fExponentialShadowMapScale", 10.0F },
                  { "fEyeEnvMapLOD1", 500.0F },
                  { "fEyeEnvMapLOD2", 800.0F },
               #pragma endregion
               #pragma region F
                  { { game::skyrim_special }, "fFilmicWhiteScale", 1.399999976158142F },
                  { { game::skyrim_special }, "fFilteringAlphaThreshold", 0.009999999776482582F },
                  { { game::skyrim_special }, "fFilteringWaterDepthThreshold", 0.004999999888241291F },
                  { { game::skyrim_special }, "fFirstSliceDistance", 1250.0F },
               #pragma endregion
               #pragma region G
                  { "fGammaMax", 0.6000000238418579F },
                  { "fGammaMin", 1.399999976158142F },
                  { { game::skyrim_special }, "fGlobalBloomThresholdBoost", 0.0F },
                  { { game::skyrim_special }, "fGlobalBrightnessBoost", 0.0F },
                  { { game::skyrim_special }, "fGlobalContrastBoost", 0.0F },
                  { { game::skyrim_special }, "fGlobalEyeAdaptSpeedScale", 2.0F },
                  { { game::skyrim_special }, "fGlobalEyeAdaptStrengthScale", 0.4000000059604645F },
                  { { game::skyrim_special }, "fGlobalMapBloomThresholdBoost", 0.25F },
                  { { game::skyrim_special }, "fGlobalMapBrightnessBoost", 0.25F },
                  { { game::skyrim_special }, "fGlobalMapContrastBoost", -0.30000001192092896F },
                  { { game::skyrim_special }, "fGlobalSaturationBoost", 0.0F },
               #pragma endregion
               #pragma region I
                  { { game::skyrim_special }, "fIBLFAnamorphicsIntensity", 0.10000000149011612F },
                  { { game::skyrim_special }, "fIBLFAnamorphicsIntensityFar", 1.0F },
                  { { game::skyrim_special }, "fIBLFBloomIntensity", 0.0F },
                  { { game::skyrim_special }, "fIBLFChannelsDistortionBlue", -2.0F },
                  { { game::skyrim_special }, "fIBLFChannelsDistortionGreen", 0.0F },
                  { { game::skyrim_special }, "fIBLFChannelsDistortionRed", 2.0F },
                  { { game::skyrim_special }, "fIBLFFlaresDispersal", 0.30000001192092896F },
                  { { game::skyrim_special }, "fIBLFGlobalIntensity", 0.699999988079071F },
                  { { game::skyrim_special }, "fIBLFHaloFetch", 0.5F },
                  { { game::skyrim_special }, "fIBLFHaloWidthPow", 3.0F },
                  { { game::skyrim_special }, "fIBLFLightsBurn", 1.0F },
                  { { game::skyrim_special }, "fIBLFLightsRangeDownshift", 1.0F },
                  { { game::skyrim_special }, "fIncreaseDRMilliseconds", 30.0F },
                  { { game::skyrim_special }, "fIndBias", 2.5F },
                  { { game::skyrim_special }, "fIndIntensity", 50.0F },
                  { { game::skyrim_special }, "fIndRadius", 100.0F },
               #pragma endregion
               #pragma region L
                  { "fLODNoiseMipBias", 0.0F },
                  { "fLandLOFadeSeconds", 15.0F },
                  { "fLightLODDefaultStartFade", 1000.0F },
                  { "fLightLODMaxStartFade", 1200.0F },
                  { "fLightLODMinStartFade", 200.0F },
                  { "fLightLODRange", 500.0F },
                  { "fLinePrimitiveWidth", 8.0F },
                  { { game::skyrim_special }, "fLoadingMenuShadowBias", 30.639999389648438F },
                  { { game::skyrim_special }, "fLoadingMenuShadowFallOff", 0.0F },
                  { "fLowHealthIModInterval", 2.0F },
                  { "fLowHealthIModStrengthMax", 1.5F },
                  { "fLowHealthIModStrengthMin", 0.800000011920929F },
                  { { game::skyrim_special }, "fLowestDynamicHeightRatio", 1.0F },
                  { { game::skyrim_special }, "fLowestDynamicWidthRatio", 0.699999988079071F },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "fMaxFocusShadowMapDistance", 450.0F },
                  { { game::skyrim_special }, "fMaxHeightShadowCastingTrees", 5000.0F },
                  { "fMeshLODFadeTime", 1.0F },
                  { "fMipBias", 0.0F },
               #pragma endregion
               #pragma region N
                  { "fNear1stPersonDistance", 5.0F },
                  { "fNearDistance", 15.0F },
                  { "fNoLODFarDistanceMax", 10240.0F },
                  { "fNoLODFarDistanceMin", 100.0F },
                  { "fNoLODFarDistancePct", 1.0F },
                  { { game::skyrim_special }, "fNonSpecularSparklesIntensity", 0.4000000059604645F },
               #pragma endregion
               #pragma region P
                  { { game::skyrim_special }, "fPoissonRadiusScale", 4.0F },
               #pragma endregion
               #pragma region R
                  { { game::skyrim_special }, "fRatioDecreasePerSeconds", 0.07000000029802322F },
                  { { game::skyrim_special }, "fRatioIncreasePerSeconds", 0.029999999329447746F },
                  { { game::skyrim_special }, "fReflectionMarchingRadius", 0.4000000059604645F },
                  { { game::skyrim_special }, "fReflectionRayThickness", 0.0010000000474974513F },
                  { { game::skyrim_special }, "fReflectionsIntensityScale", 0.800000011920929F },
                  { { game::skyrim_special }, "fReflectionsMotionVectorScale", 1.0F },
                  { { game::skyrim_special }, "fReinhardWhiteScale", 1.100000023841858F },
               #pragma endregion
               #pragma region S
                  { { game::skyrim_special }, "fSAOBias", 2.5F },
                  { { game::skyrim_special }, "fSAOExpFactor", 0.10999999940395355F },
                  { { game::skyrim_special }, "fSAOIntensity", 15.0F },
                  { { game::skyrim_special }, "fSAORadius", 250.0F },
                  { { game::skyrim_special }, "fSAOValueDiffFactor", 0.30000001192092896F },
                  { { game::skyrim_special }, "fSAO_CS_Bias", 2.5F },
                  { { game::skyrim_special }, "fSAO_CS_Intensity", 10.0F },
                  { { game::skyrim_special }, "fSAO_CS_Radius", 250.0F },
                  { "fScopeScissorAmount", 0.30000001192092896F },
                  { { game::skyrim_special }, "fShadowBiasScale", 1.0F },
                  { { game::skyrim_special }, "fShadowClampValue", 0.30000001192092896F },
                  { { game::skyrim_special }, "fShadowDirectionalBiasScale", 0.30000001192092896F },
                  { "fShadowFadeTime", 1.0F },
                  { "fShadowLODDefaultStartFade", 200.0F },
                  { "fShadowLODMaxStartFade", 300.0F },
                  { "fShadowLODMinStartFade", 100.0F },
                  { "fShadowLODRange", 200.0F },
                  { { game::skyrim_special }, "fShadowSparkleIntensity", 0.25F },
                  { "fSkinnedDecalLOD0", 300.0F },
                  { "fSkinnedDecalLOD1", 500.0F },
                  { "fSkinnedDecalLOD2", 800.0F },
                  { { game::skyrim_special }, "fSnowGeometrySpecPower", 3.0F },
                  { { game::skyrim_special }, "fSnowNormalSpecPower", 2.0F },
                  { { game::skyrim_special }, "fSnowRimLightIntensity", 0.30000001192092896F },
                  { { game::skyrim_special }, "fSnowSSSColorB", 0.07999999821186066F },
                  { { game::skyrim_special }, "fSnowSSSColorG", 0.0F },
                  { { game::skyrim_special }, "fSnowSSSColorR", 0.0F },
                  { { game::skyrim_special }, "fSnowSSSDepthDiff", 0.5F },
                  { { game::skyrim_special }, "fSnowSSSStrength", 50.0F },
                  { { game::skyrim_special }, "fSnowSparklesColorB", 1.0F },
                  { { game::skyrim_special }, "fSnowSparklesColorG", 1.0F },
                  { { game::skyrim_special }, "fSnowSparklesColorR", 1.0F },
                  { { game::skyrim_special }, "fSparklesDensity", 0.8500000238418579F },
                  { { game::skyrim_special }, "fSparklesIntensity", 1.0F },
                  { { game::skyrim_special }, "fSparklesMaxDistance", 1000.0F },
                  { { game::skyrim_special }, "fSparklesSize", 6.0F },
                  { { game::skyrim_special }, "fSparklesSpecularPower", 2.0F },
                  { { game::skyrim_special }, "fSpecMaskBegin", 0.10000000149011612F },
                  { { game::skyrim_special }, "fSpecMaskSpan", 0.0F },
                  { "fSpecularLODDefaultStartFade", 500.0F },
                  { "fSpecularLODMaxStartFade", 600.0F },
                  { "fSpecularLODMinStartFade", 200.0F },
                  { "fSpecularLODRange", 300.0F },
                  { { game::skyrim_special }, "fSpecularSparklesIntensity", 1.0F },
                  { { game::skyrim_special }, "fSplitOverlap", 100.0F },
                  { "fSunShadowUpdateTime", 1.0F },
                  { { game::skyrim_special }, "fSunStaticTimeUpdateScale", 0.10000000149011612F },
                  { "fSunUpdateThreshold", 0.5F },
               #pragma endregion
               #pragma region T
                  { { game::skyrim_special }, "fTAAEffectThreshold", 0.10000000149011612F },
                  { { game::skyrim_special }, "fTAAHighFreq", 0.800000011920929F },
                  { { game::skyrim_special }, "fTAALowFreq", 0.5F },
                  { { game::skyrim_special }, "fTAAPostOverlay", 0.20999999344348907F },
                  { { game::skyrim_special }, "fTAAPostSharpen", 0.20999999344348907F },
                  { { game::skyrim_special }, "fTAASharpen", 1.0F },
               #pragma endregion
               #pragma region V
                  { { game::skyrim_special }, "fVolumetricLightingCustomColorContribution", 0.0F },
                  { { game::skyrim_special }, "fVolumetricLightingDensityContribution", 0.30000001192092896F },
                  { { game::skyrim_special }, "fVolumetricLightingDensityScale", 300.0F },
                  { { game::skyrim_special }, "fVolumetricLightingIntensity", 2.0F },
                  { { game::skyrim_special }, "fVolumetricLightingPhaseContribution", 0.8299999833106995F },
                  { { game::skyrim_special }, "fVolumetricLightingPhaseScattering", 0.8500000238418579F },
                  { { game::skyrim_special }, "fVolumetricLightingRangeFactor", 40.0F },
                  { { game::skyrim_special }, "fVolumetricLightingTemporalAccumulationFactor", 0.75F },
                  { { game::skyrim_special }, "fVolumetricLightingWindFallingSpeed", 0.30000001192092896F },
                  { { game::skyrim_special }, "fVolumetricLightingWindSpeedScale", 15.0F },
               #pragma endregion
               #pragma region W
                  { { game::skyrim_special }, "fWaterSSRBlurAmount", 0.30000001192092896F },
                  { { game::skyrim_special }, "fWaterSSRIntensity", 1.2999999523162842F },
                  { { game::skyrim_special }, "fWaterSSRNormalPerturbationScale", 0.05000000074505806F },
                  { { game::skyrim_special }, "fWindGrassMultiplier", 1.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region A
                  { "iAdapter", 0 },
                  { "iAutoViewHiFrameRate", 40 },
                  { "iAutoViewLowFrameRate", 20 },
                  { "iAutoViewMinDistance", 2000 },
               #pragma endregion
               #pragma region D
                  { "iDebugTextLeftRightOffset", 10 },
                  { "iDebugTextSubPage", 0 },
                  { "iDebugTextTopBottomOffset", 20 },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "iEnableShadowCastingFlag", 2 },
               #pragma endregion
               #pragma region L
                  { { game::skyrim_special }, "iLandscapeMultiNormalTilingFactor", 4 },
                  { { game::skyrim_special }, "iLoadingMenuShadowLightFlags", 1 },
                  { "iLocation X", 5 },
                  { "iLocation Y", 5 },
               #pragma endregion
               #pragma region N
                  { "iNPatchNOrder", 0 },
                  { "iNPatchPOrder", 0 },
                  { "iNPatches", 0 },
               #pragma endregion
               #pragma region P
                  { "iPresentInterval", 1 },
               #pragma endregion
               #pragma region S
                  { "iShaderPackageMemoryCap", 409600 },
                  { { game::skyrim_special }, "iSnowSSSCurrentColor", 2 },
                  { { game::skyrim_special }, "iSnowSparklesColor", 2 },
               #pragma endregion
               #pragma region T
                  { "iTrilinearThreshold", 3 },
               #pragma endregion
               #pragma region U
                  { { game::skyrim_special }, "iUnstableFrameTimeHistorySize", 8 },
               #pragma endregion
               #pragma region V
                  { { game::skyrim_special }, "iVolumetricLightingNoiseTextureDepth", 32 },
                  { { game::skyrim_special }, "iVolumetricLightingNoiseTextureHeight", 32 },
                  { { game::skyrim_special }, "iVolumetricLightingNoiseTextureWidth", 32 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureDepthHigh", 90 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureDepthLow", 50 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureDepthMedium", 70 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureFormatHigh", 1 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureFormatLow", 0 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureFormatMedium", 1 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureHeightHigh", 192 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureHeightLow", 96 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureHeightMedium", 128 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureWidthHigh", 320 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureWidthLow", 160 },
                  { { game::skyrim_special }, "iVolumetricLightingTextureWidthMedium", 224 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region D
                  { "sDebugText", "VATS" },
               #pragma endregion
               #pragma region S
                  { "sScreenShotBaseName", "ScreenShot" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("FaceGen", {
            { "bUseCustomizationMorphs", true },
            { "bUseRaceMorph", true },
         }),
         section_definition("Fonts", {
            { "sFontConfigFile", "Interface\\FontConfig.txt" },
         }),
         section_definition("FootIK", {
            #pragma region Booleans
               #pragma region F
                  { "bFootPlacementOn", true },
               #pragma endregion
               #pragma region R
                  { "bRigidBodyController", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fAnkleOffset", 0.20000000298023224F },
               #pragma endregion
               #pragma region C
                  { "fControllerTetherLen", 6.0F },
               #pragma endregion
               #pragma region F
                  { "fFootPlantedGain", 1.0F },
                  { "fFootRaisedGain", 0.8999999761581421F },
               #pragma endregion
               #pragma region G
                  { "fGroundAscendingGain", 0.4000000059604645F },
                  { "fGroundDescendingGain", 0.4000000059604645F },
               #pragma endregion
               #pragma region M
                  { "fMaxFootCastMilliSec", 0.6000000238418579F },
                  { "fMaxStepVertError", 3.5F },
               #pragma endregion
               #pragma region O
                  { "fOnOffGain", 0.5F },
                  { "fOriginalGroundHeightMS", -0.10999999940395355F },
               #pragma endregion
               #pragma region P
                  { "fPelvisOffsetDamping", 0.20000000298023224F },
                  { "fPelvisUpDownBias", 0.75F },
               #pragma endregion
               #pragma region R
                  { "fRagdollFeedback", 0.699999988079071F },
               #pragma endregion
               #pragma region V
                  { "fVertErrorGain", 0.5F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region N
                  { "iNumFramesFootEaseOut", 30 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Gameplay", {
            #pragma region Booleans
               #pragma region A
                  { "bAllowDragonFlightLocationDiscovery", false },
                  { "bAllowHavokGrabTheLiving", false },
               #pragma endregion
               #pragma region E
                  { "bEssentialTakeNoDamage", true },
               #pragma endregion
               #pragma region H
                  { "bHealthBarShowing", false },
               #pragma endregion
               #pragma region I
                  { "bInstantLevelUp", false },
               #pragma endregion
               #pragma region T
                  { "bTargetLockIsToggle", true },
                  { "bTrackProgress", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region F
                  { "fFootIKDistance", 1024.0F },
               #pragma endregion
               #pragma region M
                  { "fMagicTargetLocationExtraLargeActorRadius", 256.0F },
                  { "fMagicTargetLocationNormalActorRadius", 32.0F },
                  { "fMapMarkerUpdateTime", 0.05000000074505806F },
               #pragma endregion
               #pragma region P
                  { "fPlayerHealthSaveOnPauseLimit", 0.25F },
                  { "fPlayerSunGazeDelta", 0.9848080277442932F },
                  { "fPlayerSunGazeStartTimer", 0.5F },
               #pragma endregion
               #pragma region T
                  { "fTargetLockXYRange", 7500.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region A
                  { "iActorsDismemberedPerFrame", 2 },
               #pragma endregion
               #pragma region D
                  { "iDetectionPicks", 21 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region T
                  { "sTrackProgressPath", "\\\\vault2\\Fallout\\LevelData\\" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("General", {
            #pragma region Booleans
               #pragma region A
                  { { game::skyrim_special }, "bActivateFromSave", false },
                  { "bAlwaysActive", false },
                  { "bAnimateDoorPhysics", false },
               #pragma endregion
               #pragma region B
                  { "bBackgroundLoadVMData", false },
                  { "bBorderRegionsEnabled", true },
               #pragma endregion
               #pragma region C
                  { "bCRTMemoryChecks", false },
                  { "bCalculateArmorMeshAndTextureFileCounts", true },
                  { "bChangeTimeMultSlowly", true },
                  { "bCheckCellOffsetsOnInit", false },
                  { "bCheckPurgedTextureList", false },
                  { "bCreate Maps Enable", false },
                  { { game::skyrim_special }, "bCullingJobEnablePlaneOptimization", false },
               #pragma endregion
               #pragma region D
                  { "bDebugSpectatorThreats", false },
                  { "bDefaultCOCPlacement", false },
                  { "bDirectionalMaterial", true },
                  { "bDisableAllGore", false },
                  { "bDisableDuplicateReferenceCheck", true },
                  { "bDisableGearedUp", true },
                  { { game::skyrim_special }, "bDisableWarningWindows", false },
                  { "bDisplayBoundingVolumes", false },
               #pragma endregion
               #pragma region E
                  { "bEnableBoundingVolumeOcclusion", true },
                  { "bEnableFileCaching", false },
                  { { game::skyrim_special }, "bEnableFriendHenchman", false },
                  { "bExternalLODDataFiles", true },
               #pragma endregion
               #pragma region F
                  { "bFaceMipMaps", true },
                  { "bFacegenDisableMorphs", false },
                  { "bFixAIPackagesOnLoad", false },
                  { "bFlyingMountFastTravelCruiseEnabled", false },
               #pragma endregion
               #pragma region H
                  { "bHealthRegenFromRacePlayerOnly", true },
               #pragma endregion
               #pragma region J
                  { { game::skyrim_special }, "bJoblistActiveWait", true },
               #pragma endregion
               #pragma region K
                  { "bKeepDLStringBlocksLoaded", false },
                  { "bKeepILStringBlocksLoaded", true },
                  { "bKeepPluginWhenMerging", false },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "bModManagerMenuEnabled", true },
                  { "bMultiThreadMovement", true },
               #pragma endregion
               #pragma region P
                  { "bParallelAnimUpdate", false },
                  { { game::skyrim_special }, "bPauseWhenConstrained", true },
                  { "bPreCullActors", true },
                  { "bPreemptivelyUnloadCells", false },
                  { "bPreloadIntroSequence", true },
                  { { game::skyrim_special }, "bPreloadLinkedInteriors", false },
               #pragma endregion
               #pragma region Q
                  { "bQueueWarnings", false },
               #pragma endregion
               #pragma region R
                  { "bReconstructIDTags", false },
                  { "bRunMiddleLowLevelProcess", true },
                  { "bRunVTuneTest", false },
               #pragma endregion
               #pragma region S
                  { "bShowCheckMemoryOutput", false },
                  { "bShowGunTarget", false },
                  { "bShowLoadingAreaMessage", false },
               #pragma endregion
               #pragma region T
                  { "bTaskletActorSceneGraphUpdates", true },
                  { "bTaskletCellTransformsUpdate", true },
                  { "bTintMipMaps", false },
                  { "bTrackAllDeaths", false },
               #pragma endregion
               #pragma region U
                  { "bUseBodyMorphs", true },
                  { "bUseEyeEnvMapping", true },
                  { "bUseFaceGenPreprocessedHeads", true },
                  { "bUseHardDriveCache", false },
                  { "bUseMovementBlockedPackage", false },
                  { "bUseMultibounds", true },
                  { "bUseMyGamesDirectory", true },
                  { "bUseOptimizedTextureLoading", true },
                  { "bUseThreadedMorpher", false },
                  { "bUseThreadedParticleSystem", false },
                  { "bUseThreadedTempEffects", true },
               #pragma endregion
               #pragma region W
                  { "bWarnOnMaterialCollisions", false },
                  { "bWarnOnMissingFileEntry", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fAutoDoorFadeSecs", 0.5F },
               #pragma endregion
               #pragma region B
                  { "fBSTaskTime", 2.0F },
                  { "fBetaDeferredKillTimer", 15.0F },
               #pragma endregion
               #pragma region C
                  { "fCloakEffectUpdateInterval", 0.5F },
               #pragma endregion
               #pragma region D
                  { "fDragonLandingForPlayerNavMeshSize", 250.0F },
                  { "fDragonLandingForPlayerSearchDist", 3000.0F },
               #pragma endregion
               #pragma region E
                  { "fEncumberedReminderTimer", 30.0F },
               #pragma endregion
               #pragma region F
                  { "fFastTravelFadeSecs", 0.5F },
                  { "fFlickeringLightDistance", 1024.0F },
                  { "fFlyingMountDismountOffset", 30.0F },
                  { "fFlyingMountFastTravelArrivalHeight", 1000.0F },
                  { "fFlyingMountFastTravelDragonSpeed", 7000.0F },
                  { "fFlyingMountLandingRequestTimer", 3.0F },
                  { "fFlyingMountSlowestSpeedMult", 0.25F },
                  { "fFlyingMountTutorialMessageDelay", 15.0F },
               #pragma endregion
               #pragma region L
                  { "fLoadGameFadeSecs", 1.0F },
               #pragma endregion
               #pragma region M
                  { "fMasterFilePreLoadMB", 40.0F },
               #pragma endregion
               #pragma region N
                  { "fNormalDoorFadeSecs", 0.4000000059604645F },
                  { "fNormalDoorFadeWait", 0.009999999776482582F },
               #pragma endregion
               #pragma region P
                  { "fPlayerFlyingMountBaseTargetSpeed", 700.0F },
                  { "fPlayerFlyingMountFastBaseTargetSpeed", 1400.0F },
                  { "fPlayerFlyingMountNothingLoadingMult", 10.0F },
                  { "fPlayerFlyingMountTravelMaxHeight", 400.0F },
                  { "fPlayerFlyingMountTravelMinHeight", 200.0F },
                  { "fProcessListsUpdateHighFrameRate", 30.0F },
                  { "fProcessListsUpdateLowFrameRate", 10.0F },
                  { "fProcessListsUpdateTimeMax", 5.0F },
                  { "fProcessListsUpdateTimeMin", 0.5F },
               #pragma endregion
               #pragma region S
                  { "fStoryTellerQuestFindTime", 2.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region A
                  { "iAIThread1HWThread", 3 },
                  { "iAIThread2HWThread", 5 },
               #pragma endregion
               #pragma region E
                  { "iEnumRefsAllowedPerFrame", 1 },
               #pragma endregion
               #pragma region F
                  { "iFPSClamp", 0 },
                  { "iFlyingMountSlowestQueuedRefCount", 250 },
               #pragma endregion
               #pragma region H
                  { "iHWThread1", 4 },
                  { "iHWThread2", 4 },
                  { "iHWThread3", 4 },
                  { "iHWThread4", 5 },
                  { "iHWThread5", 5 },
                  { "iHWThread6", 5 },
               #pragma endregion
               #pragma region I
                  { "iIntroSequencePriority", 3 },
               #pragma endregion
               #pragma region L
                  { "iLargeIntRefCount", 1000 },
                  { "iLowProcessingMilliseconds", 2 },
               #pragma endregion
               #pragma region M
                  { { game::skyrim_special }, "iMaxJobThreads", 32 },
               #pragma endregion
               #pragma region N
                  { "iNumBitsForFullySeen", 248 },
                  { "iNumHWThreads", 4 },
               #pragma endregion
               #pragma region P
                  { "iPreloadSizeLimit", 26214400 },
               #pragma endregion
               #pragma region R
                  { "iRenderingThread1HWThread", 0 },
                  { "iRenderingThread2HWThread", 1 },
               #pragma endregion
               #pragma region U
                  { "iUpdateDetectionsAllowedPerFrame", 100 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region C
                  { "sCharGenQuest", "0003372b" },
               #pragma endregion
               #pragma region E
                  { "sEssentialFileCacheList", "Not enough arguments..." },
               #pragma endregion
               #pragma region G
                  { { game::skyrim_special }, "sGamerIconTextureName", "BGSUserIcon" },
               #pragma endregion
               #pragma region I
                  { "sIntroMovie", "Not enough arguments..." },
                  { "sIntroSequence", "BGS_LOGO.BIK" },
               #pragma endregion
               #pragma region L
                  { "sLanguage", "ENGLISH" },
                  { { game::skyrim_special }, "sLocalCharacterDataPath", "Saves\\Character\\" },
                  { "sLocalMasterPath", "Data\\" },
                  { "sLocalSavePath", "Saves\\" },
               #pragma endregion
               #pragma region M
                  { "sMainMenuMovieIntro", "Not enough arguments..." },
                  { "sMainMenuMusic", "\\Data\\Music\\Special\\MUS_MainTheme.xwm" },
               #pragma endregion
               #pragma region S
                  { "sStartingCell", "Not enough arguments..." },
                  { "sStartingCellX", "Not enough arguments..." },
                  { "sStartingCellY", "Not enough arguments..." },
                  { "sStartingWorld", "Not enough arguments..." },
               #pragma endregion
               #pragma region T
                  { "sTestFile1", "Skyrim.ESM" },
                  { "sTestFile10", "Not enough arguments..." },
                  { "sTestFile2", "Not enough arguments..." },
                  { "sTestFile3", "Not enough arguments..." },
                  { "sTestFile4", "Not enough arguments..." },
                  { "sTestFile5", "Not enough arguments..." },
                  { "sTestFile6", "Not enough arguments..." },
                  { "sTestFile7", "Not enough arguments..." },
                  { "sTestFile8", "Not enough arguments..." },
                  { "sTestFile9", "Not enough arguments..." },
               #pragma endregion
               #pragma region U
                  { "sUnessentialFileCacheList", "Not enough arguments..." },
               #pragma endregion
               #pragma region T
                  { { game::skyrim_special }, "strPluginsFileHeader", "# This file is used by Skyrim to keep track of your downloaded content." },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("GeneralWarnings", {
            { "sGeneralMasterMismatchWarning", "One or more plugins could not find the correct versions of the master files they depend on. Errors may occur during load or game play. Check the \"Warnings.txt\" file for more information." },
            { "sMasterMismatchWarning", "One of the files that \"%s\" is dependent on has changed since the last save." },
         }),
         section_definition("GethitShader", {
            { "fBlockedTexOffset", 0.0010000000474974513F },
            { "fBlurAmmount", 0.5F },
            { "fHitTexOffset", 0.004999999888241291F },
         }),
         section_definition("GrabIK", {
            { "fDriveGain", 0.25F },
         }),
         section_definition("Grass", {
            #pragma region Booleans
               #pragma region A
                  { "bAllowCreateGrass", false },
                  { "bAllowLoadGrass", true },
               #pragma endregion
               #pragma region D
                  { "bDrawShaderGrass", true },
               #pragma endregion
               #pragma region E
                  { { game::skyrim_special }, "bEnableGrassFade", true },
               #pragma endregion
               #pragma region G
                  { "bGenerateGrassDataFiles", false },
                  { "bGrassPointLighting", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region G
                  { "fGrassDefaultStartFadeDistance", 3500.0F },
                  { { game::skyrim_special }, "fGrassFadeInTime", 1.7999999523162842F },
                  { "fGrassFadeRange", 1000.0F },
                  { "fGrassWindMagnitudeMax", 125.0F },
                  { "fGrassWindMagnitudeMin", 5.0F },
               #pragma endregion
               #pragma region T
                  { "fTexturePctThreshold", 0.0F },
               #pragma endregion
               #pragma region W
                  { "fWaveOffsetRange", 1.75F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region G
                  { "iGrassCellRadius", 2 },
               #pragma endregion
               #pragma region M
                  { "iMaxGrassTypesPerTexure", 2 },
                  { "iMinGrassSize", 20 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("HAVOK", {
            #pragma region Booleans
               #pragma region A
                  { "bAddBipedWhenKeyframed", false },
                  { "bAllowCharacterBumper", true },
                  { "bAllowDeactivationWhileWarmStarting", false },
               #pragma endregion
               #pragma region D
                  { "bDebugMultithreaded", false },
                  { "bDisablePlayerCollision", false },
               #pragma endregion
               #pragma region F
                  { "bFindContactPointsOnAdd", false },
                  { "bForceJumpingFromGraph", false },
               #pragma endregion
               #pragma region H
                  { "bHavokDebug", false },
               #pragma endregion
               #pragma region P
                  { "bPreventHavokAddAll", false },
                  { "bPreventHavokAddClutter", false },
               #pragma endregion
               #pragma region R
                  { "bRegisterAllVDBViewers", false },
               #pragma endregion
               #pragma region U
                  { "bUseCharRBExtrapolation", true },
                  { "bUseCharacterRB", true },
                  { "bUseConstraintProjector", true },
                  { "bUseUnsupportedCast", true },
                  { "bUseWorldLock", true },
               #pragma endregion
               #pragma region W
                  { "bWONameSync", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region C
                  { "fCameraCasterFadeSittingRadius", 45.0F },
                  { "fCharControllerWarpDistSqr", 6000000.0F },
               #pragma endregion
               #pragma region D
                  { "fDefaultWaterfallCurrentVelocity", 6.0F },
               #pragma endregion
               #pragma region G
                  { "fGoodPosCastCheckDepth", 0.10000000149011612F },
                  { "fGoodPosCheckDepth", 0.10000000149011612F },
               #pragma endregion
               #pragma region I
                  { "fInAirFallingCharGravityMult", 1.350000023841858F },
               #pragma endregion
               #pragma region J
                  { "fJumpAnimDelay", 0.75F },
               #pragma endregion
               #pragma region M
                  { "fMaxPickTime", 0.003000000026077032F },
                  { "fMaxPickTimeDebug", 0.05999999865889549F },
                  { "fMaxPickTimeDebugVATS", 0.6000000238418579F },
                  { "fMaxPickTimeVATS", 0.029999999329447746F },
                  { "fMaxTime", 0.01666666753590107F },
                  { "fMaxTimeComplex", 0.03333333507180214F },
                  { "fMoveLimitMass", 95.0F },
               #pragma endregion
               #pragma region O
                  { "fOD", 0.8999999761581421F },
               #pragma endregion
               #pragma region Q
                  { "fQuadrupedPitchMult", 10.0F },
               #pragma endregion
               #pragma region R
                  { "fRF", 1000.0F },
               #pragma endregion
               #pragma region S
                  { "fSD", 0.9800000190734863F },
                  { "fSE", 0.30000001192092896F },
               #pragma endregion
               #pragma region T
                  { "fTimePerSubStep", 0.00800000037997961F },
                  { "fTrapHitEventDelayMS", 500.0F },
                  { "fTriggerEventDelayMS", 500.0F },
               #pragma endregion
               #pragma region U
                  { "fUnsupportCastLength", 1.0F },
                  { "fUnsupportedGravMult", 4.0F },
               #pragma endregion
               #pragma region W
                  { "fWarmStartMaxTime", 4.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region E
                  { "iEntityBatchRemoveRate", 100 },
               #pragma endregion
               #pragma region M
                  { "iMinNumSubSteps", 8 },
               #pragma endregion
               #pragma region N
                  { "iNumThreads", 1 },
               #pragma endregion
               #pragma region S
                  { "iSimType", 1 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("HeadTracking", {
            { "bDisableHeadTracking", false },
            { "fHeadTrackingMaxAngle", 90.0F },
            { "fMaxPathLookAtPointDist", 512.0F },
            { "fMinPathLookAtPointDist", 128.0F },
            { "fPathLookAtPointTime", 2.0F },
            { "fUpdateDelayNewTargetSecondsMax", 8.0F },
            { "fUpdateDelayNewTargetSecondsMin", 3.0F },
            { "fUpdateDelaySecondsMax", 1.5F },
            { "fUpdateDelaySecondsMin", 1.0F },
            { "iUpdateActorsPerFrame", 10 },
         }),
         section_definition("Imagespace", {
            { "bDoRadialBlur", true },
            { { game::skyrim_special }, "fLensFlareFalloffRange", 256.0F },
            { { game::skyrim_special }, "fLensFlareGlobalIntensity", 1.0F },
            { "fRenderDepthMaxDepth", 10000.0F },
            { "iRadialBlurLevel", 0 },
         }),
         section_definition("Interface", {
            #pragma region Booleans
               #pragma region S
                  { "bShowCrosshair", true },
                  { "bShowHUDMessages", true },
                  { "bShowInventory3D", true },
                  { "bShowSubtitleSpeakerName", true },
                  { "bShowTutorials", true },
               #pragma endregion
               #pragma region U
                  { "bUseAllNonDefaultLoadScreensFirst", false },
                  { "bUseFuzzyPicking", true },
                  { "bUserClosesLoadingMenu", false },
               #pragma endregion
               #pragma region W
                  { "bWriteTranslationFile", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fActivatePickLength", 180.0F },
                  { "fActivatePickRadius", 16.0F },
               #pragma endregion
               #pragma region B
                  { "fBookDistance", 110.0F },
                  { "fBookLight2DiffuseColorB", 0.8299999833106995F },
                  { "fBookLight2DiffuseColorG", 0.949999988079071F },
                  { "fBookLight2DiffuseColorR", 0.9800000190734863F },
                  { "fBookLight2DimmerValue", 1.0F },
                  { "fBookLight2Radius", 400.0F },
                  { "fBookLight2X", 10.0F },
                  { "fBookLight2Y", -75.0F },
                  { "fBookLight2Z", 10.0F },
                  { "fBookLightDiffuseColorB", 0.8299999833106995F },
                  { "fBookLightDiffuseColorG", 0.949999988079071F },
                  { "fBookLightDiffuseColorR", 0.9800000190734863F },
                  { "fBookLightDimmerValue", 1.75F },
                  { "fBookLightRadius", 400.0F },
                  { "fBookLightX", 100.0F },
                  { "fBookLightY", -350.0F },
                  { "fBookLightZ", 100.0F },
                  { "fBookOpenTime", 1000.0F },
                  { "fBookPosHeightPercentage", 0.4449999928474426F },
                  { "fBookPosWidthPercentage", 0.5F },
                  { "fBookXRotation", -25.0F },
               #pragma endregion
               #pragma region C
                  { "fCrafting3DItemPosX", 0.0F },
                  { "fCrafting3DItemPosXWide", 0.0F },
                  { "fCrafting3DItemPosY", -500.0F },
                  { "fCrafting3DItemPosYWide", -500.0F },
                  { "fCrafting3DItemPosZ", 16.0F },
                  { "fCrafting3DItemPosZWide", 12.0F },
                  { "fCrafting3DItemScale", 1.8700000047683716F },
                  { "fCrafting3DItemScaleWide", 1.5F },
               #pragma endregion
               #pragma region D
                  { "fDebugFontSize", 13.0F },
               #pragma endregion
               #pragma region F
                  { "fFadeToBlackFadeSeconds", 1.0F },
               #pragma endregion
               #pragma region G
                  { "fGamepadCursorSpeed", 11.0F },
               #pragma endregion
               #pragma region I
                  { "fInterfaceTintB", 0.8823999762535095F },
                  { "fInterfaceTintG", 0.9843000173568726F },
                  { "fInterfaceTintR", 0.6313999891281128F },
                  { "fInventory3DBoundRadiusScale", 12.5F },
                  { "fInventory3DItemPosScale", 1.8700000047683716F },
                  { "fInventory3DItemPosScaleWide", 1.5F },
                  { "fInventory3DItemPosX", -29.0F },
                  { "fInventory3DItemPosXWide", -22.0F },
                  { "fInventory3DItemPosY", -500.0F },
                  { "fInventory3DItemPosYWide", -500.0F },
                  { "fInventory3DItemPosZ", 16.0F },
                  { "fInventory3DItemPosZWide", 12.0F },
                  { "fInventory3DItemRotMouseSpeed", 50.0F },
                  { "fInventory3DItemRotSpeed", 3.0F },
                  { "fInventory3DItemZoomScale", 2.25F },
                  { "fInventory3DItemZoomSpeed", 1.5F },
                  { "fInventory3DItemZoomX", 0.0F },
                  { "fInventory3DItemZoomY", -500.0F },
                  { "fInventory3DItemZoomZ", 0.0F },
                  { "fInventoryLight2DiffuseColorB", 0.8299999833106995F },
                  { "fInventoryLight2DiffuseColorG", 0.949999988079071F },
                  { "fInventoryLight2DiffuseColorR", 0.9800000190734863F },
                  { "fInventoryLight2DimmerValue", 1.75F },
                  { "fInventoryLight2Radius", 0.0F },
                  { "fInventoryLightDiffuseColorB", 0.8299999833106995F },
                  { "fInventoryLightDiffuseColorG", 0.949999988079071F },
                  { "fInventoryLightDiffuseColorR", 0.9800000190734863F },
                  { "fInventoryLightDimmerValue", 1.75F },
                  { "fInventoryLightRadius", 400.0F },
                  { "fInventoryMenuLight2X", 100.0F },
                  { "fInventoryMenuLight2Y", -350.0F },
                  { "fInventoryMenuLight2Z", 100.0F },
                  { "fInventoryMenuLightX", 100.0F },
                  { "fInventoryMenuLightY", -350.0F },
                  { "fInventoryMenuLightZ", 100.0F },
               #pragma endregion
               #pragma region J
                  { "fJournalLongRepeatRate", 0.20000000298023224F },
                  { "fJournalShortRepeatRate", 0.07500000298023224F },
               #pragma endregion
               #pragma region L
                  { "fLargeActivatePickLength_G", 500.0F },
                  { "fLockCenterOffset", 9.0F },
                  { "fLockMaxAngle", 90.0F },
                  { "fLockPositionX", 0.0F },
                  { "fLockPositionY", -1100.0F },
                  { "fLockPositionYWide", -1300.0F },
                  { "fLockPositionZ", 3.0F },
                  { "fLockRotCenterOffsetX", -14.5F },
                  { "fLockRotCenterOffsetZ", 3.0F },
                  { "fLockRotationSpeed", 80.0F },
                  { "fLockpickLightDiffuseColorB", 0.8299999833106995F },
                  { "fLockpickLightDiffuseColorG", 0.949999988079071F },
                  { "fLockpickLightDiffuseColorR", 0.9800000190734863F },
                  { "fLockpickLightDimmerValue", 1.75F },
                  { "fLockpickLightRadius", 400.0F },
                  { "fLockpickLightX", 100.0F },
                  { "fLockpickLightY", -1000.0F },
                  { "fLockpickLightZ", 100.0F },
               #pragma endregion
               #pragma region M
                  { "fMagic3DItemPosScale", 1.8700000047683716F },
                  { "fMagic3DItemPosScaleWide", 1.75F },
                  { "fMagic3DItemPosX", 29.0F },
                  { "fMagic3DItemPosXWide", 22.0F },
                  { "fMagic3DItemPosY", -500.0F },
                  { "fMagic3DItemPosYWide", -500.0F },
                  { "fMagic3DItemPosZ", 8.0F },
                  { "fMagic3DItemPosZWide", 6.0F },
                  { "fMaxSubtitleDistance", 1250.0F },
                  { "fMenuKeyRepeatLong", 0.5F },
                  { "fMenuKeyRepeatShort", 0.10000000149011612F },
                  { "fMinSecondsForLoadFadeIn", 1.5F },
               #pragma endregion
               #pragma region N
                  { "fNoteDistance", 90.0F },
               #pragma endregion
               #pragma region P
                  { "fPackratRatio", 0.0F },
                  { "fPickMouseRotationSpeed", 15.0F },
                  { "fPickRotationSpeed", 400.0F },
                  { "fPlayerBodyEditDistance", 175.0F },
                  { "fPlayerFaceEditDistance", 100.0F },
                  { "fPlayerRotationAngle", 30.0F },
                  { "fPlayerZoomTime", 1000.0F },
               #pragma endregion
               #pragma region R
                  { "fRSMCameraLookAtPercent", 0.9549999833106995F },
                  { "fRSMLookAtOnGain", 0.05999999865889549F },
               #pragma endregion
               #pragma region S
                  { "fSafeZoneX", 15.0F },
                  { "fSafeZoneXWide", 15.0F },
                  { "fSafeZoneY", 15.0F },
                  { "fSafeZoneYWide", 15.0F },
                  { "fSleepFaderTime", 0.699999988079071F },
               #pragma endregion
               #pragma region T
                  { "fTweenLongRepeatRate", 0.20000000298023224F },
                  { "fTweenShortRepeatRate", 0.10000000149011612F },
               #pragma endregion
               #pragma region U
                  { "fUIAltLogoModel_TranslateX_G", 0.0F },
                  { "fUIAltLogoModel_TranslateY_G", 0.0F },
                  { "fUIAltLogoModel_TranslateZ_G", 0.0F },
                  { "fUICameraFarDistance", 20480.0F },
                  { "fUICameraNearDistance", 15.0F },
                  { "fUILogoModel_AutoRotateSpeed", 0.10000000149011612F },
                  { "fUILogoModel_FadeSecs", 0.00009999999747378752F },
                  { "fUILogoModel_MouseThreshold", 2.0F },
                  { "fUILogoModel_MouseToPanSpeed", 1.0F },
                  { "fUILogoModel_MouseToRotateSpeed", 0.019999999552965164F },
                  { "fUILogoModel_MouseToZoomSpeed", 0.6000000238418579F },
                  { "fUILogoModel_RotationPauseDuration", 0.25F },
                  { "fUILogoModel_ThumbstickToPanSpeed", 8.0F },
                  { "fUILogoModel_ThumbstickToRotateSpeed", 0.44999998807907104F },
                  { "fUILogoModel_ThumbstickToZoomSpeed", 5.0F },
                  { "fUIMistMenu_CameraFOV_G", 75.0F },
                  { "fUIMistMenu_CameraLookAtX_G", -50.0F },
                  { "fUIMistMenu_CameraLookAtY_G", 0.0F },
                  { "fUIMistMenu_CameraLookAtZ_G", 0.0F },
                  { "fUIMistMenu_CameraX_G", -50.0F },
                  { "fUIMistMenu_CameraY_G", 600.0F },
                  { "fUIMistMenu_CameraZ_G", 80.0F },
                  { "fUIMistMenu_DefaultLogoNIFScale", 1.0F },
                  { "fUIMistMenu_LogoOnscreenPanThresholdX", 0.5F },
                  { "fUIMistMenu_LogoOnscreenPanThresholdY", 0.33000001311302185F },
                  { "fUIMistMenu_LogoOnscreenZoomMaxFOV", 95.0F },
                  { "fUIMistMenu_LogoOnscreenZoomMinFOV", 60.0F },
                  { "fUIMistMenu_LogoOnscreenZoomThresholdFar", 0.10000000149011612F },
                  { "fUIMistMenu_LogoOnscreenZoomThresholdNear", 3.5F },
                  { "fUIMistModel_FadeOutTime", 0.0F },
                  { "fUIMistModel_RotateZ_G", -180.0F },
                  { "fUIMistModel_TranslateX_G", 0.0F },
                  { "fUIMistModel_TranslateY_G", 0.0F },
                  { "fUIMistModel_TranslateZ_G", 0.0F },
                  { "fUIPlayerSceneLight2DiffuseColorB", 0.800000011920929F },
                  { "fUIPlayerSceneLight2DiffuseColorG", 0.8100000023841858F },
                  { "fUIPlayerSceneLight2DiffuseColorR", 0.699999988079071F },
                  { "fUIPlayerSceneLight2DimmerValue", 3.0F },
                  { "fUIPlayerSceneLight2Radius", 1024.0F },
                  { "fUIPlayerSceneLight2X", 160.0F },
                  { "fUIPlayerSceneLight2Y", -96.0F },
                  { "fUIPlayerSceneLight2Z", 160.0F },
                  { "fUIPlayerSceneLight3DiffuseColorB", 1.0F },
                  { "fUIPlayerSceneLight3DiffuseColorG", 1.0F },
                  { "fUIPlayerSceneLight3DiffuseColorR", 1.0F },
                  { "fUIPlayerSceneLight3DimmerValue", 0.10000000149011612F },
                  { "fUIPlayerSceneLight3Radius", 1024.0F },
                  { "fUIPlayerSceneLight3X", 128.0F },
                  { "fUIPlayerSceneLight3Y", 160.0F },
                  { "fUIPlayerSceneLight3Z", -96.0F },
                  { "fUIPlayerSceneLightDiffuseColorB", 0.8199999928474426F },
                  { "fUIPlayerSceneLightDiffuseColorG", 0.9599999785423279F },
                  { "fUIPlayerSceneLightDiffuseColorR", 0.9599999785423279F },
                  { "fUIPlayerSceneLightDimmerValue", 1.600000023841858F },
                  { "fUIPlayerSceneLightRadius", 1500.0F },
                  { "fUIPlayerSceneLightX", -160.0F },
                  { "fUIPlayerSceneLightY", 160.0F },
                  { "fUIPlayerSceneLightZ", 128.0F },
                  { "fUIRaceSexLight2DiffuseColorB", 0.8299999833106995F },
                  { "fUIRaceSexLight2DiffuseColorG", 0.949999988079071F },
                  { "fUIRaceSexLight2DiffuseColorR", 0.9800000190734863F },
                  { "fUIRaceSexLight2DimmerValue", 1.75F },
                  { "fUIRaceSexLight2Radius", 1400.0F },
                  { "fUIRaceSexLight2X", 0.5F },
                  { "fUIRaceSexLight2Y", -150.0F },
                  { "fUIRaceSexLight2Z", 60.5F },
                  { "fUIRaceSexLightDiffuseColorB", 0.8299999833106995F },
                  { "fUIRaceSexLightDiffuseColorG", 0.949999988079071F },
                  { "fUIRaceSexLightDiffuseColorR", 0.9800000190734863F },
                  { "fUIRaceSexLightDimmerValue", 0.6499999761581421F },
                  { "fUIRaceSexLightRadius", 1400.0F },
                  { "fUIRaceSexLightX", 0.5F },
                  { "fUIRaceSexLightY", -600.0F },
                  { "fUIRaceSexLightZ", 60.5F },
                  { "fUnlockDoorDelay", 1.5F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region F
                  { "iFavoriteItemQueueSize", 100 },
               #pragma endregion
               #pragma region M
                  { "iMaxViewCasterPicksFuzzy", 5 },
                  { "iMaxViewCasterPicksGamebryo", 10 },
                  { "iMaxViewCasterPicksHavok", 10 },
               #pragma endregion
               #pragma region S
                  { "iSubtitleSpeakerNameColor", 8947848 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region C
                  { "sCreditsFile", "Interface/Credits.txt" },
                  { "sCreditsFileFrench", "Interface/Credits_French.txt" },
                  { { game::skyrim_special }, "sCreditsFilePLRU", "Interface/Credits_PLRU.txt" },
               #pragma endregion
               #pragma region F
                  { "sForcedLoadScreenEditorID", "Not enough arguments..." },
               #pragma endregion
               #pragma region P
                  { "sPosePlayerRaceSexMenu", "OffsetBoundStandingPlayerInstant" },
               #pragma endregion
               #pragma region U
                  { "sUIMistMenu_DefaultLogoCameraPath", "Not enough arguments..." },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Kinect", {
            { "fKinectMaxAllyTradeDistance", 60000.0F },
            { "fKinectMinConfidence", 0.30000001192092896F },
            { "fKinectMinReportConfidence", 0.009999999776482582F },
            { "fKinectMinReportShoutConfidence", 0.004999999888241291F },
            { "fKinectMinRuleConfidence", 0.25F },
            { "fKinectMinShoutConfidence", 0.007499999832361937F },
         }),
         section_definition("Landscape", {
            { "bCurrentCellOnly", false },
            { { game::skyrim_special }, "bLandSpecular", true },
            { "fLandFriction", 2.5F },
            { "fLandTextureTilingMult", 3.0F },
            { "iLandBorder1B", 0 },
            { "iLandBorder1G", 255 },
            { "iLandBorder1R", 255 },
            { "iLandBorder2B", 0 },
            { "iLandBorder2G", 0 },
            { "iLandBorder2R", 0 },
            { "sDefaultLandDiffuseTexture", "Dirt02.dds" },
            { "sDefaultLandNormalTexture", "Dirt02_N.dds" },
         }),
         section_definition("LANGUAGE", {
            #pragma region Strings
               #pragma region F
                  { "sFailureMessage", "Something is broken" },
               #pragma endregion
               #pragma region G
                  { { game::skyrim_special }, "sGamepadDisconnectedMessage", "Please connect a controller to continue." },
                  { { game::skyrim_special }, "sGamepadDisconnectedTitle", "Controller disconnected." },
               #pragma endregion
               #pragma region S
                  { "sSysUtil_AutoSaveWarning", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_AwardsConfiguring", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_AwardsInstalling", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_AwardsLoading", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_AwardsReinstalling", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_AwardsUpdating", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_DirtyDisc", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_DiscEject", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_GameContentInstalling", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_GameDataCorrupt", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_GameDataInsufficientSpace", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_NPDRMInstalling", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_Retry", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_SaveDataCreateNew", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_SaveDataInsufficientSpace", "No Default string.  Must be loaded from INIFile" },
                  { "sSysUtil_SaveDataOwnershipWarning", "No Default string.  Must be loaded from INIFile" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Light", {
            { "fLightEnableDisableFadeTime", 1.0F },
         }),
         section_definition("LightingShader", {
            { "fDecalLODFadeEnd", 0.05999999865889549F },
            { "fDecalLODFadeStart", 0.05000000074505806F },
            { "fEnvmapLODFadeEnd", 0.10000000149011612F },
            { "fEnvmapLODFadeStart", 0.09000000357627869F },
            { "fEyeEnvmapLODEnd", 0.05000000074505806F },
            { "fRefractionLODFadeEnd", 0.029999999329447746F },
            { "fRefractionLODFadeStart", 0.02500000037252903F },
            { "fSpecularLODFadeEnd", 0.10000000149011612F },
            { "fSpecularLODFadeStart", 0.09000000357627869F },
         }),
         section_definition("LOD", {
            #pragma region Booleans
               #pragma region D
                  { "bDisplayLODLand", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fActorLODMax", 15.0F },
                  { "fActorLODMin", 2.0F },
               #pragma endregion
               #pragma region D
                  { "fDistanceMultiplier", 1.0F },
               #pragma endregion
               #pragma region F
                  { "fFadeInThreshold", 0.699999988079071F },
                  { "fFadeInTime", 1.2000000476837158F },
                  { "fFadeOutThreshold", 0.30000001192092896F },
                  { "fFadeOutTime", 2.0F },
               #pragma endregion
               #pragma region I
                  { "fItemLODMax", 15.0F },
                  { "fItemLODMin", 1.0F },
               #pragma endregion
               #pragma region L
                  { "fLODBoundRadiusMult", 10.0F },
                  { "fLODFadeOutActorMultCity", 1.0F },
                  { "fLODFadeOutActorMultComplex", 1.0F },
                  { "fLODFadeOutActorMultInterior", 1.0F },
                  { "fLODFadeOutItemMultCity", 1.0F },
                  { "fLODFadeOutItemMultComplex", 1.0F },
                  { "fLODFadeOutItemMultInterior", 1.0F },
                  { "fLODFadeOutObjectMultCity", 1.0F },
                  { "fLODFadeOutObjectMultComplex", 1.0F },
                  { "fLODFadeOutObjectMultInterior", 1.0F },
                  { "fLODFadeOutPercent", 0.6000000238418579F },
                  { "fLODLandDropAmount", 230.0F },
                  { "fLODLandVerticalBias", 0.0F },
                  { "fLODMultTrees", 0.5F },
                  { "fLodDistance", 500.0F },
               #pragma endregion
               #pragma region O
                  { "fObjectLODMax", 15.0F },
                  { "fObjectLODMin", 1.0F },
               #pragma endregion
               #pragma region T
                  { "fTalkingDistance", 2000.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region F
                  { "iFadeNodeMinNearDistance", 500 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("LookIK", {
            { "bAdditiveHeadAnim", true },
            { "fAngleMax", 2.0F },
            { "fAngleMaxEase", 90.0F },
            { "fEaseAngleShutOff", 0.5F },
            { "fEyeIKDistanceMax", 2.5F },
            { "fMaxTrackingDist", 5000.0F },
            { "fMinTrackingDist", 12.0F },
         }),
         section_definition("MAIN", {
            { "fQuestScriptDelayTime", 5.0F },
         }),
         section_definition("MapMenu", {
            #pragma region Booleans
               #pragma region W
                  { "bWorldMapNoSkyDepthBlur", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region G
                  { "fGamepadCursorSensitivity", 2.0F },
               #pragma endregion
               #pragma region M
                  { "fMapLocalCursorPanSpeed", 2000.0F },
                  { "fMapLocalGamepadPanSpeed", 100.0F },
                  { "fMapLocalGamepadZoomSpeed", 0.029999999329447746F },
                  { "fMapLocalHeight", 40000.0F },
                  { "fMapLocalMarkerSelectionDist", 0.029999999329447746F },
                  { "fMapLocalMinFrustumWidth", 4000.0F },
                  { "fMapLocalMousePanSpeed", 20.0F },
                  { "fMapLocalMouseZoomSpeed", 0.10000000149011612F },
                  { "fMapLookGamepadSpeed", 1.5F },
                  { "fMapLookMouseSpeed", 3.0F },
                  { "fMapLoopFadeTimeSeconds", 1.0F },
                  { "fMapMenuNearClipPlane", 128.0F },
                  { "fMapMenuOverlayNormalSnowStrength", 0.4000000059604645F },
                  { "fMapMenuOverlayNormalStrength", 1.100000023841858F },
                  { "fMapMenuOverlayScale", 0.000035000000934815034F },
                  { "fMapMenuOverlaySnowScale", 0.000045000000682193786F },
                  { "fMapMoveKeyboardSpeed", 0.019999999552965164F },
                  { "fMapTransitionSpeed", 0.75F },
                  { "fMapWorldCursorMoveArea", 0.8999999761581421F },
                  { "fMapWorldHeightAdjustmentForce", 4.0F },
                  { "fMapWorldMaxPanSpeed", 75000.0F },
                  { "fMapWorldMaxPitch", 75.0F },
                  { "fMapWorldMinPanSpeed", 60000.0F },
                  { "fMapWorldMinPitch", 15.0F },
                  { "fMapWorldTransitionHeight", 10000.0F },
                  { "fMapWorldYawRange", 80.0F },
                  { "fMapWorldZoomSpeed", 2.0F },
                  { "fMapZoomMouseSpeed", 2.0F },
                  { "fMaxMarkerSelectionDist", 0.003000000026077032F },
               #pragma endregion
               #pragma region W
                  { "fWorldMapDepthBlurScale", 0.30000001192092896F },
                  { "fWorldMapFocalDepth", 45000.0F },
                  { "fWorldMapMaximumDepthBlur", 0.44999998807907104F },
                  { "fWorldMapNearDepthBlurScale", 4.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region R
                  { "iRightStickRepeatRate", 250 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region M
                  { "sMapWorldDefaultWorldSpace", "Tamriel" },
               #pragma endregion
               #pragma region W
                  { "sWorldMapOverlayNormalSnowTexture", "Data\\Textures\\Terrain\\WorldMapOverlaySnow_n.dds" },
                  { "sWorldMapOverlayNormalTexture", "Data\\Textures\\Terrain\\WorldMapOverlay_n.dds" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Menu", {
            { "iConsoleHistoryCharBufferSize", 16384 },
            { "iConsoleSizeScreenPercent", 40 },
            { "iConsoleTextSize", 20 },
         }),
         section_definition("MESSAGES", {
            { "bAllowFileWrite", true },
            { "bAllowYesToAll", true },
            { { game::skyrim_special }, "bAssertsWithoutDebugger", false },
            { "bBlockMessageBoxes", false },
            { "bDisableAssertQueuing", true },
            { "bFaceGenWarnings", false },
            { { game::skyrim_special }, "bNoBreaksForAsserts", false },
            { "bShowMissingAudioWarnings", true },
            { "bShowMissingLipWarnings", true },
            { "bSkipInitializationFlows", true },
            { "bSkipProgramFlows", true },
            { "bUseWindowsMessageBox", false },
            { "iFileLogging", 0 },
            { { game::skyrim_special }, "sDisabledProgramFlowContexts", "Not enough arguments..." },
         }),
         section_definition("NavMeshGeneration", {
            { "bGlobalNavMeshCheck", false },
            { "bGlobalNavMeshCheckDeleteWarningTriangles", false },
         }),
         section_definition("Papyrus", {
            { "bEnableLogging", false },
            { "bEnableProfiling", false },
            { "bEnableTrace", false },
            { "bLoadDebugInformation", false },
            { "fArchiveInitBufferMB", 8.0F },
            { "fExtraTaskletBudgetMS", 1.2000000476837158F },
            { "fPostLoadUpdateTimeMS", 2000.0F },
            { "fUpdateBudgetMS", 1.2000000476837158F },
            { "iMaxAllocatedMemoryBytes", 76800 },
            { "iMaxMemoryPageSize", 512 },
            { "iMinMemoryPageSize", 128 },
         }),
         section_definition("Pathfinding", {
            #pragma region Booleans
               #pragma region A
                  { "bAvoidBoxTriggersFailure", false },
               #pragma endregion
               #pragma region B
                  { "bBackgroundNavmeshUpdate", true },
                  { "bBackgroundPathing", true },
               #pragma endregion
               #pragma region C
                  { "bCreateDebugInfo", false },
                  { "bCutDoors", true },
               #pragma endregion
               #pragma region D
                  { "bDisableUnloadedPaths", false },
               #pragma endregion
               #pragma region F
                  { "bFacePathVector", true },
                  { "bFixNavmeshInfosOnLoad", false },
               #pragma endregion
               #pragma region I
                  { "bIgnoreThresholds", false },
               #pragma endregion
               #pragma region R
                  { "bRebuildPathIfSmootherFailed", true },
               #pragma endregion
               #pragma region S
                  { "bSlowDownForActorAvoidance", false },
                  { "bStaticAvoidanceTriggerMovementBlocked", false },
               #pragma endregion
               #pragma region U
                  { "bUseActorAvoidBox", true },
                  { "bUseActorAvoidance", true },
                  { "bUseAlternateSmoothingForPrime", true },
                  { "bUseOldPathSmoothing", false },
                  { "bUsePathSmoothing", true },
                  { "bUseRayCasts", true },
                  { "bUseStraightLineCheckFirst", true },
                  { "bUseTangentSmoothing", true },
                  { "bUseTaskletsToRecomputeBounds", false },
                  { "bUseTweenedAnimations", true },
                  { "bUseVelocityObstacles", true },
               #pragma endregion
               #pragma region W
                  { "bWarnIfHighLevelSearchFails", false },
                  { "bWarpOnConsecutiveFailures", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fAICombatTurnSpeedScale", 2.5F },
                  { "fAITurnSpeedScale", 1.5F },
                  { "fAcceptableErrorRatio", 0.8999999761581421F },
                  { "fAvoidNodeCost", 24.0F },
                  { "fAvoidNodeRadiusAdd", 11.0F },
                  { "fAvoidPreferredTriangleCrossingMultiplier", 10.0F },
                  { "fAvoidPreferredTriangleMultiplier", 3.0F },
                  { "fAvoidanceDistanceRadiusMult", 1.0F },
                  { "fAvoidanceTimeDelta", 3.0F },
               #pragma endregion
               #pragma region B
                  { "fBackPedalAngle", 160.0F },
                  { "fBadTriangleMultiplier", 100.0F },
               #pragma endregion
               #pragma region D
                  { "fDefaultAvoidBoxAvoidNodeRadius", 32.0F },
                  { "fDefaultAvoidNodeCost", 32.0F },
                  { "fDefaultPreferredFactor", 0.5F },
                  { "fDefaultStaticAvoidNodeRadius", 32.0F },
                  { "fDefaultTangentSmoothingFactor", 1.0F },
                  { "fDistFromPathForFollowingRadiusMult", 2.0F },
               #pragma endregion
               #pragma region F
                  { "fFindMaxSpeedMinParamIncrementPercent", 0.10000000149011612F },
                  { "fFollowerTeleportOffsetFudge", 10.0F },
               #pragma endregion
               #pragma region H
                  { "fHeadingToPathTangentMaxAngle", 15.0F },
               #pragma endregion
               #pragma region L
                  { "fLedgeJumpHeightBuffer", 16.0F },
               #pragma endregion
               #pragma region M
                  { "fMaxAvoidanceRadius", 512.0F },
                  { "fMaxCollisionTime", 1.0F },
                  { "fMaxDistFromPathRadiusMult", 5.0F },
                  { "fMaxDistanceFromNavmeshMult", 0.25F },
                  { "fMaxDistanceMoved", 5.0F },
                  { "fMaxEdgeLength", 512.0F },
                  { "fMaxFitnessMultiplier", 2.0F },
                  { "fMaxHeightFromNavmeshToObstacleBoxBottom", 128.0F },
                  { "fMaxHeightFromObstacleBoxToToNavmesh", -16.0F },
                  { "fMaxTimeBlockedByActors", 1.0F },
                  { "fMaxTimeSizeAvoidNode", 1.0F },
                  { "fMinAvoidanceRadius", 256.0F },
                  { "fMinCollisionTime", 0.25F },
                  { "fMinDist", 50.0F },
                  { "fMinFailureDistance", 50.0F },
                  { "fMinFrictionSpeed", 2.0F },
                  { "fMinNormalizedSpeedForSlowdown", 0.75F },
                  { "fMinStairSpeed", 80.0F },
                  { "fMinTimeToNextPoint", 0.30000001192092896F },
                  { "fMinimalUsePathingCost", 409600.0F },
                  { "fMovementBlockedTimer", 0.019999999552965164F },
               #pragma endregion
               #pragma region N
                  { "fNavmeshBoundsActorRadiusMultiplier", 1.0F },
                  { "fNavmeshBoundsMinTimeOfImpact", 0.0333000011742115F },
                  { "fNodeDistanceThreshold", 25.0F },
               #pragma endregion
               #pragma region O
                  { "fObstacleManagerMinHeight", 48.0F },
                  { "fObstacleManagerMinWidth", 16.0F },
                  { "fObstacleUpdateDeltaWhenMoving", 1.0F },
                  { "fObstacleUpdateDeltaWhenUnknown", 5.0F },
               #pragma endregion
               #pragma region P
                  { "fPOVSmootherAvoidNodeCost", 7.0F },
                  { "fPathManagerDebugInfoWindow", 1.0F },
                  { "fPathToAnimLengthMaxMultiplier", 2.0F },
                  { "fPathingLargeActorRadius", 80.0F },
                  { "fPreferredTriangleMultiplier", 0.009999999776482582F },
               #pragma endregion
               #pragma region R
                  { "fRotateTowardsPathThreshold", 5.0F },
               #pragma endregion
               #pragma region S
                  { "fShortPathRadiusMult", 2.0F },
                  { "fSlowDownMultiplier", 0.5F },
                  { "fSmoothingStepHeight", 25.0F },
                  { "fSprintAccelerationMult", 10.0F },
                  { "fSprintAngleToPathThreshold", 5.0F },
                  { "fSprintDistToPathThresholdRadiusMult", 0.5F },
                  { "fStaticPathTangentSmoothingFactor", 0.5F },
               #pragma endregion
               #pragma region T
                  { "fTeleportNodeAngleTolerance", 5.0F },
                  { "fTotalDisplacementThresholdRadiusMult", 0.6600000262260437F },
                  { "fTotalTimePadding", 0.5F },
                  { "fTotalTimeThreshold", 1.0F },
                  { "fTweenerAnimDurationOffset", 0.10000000149011612F },
                  { "fTweeningMaxPercentSpeedDelta", 0.20000000298023224F },
               #pragma endregion
               #pragma region W
                  { "fWarpMaxTime", 5.0F },
                  { "fWarpPathOffset", 100.0F },
                  { "fWarpRequestActorRadius", 5.0F },
                  { "fWaterTriangleCostMultiplier", 4.0F },
                  { "fWaterTriangleCrossingCostMultiplier", 5.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region D
                  { "iDefaultRetryCount", 3 },
               #pragma endregion
               #pragma region F
                  { "iFindMaxSpeedMinIterationCount", 10 },
               #pragma endregion
               #pragma region M
                  { "iMaxActorsToAvoid", 10 },
                  { "iMaxAvoidBoxCheckPerFrame", 2 },
                  { "iMaxHavokRequestsPerFrame", 2 },
                  { "iMaxObstacleBuildPerFrame", 1 },
                  { "iMaxPathRequestsPerFrameTracked", 32 },
                  { "iMaxQueuedPathingRequests", 50 },
               #pragma endregion
               #pragma region P
                  { "iPathRequestsAllowedPerFrame", 2 },
               #pragma endregion
               #pragma region W
                  { "iWarpMaxPathFailureCount", 3 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Pathing", {
            { "fINIDetectDoorsForPathingTime", 0.5F },
         }),
         section_definition("RagdollAnim", {
            #pragma region Booleans
               #pragma region F
                  { "bFootIK", true },
               #pragma endregion
               #pragma region G
                  { "bGrabIK", true },
               #pragma endregion
               #pragma region L
                  { "bLookIK", true },
               #pragma endregion
               #pragma region P
                  { "bPoseMatching", true },
               #pragma endregion
               #pragma region R
                  { "bRagdollAnim", true },
                  { "bRagdollFeedback", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fAccelerationGain", 1.0F },
               #pragma endregion
               #pragma region C
                  { "fCameraDist", 1000.0F },
               #pragma endregion
               #pragma region D
                  { "fDesiredVel", 1.5F },
                  { "fDetectionUpdateTimeSec", 5.0F },
               #pragma endregion
               #pragma region F
                  { "fFeedbackImpulseMult", 500.0F },
                  { "fFeedbackOnOffGain", 0.30000001192092896F },
                  { "fFeedbackOnOffGainTimeMS", 1000.0F },
                  { "fFeedbackTimeMS", 10000.0F },
               #pragma endregion
               #pragma region H
                  { "fHierarchyGain", 0.17000000178813934F },
               #pragma endregion
               #pragma region I
                  { "fImpulseLimit", 15.0F },
               #pragma endregion
               #pragma region P
                  { "fPositionGain", 0.05000000074505806F },
                  { "fPositionMaxAngularVelocity", 18.0F },
                  { "fPositionMaxLinearVelocity", 14.0F },
               #pragma endregion
               #pragma region S
                  { "fSnapGain", 0.10000000149011612F },
                  { "fSnapMaxAngularDistance", 1.0F },
                  { "fSnapMaxAngularVelocity", 0.30000001192092896F },
                  { "fSnapMaxLinearDistance", 0.30000001192092896F },
                  { "fSnapMaxLinearVelocity", 3.0F },
               #pragma endregion
               #pragma region V
                  { "fVelocityDamping", 0.0F },
                  { "fVelocityGain", 0.6000000238418579F },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("SaveGame", {
            #pragma region Booleans
               #pragma region A
                  { "bAllowProfileTransfer", false },
                  { "bAllowScriptedAutosave", true },
                  { "bAllowScriptedForceSave", true },
                  { { game::skyrim_special }, "bAutoSaveOnUserStale", true },
               #pragma endregion
               #pragma region C
                  { { game::skyrim_special }, "bCompressBuffer", false },
                  { { game::skyrim_special }, "bConvertNonUtilitySaves", false },
                  { "bCopySaveGameToHostOrMemStick", false },
               #pragma endregion
               #pragma region D
                  { "bDisableAutoSave", false },
                  { "bDisplayMissingContentDialogue", true },
               #pragma endregion
               #pragma region O
                  { "bOutputSaveGameScreenshot", false },
               #pragma endregion
               #pragma region U
                  { "bUsePagedBuffers", true },
                  { "bUseSaveGameHistory", false },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region A
                  { "iAutoSaveCount", 3 },
               #pragma endregion
               #pragma region S
                  { "iSaveGameBackupCount", 1 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region S
                  { "sSaveGameGameVersionOutdated", "This save game was created on a later version of Skyrim. Please download any updates." },
                  { "sSaveGameSafeMarkerID", "1DC0A" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("ScreenSplatter", {
            { "bBloodSplatterEnabled", true },
         }),
         section_definition("SpeedTree", {
            { "fLODTreeMipMapLODBias", -0.75F },
            { "fLocalTreeMipMapLODBias", -0.25F },
         }),
         section_definition("StreamInstall", {
            { { game::skyrim_special }, "bResetGameAfterStreamingInstall", false },
         }),
         section_definition("Terrain", {
            { "fHDLODSnowThresholdAngle", 87.0F },
            { "fLODSnowThresholdAngle", 100.0F },
         }),
         section_definition("TerrainManager", {
            { "bDisplayCloudLOD", true },
            { "bKeepLowDetailTerrain", true },
            { "bUseNewTerrainSystem", true },
            { "fCameraAboveMaxHeightThreshold", 2048.0F },
         }),
         section_definition("TestAllCells", {
            { "bFileCheckModelCollision", false },
            { "bFileControllerOnRoot", true },
            { "bFileGoneMessage", true },
            { "bFileNeededMessage", true },
            { "bFileShowIcons", true },
            { "bFileShowTextures", true },
            { "bFileSkipIconChecks", false },
            { "bFileSkipModelChecks", false },
            { "bFileTestLoad", false },
            { "bFileUnusedObject", false },
         }),
         section_definition("Trees", {
            { "bEnableTreeAnimations", true },
            { "bEnableTrees", true },
            { "bForceFullDetail", false },
            { "bPickSkinnedTrees", true },
            { "fUpdateBudget", 1.5F },
         }),
         section_definition("VATS", {
            #pragma region Booleans
               #pragma region V
                  { "bVATSAllowNoKill", false },
                  { "bVATSDisable", false },
                  { "bVATSForceRanged", false },
                  { "bVATSIgnoreProjectileTest", false },
                  { "bVATSMultipleCombatants", false },
                  { "bVATSRangedSelective", true },
                  { "bVATSSmartCameraCheckDebug", false },
                  { "bVatsDebug", false },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region V
                  { "fVATSCastingAfterKillDelay", 1.2000000476837158F },
                  { "fVATSFocus", 3.200000047683716F },
                  { "fVATSKillMoveEnd", 4.0F },
                  { "fVATSLightAngle", 0.0F },
                  { "fVATSLightDistance", 100.0F },
                  { "fVATSLightElevation", 100.0F },
                  { "fVATSLightLevelMax", 40.0F },
                  { "fVATSLightLevelMin", 20.0F },
                  { "fVATSRangedLongDistance", 2500.0F },
                  { "fVATSRangedLowHealthPercent", 5.0F },
                  { "fVATSRangedPercentMin", 50.0F },
                  { "fVATSRangedPercentSneakKill", 20.0F },
                  { "fVATSRangedPercentTargetNoThreat", 10.0F },
                  { "fVATSRangedTargetLowLevelMult", 0.30000001192092896F },
                  { "fVatsLightColorB", 1.0F },
                  { "fVatsLightColorG", 1.0F },
                  { "fVatsLightColorR", 1.0F },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Verlet", {
            { "bUseConstantStepDelta", true },
            { "bUseRotationBasedTransformUpdate", true },
            { "fGravity", -1500.0F },
            { "fVerticalCapsuleOffset", 50.0F },
         }),
         section_definition("Voice", {
            { "sFileTypeGame", "wav" },
            { "sFileTypeLTF", "ltf" },
            { "sFileTypeLip", "lip" },
            { "sFileTypeSource", "wav" },
         }),
         section_definition("Water", {
            #pragma region Booleans
               #pragma region A
                  { "bAutoWaterSilhouetteReflections", false },
               #pragma endregion
               #pragma region F
                  { "bForceHighDetailReflections", false },
                  { "bForceLowDetailReflections", false },
                  { "bForceLowDetailWater", false },
               #pragma endregion
               #pragma region R
                  { "bReflectExplosions", false },
                  { "bReflectLODLand", true },
                  { "bReflectLODObjects", false },
                  { "bReflectLODTrees", false },
                  { "bReflectSky", false },
               #pragma endregion
               #pragma region U
                  { "bUseBulletWaterDisplacements", true },
                  { "bUseCubeMapReflections", true },
                  { "bUsePerWorldSpaceWaterNoise", true },
                  { "bUseWater", true },
                  { "bUseWaterHiRes", false },
                  { "bUseWaterLOD", true },
                  { "bUseWaterReflectionBlur", false },
                  { "bUseWaterShader", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region C
                  { "fCubeMapRefreshRate", 0.0F },
               #pragma endregion
               #pragma region E
                  { "fExteriorWaterReflectionThreshold", 300.0F },
               #pragma endregion
               #pragma region I
                  { "fInteriorWaterReflectionThreshold", 10.0F },
               #pragma endregion
               #pragma region R
                  { "fRefractionWaterPlaneBias", 3.0F },
               #pragma endregion
               #pragma region S
                  { "fSurfaceTileSize", 2048.0F },
               #pragma endregion
               #pragma region T
                  { "fTileTextureDivisor", 4.75F },
               #pragma endregion
               #pragma region W
                  { "fWadingWaterQuadSize", 2048.0F },
                  { "fWadingWaterTextureRes", 512.0F },
                  { "fWaterGroupHeightRange", 10.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region W
                  { "iWaterBlurAmount", 1 },
                  { "iWaterNoiseResolution", 256 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               #pragma region S
                  { "sSurfaceTexture", "water" },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("Weather", {
            { "bFogEnabled", true },
            { "bPrecipitation", true },
            { "fAlphaReduce", 1.0F },
            { "fSunBaseSize", 425.0F },
            { { game::skyrim_special }, "fSunBoost", 1.0F },
            { { game::skyrim_special }, "fSunGlareMultiplier", 2.0F },
            { "fSunGlareSize", 600.0F },
            { "sBumpFadeColor", "255,255,255,255" },
            { "sEnvReduceColor", "255,255,255,255" },
            { "sLerpCloseColor", "255,255,255,255" },
         }),
      });
      #pragma endregion
   }
}