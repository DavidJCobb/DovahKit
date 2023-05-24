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
   extern constexpr setting_type get_setting_type_from_name(const char* name) {
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
         case 'u':
         case 'U':
            return setting_type::integer_unsigned;
         case 's':
         case 'S':
            return setting_type::string;
      }
      return setting_type::none;
   }

   section_definition::section_definition(const char* name, std::initializer_list<setting_definition> settings) : name(name), settings(settings) {
   }

   file_definition::file_definition(const char* name, std::initializer_list<section_definition> sections) : filename(name), sections(sections) {
   }
   const setting_definition* file_definition::lookup(const char* section, const char* setting) const noexcept {
      for (auto& a : this->sections) {
         if (_stricmp(a.name.c_str(), section) != 0)
            continue;
         for (auto& b : a.settings) {
            if (_stricmp(b.name, setting) == 0)
               return &b;
         }
         return nullptr;
      }
      return nullptr;
   }
   #pragma endregion

   namespace files {
      namespace {
         static constexpr game_list sse_only = game_list::from<game::skyrim_special>();
      }

      #pragma region Skyrim.ini
      extern const file_definition skyrim = file_definition("Skyrim", {
         section_definition("", { // Unnamed section
            #pragma region Booleans
               { "bEnableLipLookup", true },
               { "bPrimitivesOn", false },
               { "bUseWaterHDR", true },
            #pragma endregion
            #pragma region Floats
               { sse_only, "fFadingFracStart", 0.25F },
               { "fKeyboardRepeatDelay", 0.3F },
               { "fKeyboardRepeatRate", 0.05F },
               { "fLowPerfCombatantVoiceDistance", 1000.0F },
               { "fMapWorldTargetTransitionTime", 0.5F },
               { sse_only, "fSnowSSSDarkColorIntensity", 0.1F },
            #pragma endregion
            #pragma region Integers
               { "iDetectionHighNumPicks", 40 },
               { "iLastHDRSetting", -1 },
               { "iMaxQuestObjectives", 3000 },
               { sse_only, "iSnowNoiseTextureSize", 512 },
            #pragma endregion
            #pragma region Strings
               { "sControlsDefinitionFile", "Interface/Controls/PC/ControlMap.txt" },
               { "sControlsRemapFile", "ControlMap_Custom.txt" },
               { "sGamepadDefinitionFile", "Interface/Controls/PC/Gamepad.txt" },
               { "sKeyboardDefinitionFile", "Interface/Controls/PC/Keyboard_" },
               { "sMouseDefinitionFile", "Interface/Controls/PC/Mouse.txt" },
               { "sSaveGameScreenshotName", "BGSSaveLoadHeader_Screenshot" },
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
                  { sse_only, "bDrawAnimPoseInVDB", false },
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
                  { "fAnimInterpMinTime", 0.08F },
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
                  { "fMaxFrameCounterDifferenceToConsiderVisible", 0.06666666666666667F },
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
                  { "fPlayerCharacterPowerAttackStartTime", 0.36666666666666664F },
               #pragma endregion
               #pragma region S
                  { "fSpecialIdlePickTime", 250.0F },
               #pragma endregion
               #pragma region W
                  { "fWeaponChangeClearTime", 0.5F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               { "iMinBonesToGenerateWhileSitting", 5 },
               { "iPlayerCharacterImagespaceModifierAnimCount", 2 },
            #pragma endregion
            #pragma region Strings
               { "sHkxDBNameContextPrefix", "meshes\\" },
               { "strPlayerCharacterBehavior1stPGraph", "Actors\\Character\\_1stPerson\\FirstPerson.hkx" },
            #pragma endregion
         }),
         section_definition("Archive", {
            #pragma region Booleans
               { "bCheckRuntimeCollisions", false },
               { sse_only, "bForceAsync", false },
               { "bInvalidateOlderFiles", true },
               { sse_only, "bLoadArchiveInMemory", false },
               { sse_only, "bLoadEsmInMemory", true },
               { "bTrackFileLoading", false },
               { "bUseArchives", true },
            #pragma endregion
            #pragma region Integers
               { "iRetainDirectoryStringTable", 1 },
               { "iRetainFilenameOffsetTable", 1 },
               { "iRetainFilenameStringTable", 1 },
            #pragma endregion
            #pragma region Strings
               #pragma region A
                  { "sArchiveList", "Skyrim - Textures.bsa, Skyrim - Meshes.bsa, Skyrim - Voices.bsa" },
                  { sse_only, "sArchiveToLoadInMemoryList", "Skyrim - Animations.bsa, Skyrim - Interface.bsa, Skyrim - Misc.bsa, Skyrim - Sounds.bsa" },
               #pragma endregion
               #pragma region E
                  { sse_only, "sEsmToLoadInMemoryList", "Skyrim.esm, Update.esm, Dawnguard.esm, HearthFires.esm, Dragonborn.esm" },
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
               { "bEnableAudio", true },
               { "bEnableAudioCache", true },
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fASFadeInTime", 3.0F },
                  { "fASFadeOutTime", 8.0F },
                  { sse_only, "fAudioRumbleBigDeadZone", 0.0F },
                  { "fAudioRumbleBigExponent", 0.5F },
                  { sse_only, "fAudioRumbleBigLerpMax", 1.0F },
                  { sse_only, "fAudioRumbleBigLerpMin", 0.0F },
                  { "fAudioRumblePowerAttackAdj", 0.15F },
                  { sse_only, "fAudioRumbleSmallDeadZone", 0.0F },
                  { "fAudioRumbleSmallExponent", 0.4F },
                  { sse_only, "fAudioRumbleSmallLerpMax", 1.0F },
                  { sse_only, "fAudioRumbleSmallLerpMin", 0.0F },
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
               { "iCollisionSoundTimeDelta", 150 },
               { "iHighlightSpeechOverlap", 500 },
            #pragma endregion
            #pragma region Strings
               { "sAudioAPI", "XAudio2" },
               { "sDeathCameraEffect", "MAGShoutSlowTimeActiveLP" },
               { "sMissingAssetSoundFile", "Not enough arguments..." },
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
               { sse_only, "bAutoSkipMainMenuLogin", true },
               { sse_only, "bEnableLocalLogging", false },
               { sse_only, "bEnablePlatform", true },
               { sse_only, "bModShowDownloadInsteadOfInstallLimit", false },
               { sse_only, "bTESTGameDataWait", true },
            #pragma endregion
            #pragma region Floats
               { sse_only, "fEULATimeOutSeconds", 30.0F },
               { sse_only, "fModIntroWarningDisplayTime", 5.0F },
               { sse_only, "fModLimitVisibilityThreshold", 0.85F },
               { sse_only, "fSteamPollDuration", 90.0F },
               { sse_only, "fSteamPollInterval", 10.0F },
            #pragma endregion
            #pragma region Integers
               { sse_only, "iBethesdaEntitlementsProductId", 0 },
               { sse_only, "iBethesdaLoggingProductId", 0 },
               { sse_only, "iBethesdaMotdProductId", 0 },
               { sse_only, "iEnableEventLogging", -1 },
               { sse_only, "iEnableLogging", -1 },
               { sse_only, "iIsLiveEventLogging", -1 },
               { sse_only, "iMinHttpResonseStatusToLog", 0 },
            #pragma endregion
            #pragma region Strings
               #pragma region A
                  { sse_only, "sAccountsURL", "Not enough arguments..." },
               #pragma endregion
               #pragma region B
                  { sse_only, "sBethesdaBeamKey", "Not enough arguments..." },
                  { sse_only, "sBethesdaLoggingKey", "Not enough arguments..." },
                  { sse_only, "sBethesdaOAuthId", "Not enough arguments..." },
               #pragma endregion
               #pragma region C
                  { sse_only, "sCreationsDownloadDirectory", "Creations" },
                  { sse_only, "sCreationsInstallDirectory", "Not enough arguments..." },
               #pragma endregion
               #pragma region D
                  { sse_only, "sDeveloperEmail", "Not enough arguments..." },
                  { sse_only, "sDeveloperUsername", "Not enough arguments..." },
               #pragma endregion
               #pragma region E
                  { sse_only, "sEnvironment", "Auto" },
               #pragma endregion
               #pragma region M
                  { sse_only, "sModsDownloadDirectory", "Mods" },
                  { sse_only, "sModsInstallDirectory", "Not enough arguments..." },
                  { sse_only, "sModsURL", "Not enough arguments..." },
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
            { "fMsActiveRefCount", 0.05F },
            { "fMsActorRefCount", 0.245F },
            { "fMsAnimatedObjectsCount", 0.05F },
            { "fMsDecalCount", 0.001F },
            { "fMsEmittersCount", 0.01F },
            { "fMsGeometryCount", 0.01F },
            { "fMsHavokTriCount", 0.001F },
            { "fMsLightCount", 0.01F },
            { "fMsLightExcessGeometry", 0.01F },
            { "fMsParticlesCount", 0.001F },
            { "fMsRefCount", 0.021F },
            { "fMsTriangleCount", 0.0001F },
            { "fMsWaterCount", 0.1F },
         }),
         section_definition("Camera", {
            #pragma region Booleans
               { "bDisableAutoVanityMode", false },
               { "bDragonCameraTargetPlayer", true },
               { sse_only, "bForceAutoVanityMode", false },
               { "bReturnTo1stPersonFromVanity", false },
            #pragma endregion
            #pragma region Floats
               #pragma region 1
                  { "f1st3rdSwitchDelay", 0.25F },
                  { sse_only, "f1stHorzDampeningSpringConstant", 0.0001F },
                  { sse_only, "f1stHorzDampeningVelocityDampening", 0.4F },
                  { "f1stPitchOffsetMouseFollowSpeed", 15.0F },
                  { "f1stPitchOffsetMouseMaxLag", 4.0F },
                  { "f1stPitchOffsetMultOffAccel", 1.0F },
                  { "f1stPitchOffsetMultOffMaxSpeed", 1.0F },
                  { "f1stPitchOffsetMultOnAccel", 0.5F },
                  { "f1stPitchOffsetMultOnMaxSpeed", 0.6F },
                  { "f1stPitchOffsetTarget", 0.75F },
                  { sse_only, "f1stVertDampeningSpringConstant", 0.001F },
                  { sse_only, "f1stVertDampeningVelocityDampening", 0.6F },
               #pragma endregion
               #pragma region A
                  { "fActorFadeOutLimit", 30.0F },
                  { "fAutoVanityIncrement", 0.01F },
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
                  { "fFirstPersonSittingRotationSpeed", 0.1F },
                  { "fFreeCameraRotationSpeed", 3.0F },
                  { "fFreeCameraRunSpeed", 2.0F },
                  { "fFreeCameraTranslationSpeed", 20.0F },
                  { "fFreeCameraTriggerDeadzone", 0.1F },
                  { "fFreeRotationSpeed", 3.0F },
                  { "fFurnitureCameraAngle", 0.39269909262657166F },
                  { "fFurnitureCameraZoom", 250.0F },
               #pragma endregion
               #pragma region H
                  { "fHorseDismountYawCorrection", 0.32F },
                  { "fHorseMaxAngleBeforeTurn", 90.0F },
               #pragma endregion
               #pragma region L
                  { "fLookingSpeed", 0.1F },
               #pragma endregion
               #pragma region M
                  { "fMinCurrentZoom", -0.2F },
                  { "fMouseWheelZoomIncrement", 0.075F },
                  { "fMouseWheelZoomMinDelta", 0.005F },
                  { "fMouseWheelZoomSpeed", 0.8F },
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
                  { sse_only, "fSlowVanityIncrement", 0.02F },
               #pragma endregion
               #pragma region T
                  { "fThumbstickZoomSpeed", 0.05F },
                  { "fTweenCamRotAngle", 0.05F },
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
               { "iAnimatedTransitionMillis", 1000 },
               { "iBleedoutTransitionMillis", 500 },
               { "iHorseTransitionMillis", 500 },
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
                  { "fCartPivotZ", 0.7F },
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
                  { "fWheelAngDamp", 0.01F },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               { "sHarnessBoneCart", "FrontHorseCarriage01" },
               { "sHarnessBoneLeft", "HarnessLeftBone" },
               { "sHarnessBoneRight", "HarnessRightBone" },
               { "sHorseConnect", "HorseSpine2" },
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
                  { "fHitEffectThresholdMod", 0.04F },
                  { "fHitEffectThresholdSevere", 0.043F },
                  { "fHitVectorDelay", 0.4F },
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
               { "iMaxHiPerfCombatCount", 4 },
               { "iShowHitVector", 0 },
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
                  { sse_only, "bGamepadLookApplyMaxedOutAcceleration", true },
                  { sse_only, "bGamepadLookApplySensitivityThreshold", false },
                  { sse_only, "bGamepadLookApplySnapToAxis", true },
                  { sse_only, "bGamepadLookTimeNormalizeInputs", true },
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
                  { "fControllerBufferDepth", 0.14F },
                  { "fControllerDampenTime", 0.18F },
                  { "fControllerSampleThreshold", 0.1F },
               #pragma endregion
               #pragma region D
                  { "fDialogueHardStopAngle1P", 45.0F },
                  { "fDialogueHardStopAngle3P", 55.0F },
                  { "fDialogueSoftStopAngle1P", 20.0F },
                  { "fDialogueSoftStopAngle3P", 25.0F },
                  { "fDirectionalDeadzone", 0.5F },
                  { "fDualCastChordTime", 0.05F },
               #pragma endregion
               #pragma region F
                  { "fFreezeDirectionDefaultAngleThreshold", 60.0F },
                  { "fFreezeDirectionDefaultSpeedThreshold", 100.0F },
               #pragma endregion
               #pragma region G
                  { sse_only, "fGamepadHeadingSensitivityDefault", 0.6667199730873108F },
                  { "fGamepadHeadingSensitivityMax", 3.55F },
                  { "fGamepadHeadingSensitivityMin", 0.25F },
                  { "fGamepadHeadingXScale", 0.9F },
                  { "fGamepadHeadingYScale", 23.0F },
                  { sse_only, "fGamepadLookAccelPitchMult", 2.5F },
                  { sse_only, "fGamepadLookAccelSec", 0.92F },
                  { sse_only, "fGamepadLookAccelYawMult", 2.5F },
                  { sse_only, "fGamepadLookMultExponent", 0.0F },
               #pragma endregion
               #pragma region H
                  { "fHeadingAxisDeadzone", 0.15F },
                  { "fHorseClampAngle", 10.0F },
                  { "fHorseControlsDampenTime", 1.0F },
                  { "fHorseHeadingMovementMult", 0.75F },
                  { "fHotKeyDelay", 0.25F },
               #pragma endregion
               #pragma region I
                  { "fInitialPowerAttackDelay", 0.3F },
                  { "fInitialPowerBashDelay", 0.3F },
               #pragma endregion
               #pragma region L
                  { "fLThumbDeadzone", 0.24F },
                  { sse_only, "fLThumbDeadzoneMax", 0.97F },
                  { sse_only, "fLookCurveSensitivityThreshold", 0.1F },
                  { sse_only, "fLookGraphCoefficient1", 0.11514099687337875F },
                  { sse_only, "fLookGraphCoefficient2", -0.3826659917831421F },
                  { sse_only, "fLookGraphCoefficient3", 2.0286500453948975F },
                  { sse_only, "fLookGraphCoefficient4", -0.7544990181922913F },
                  { "fLookGraphX1", 0.4F },
                  { "fLookGraphX2", 0.6F },
                  { "fLookGraphX3", 0.8F },
                  { "fLookGraphX4", 0.9F },
                  { "fLookGraphY1", 0.1F },
                  { "fLookGraphY2", 0.2F },
                  { "fLookGraphY3", 0.3F },
                  { "fLookGraphY4", 0.6F },
                  { sse_only, "fLookSnapToAxisStrength", 0.15F },
                  { sse_only, "fLookTimeNormalizingFloorFramerate", 20.0F },
                  { sse_only, "fLookTimeNormalizingTargetFramerate", 30.0F },
               #pragma endregion
               #pragma region M
                  { "fMaxLookRampUpDelta", 0.13F },
                  { "fMaxMoveRampDownDelta", 500.0F },
                  { "fMouseHeadingSensitivityMax", 0.05F },
                  { "fMouseHeadingSensitivityMin", 0.01F },
                  { "fMouseHeadingXScale", 0.02F },
                  { "fMouseHeadingYScale", 0.85F },
                  { "fMoveGraphX1", 0.2F },
                  { "fMoveGraphX2", 0.7F },
                  { "fMoveGraphX3", 0.9F },
                  { "fMoveGraphY1", 0.1F },
                  { "fMoveGraphY2", 0.5F },
                  { "fMoveGraphY3", 0.9F },
                  { "fMovementAxisDeadzone", 0.15F },
               #pragma endregion
               #pragma region O
                  { "fOutsideDialogueAngleRotationDampen", 0.33F },
               #pragma endregion
               #pragma region P
                  { "fPCDialogueLookSpeed", 10.0F },
                  { "fPCDialogueLookStart", 25.0F },
                  { "fPlayerThirdPersonDampenTime", 0.25F },
               #pragma endregion
               #pragma region R
                  { "fRThumbDeadzone", 0.265F },
                  { sse_only, "fRThumbDeadzoneMax", 0.97F },
                  { "fReverseDirThreshold", 0.3F },
               #pragma endregion
               #pragma region S
                  { "fSprintStopThreshold", 0.5F },
                  { "fSubsequentPowerAttackDelay", 2.0F },
                  { "fSubsequentPowerBashDelay", 2.0F },
               #pragma endregion
               #pragma region T
                  { "fTogglePOVDelay", 0.0F },
                  { "fTriggerDeadzone", 0.3F },
               #pragma endregion
               #pragma region Z
                  { "fZKeyDelay", 0.2F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               { "iNumHotkeys", 37 },
               { "iNumLookGraphSettings", 4 },
               { "iNumMoveGraphSettings", 3 },
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
            { sse_only, "bWriteTACOutputToFile", false },
         }),
         section_definition("Decals", {
            { "bAllowDecalsOnAlpha", true },
            { "bBackgroundInitializeGeometryDecals", true },
            { "bDecalMultithreaded", false },
            { "bDecalOcclusionQuery", true },
            { "bDecals", true },
            { "bForceAllDecals", false },
            { "bSkinnedDecals", true },
            { "fDebrisDecalTimer", 0.005F },
         }),
         section_definition("Dialogue", {
            { "fDialogueRotationPitchOffset", 0.17F },
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
                  { sse_only, "bAllowShaderCache", true },
                  { sse_only, "bAssertOnShaderCompileAtRuntime", true },
                  { "bAutoViewDistance", false },
               #pragma endregion
               #pragma region B
                  { sse_only, "bBreakOnValidationError", true },
                  { sse_only, "bBreakOnValidationWarning", false },
               #pragma endregion
               #pragma region C
                  { sse_only, "bCharacterLighting", true },
                  { sse_only, "bCompensateUnstableFrameTime", true },
                  { "bCompileOnRender", true },
                  { sse_only, "bCreateShadowRenderTarget", false },
               #pragma endregion
               #pragma region D
                  { sse_only, "bDOFApplyCenterWeight", true },
                  { sse_only, "bDOFBilateralBlur", true },
                  { sse_only, "bDeactivateAOOnSnow", true },
                  { "bDecalsOnSkinnedGeometry", true },
                  { sse_only, "bDirShadowMapFullViewPort", true },
                  { sse_only, "bDisableHighTreeShadow", true },
                  { sse_only, "bDisableShadowJumps", true },
                  { sse_only, "bDisableZPrepassOutput", false },
                  { "bDo30VFog", true },
                  { "bDoAmbientPass", true },
                  { "bDoDiffusePass", true },
                  { "bDoSpecularPass", true },
                  { "bDoTallGrassEffect", true },
                  { "bDoTestHDR", false },
                  { "bDoTexturePass", true },
                  { sse_only, "bDownSampleNormalSSR", true },
                  { sse_only, "bDynamicDOF", true },
                  { "bDynamicWindowReflections", true },
               #pragma endregion
               #pragma region E
                  { sse_only, "bEnableAutoDynamicResolution", false },
                  { sse_only, "bEnableDownsampleComputeShader", true },
                  { sse_only, "bEnableFrontToBackPrepass", false },
                  { sse_only, "bEnableLandFade", true },
                  { sse_only, "bEnableParallaxOcclusion", false },
                  { sse_only, "bEnableProjecteUVDiffuseNormalsOnCubemap", false },
                  { sse_only, "bEnableSnowMask", true },
                  { sse_only, "bEnableSnowRimLighting", true },
                  { sse_only, "bEnableStippleFade", true },
                  { sse_only, "bEnableVolumetricLighting", false },
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
                  { sse_only, "bIndDownscaled", false },
                  { sse_only, "bIndNormalMap", true },
               #pragma endregion
               #pragma region L
                  { "bLODNoiseAniso", true },
                  { "bLoadMarkers", true },
                  { sse_only, "bLockFramerate", true },
                  { sse_only, "bLodZPrepass", true },
                  { "bLowHealthIModEnabled", true },
               #pragma endregion
               #pragma region M
                  { "bMTRendering", false },
               #pragma endregion
               #pragma region O
                  { sse_only, "bOutputMissingTexture", true },
               #pragma endregion
               #pragma region P
                  { sse_only, "bProjectileOnReflection", false },
               #pragma endregion
               #pragma region R
                  { "bReportBadTangentSpace", false },
               #pragma endregion
               #pragma region S
                  { sse_only, "bSAOApplyFog", true },
                  { sse_only, "bSAODownscaled", false },
                  { sse_only, "bSAONormalMap", true },
                  { sse_only, "bSAO_CS_Downscaled", false },
                  { sse_only, "bShadowsOnGrass", true },
                  { "bShowMarkers", false },
                  { "bShowMenuTextureUse", true },
                  { "bSimpleLighting", false },
                  { sse_only, "bSparklesOnly", false },
                  { "bStaticMenuBackground", true },
               #pragma endregion
               #pragma region T
                  { sse_only, "bTAAWater", false },
               #pragma endregion
               #pragma region U
                  { "bUse Shaders", true },
                  { sse_only, "bUse16BitsDepthTarget", true },
                  { sse_only, "bUseDeviceDebug", false },
                  { "bUseFakeFullScreenMotionBlur", false },
                  { sse_only, "bUseFilmicCurve", false },
                  { sse_only, "bUseMultipleLuminanceReferences", true },
                  { sse_only, "bUsePrecomputedNoise", false },
                  { "bUseRefractionShader", true },
                  { "bUseSunbeams", false },
               #pragma endregion
               #pragma region V
                  { sse_only, "bValidateRenderTargets", true },
                  { sse_only, "bVolumetricLightingDisableInterior", true },
                  { sse_only, "bVolumetricLightingEnableTemporalAccumulation", true },
                  { sse_only, "bVolumetricLightingUpdateWeather", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region 1
                  { sse_only, "f1stPersonFarDepthRange", 0.01F },
                  { sse_only, "f1stPersonFarDepthRangeControlDriven", 0.1F },
               #pragma endregion
               #pragma region A
                  { sse_only, "fAlphaWeight", 1.0F },
               #pragma endregion
               #pragma region C
                  { sse_only, "fCharacterLightLumMax", 0.75F },
                  { sse_only, "fCharacterLightLumScale", 2.0F },
                  { sse_only, "fCharacterLightPrimaryLightIntensity", 0.75F },
                  { sse_only, "fCharacterLightSecondaryLightIntensity", 0.3F },
                  { sse_only, "fClampScale", 0.4F },
                  { sse_only, "fConstHDRAdaptTimerForMenu", 0.3F },
               #pragma endregion
               #pragma region D
                  { sse_only, "fDDOFAngleThreshold", 2.0F },
                  { sse_only, "fDDOFFocusCenterweightExt", 1.6F },
                  { sse_only, "fDDOFFocusCenterweightInt", 1.0F },
                  { sse_only, "fDDOFFocusDelay", 500.0F },
                  { sse_only, "fDDOFFocusDuration", 1000.0F },
                  { sse_only, "fDDOFPositionThreshold", 5.0F },
                  { sse_only, "fDOFCenterWeight", 0.6F },
                  { sse_only, "fDOFMaxDepthParticipation", 50000.0F },
                  { sse_only, "fDRClampOffset", 0.002F },
                  { sse_only, "fDRClampOffsetNeo", 0.0005F },
                  { "fDecalLOD0", 800.0F },
                  { "fDecalLifetime", 30.0F },
                  { sse_only, "fDecreaseDRMilliseconds", 32.0F },
                  { "fDefault1stPersonFOV", 65.0F },
                  { "fDefaultFOV", 65.0F },
                  { "fDefaultWorldFOV", 65.0F },
                  { sse_only, "fDynamicDOFBlurMultiplierMax", 1.0F },
                  { sse_only, "fDynamicDOFBlurMultiplierMin", 0.0F },
                  { sse_only, "fDynamicDOFFarBlur", 0.7F },
                  { sse_only, "fDynamicDOFFarDist", 1000.0F },
                  { sse_only, "fDynamicDOFFarRange", 10000.0F },
                  { sse_only, "fDynamicDOFNearBlur", 1.0F },
                  { sse_only, "fDynamicDOFNearDist", 100.0F },
                  { sse_only, "fDynamicDOFNearRange", 100.0F },
               #pragma endregion
               #pragma region E
                  { "fEnvMapLOD1", 1500.0F },
                  { "fEnvMapLOD2", 1800.0F },
                  { sse_only, "fExponentialShadowMapScale", 10.0F },
                  { "fEyeEnvMapLOD1", 500.0F },
                  { "fEyeEnvMapLOD2", 800.0F },
               #pragma endregion
               #pragma region F
                  { sse_only, "fFilmicWhiteScale", 1.4F },
                  { sse_only, "fFilteringAlphaThreshold", 0.01F },
                  { sse_only, "fFilteringWaterDepthThreshold", 0.005F },
                  { sse_only, "fFirstSliceDistance", 1250.0F },
               #pragma endregion
               #pragma region G
                  { "fGammaMax", 0.6F },
                  { "fGammaMin", 1.4F },
                  { sse_only, "fGlobalBloomThresholdBoost", 0.0F },
                  { sse_only, "fGlobalBrightnessBoost", 0.0F },
                  { sse_only, "fGlobalContrastBoost", 0.0F },
                  { sse_only, "fGlobalEyeAdaptSpeedScale", 2.0F },
                  { sse_only, "fGlobalEyeAdaptStrengthScale", 0.4F },
                  { sse_only, "fGlobalMapBloomThresholdBoost", 0.25F },
                  { sse_only, "fGlobalMapBrightnessBoost", 0.25F },
                  { sse_only, "fGlobalMapContrastBoost", -0.3F },
                  { sse_only, "fGlobalSaturationBoost", 0.0F },
               #pragma endregion
               #pragma region I
                  { sse_only, "fIBLFAnamorphicsIntensity", 0.1F },
                  { sse_only, "fIBLFAnamorphicsIntensityFar", 1.0F },
                  { sse_only, "fIBLFBloomIntensity", 0.0F },
                  { sse_only, "fIBLFChannelsDistortionBlue", -2.0F },
                  { sse_only, "fIBLFChannelsDistortionGreen", 0.0F },
                  { sse_only, "fIBLFChannelsDistortionRed", 2.0F },
                  { sse_only, "fIBLFFlaresDispersal", 0.3F },
                  { sse_only, "fIBLFGlobalIntensity", 0.7F },
                  { sse_only, "fIBLFHaloFetch", 0.5F },
                  { sse_only, "fIBLFHaloWidthPow", 3.0F },
                  { sse_only, "fIBLFLightsBurn", 1.0F },
                  { sse_only, "fIBLFLightsRangeDownshift", 1.0F },
                  { sse_only, "fIncreaseDRMilliseconds", 30.0F },
                  { sse_only, "fIndBias", 2.5F },
                  { sse_only, "fIndIntensity", 50.0F },
                  { sse_only, "fIndRadius", 100.0F },
               #pragma endregion
               #pragma region L
                  { "fLODNoiseMipBias", 0.0F },
                  { "fLandLOFadeSeconds", 15.0F },
                  { "fLightLODDefaultStartFade", 1000.0F },
                  { "fLightLODMaxStartFade", 1200.0F },
                  { "fLightLODMinStartFade", 200.0F },
                  { "fLightLODRange", 500.0F },
                  { "fLinePrimitiveWidth", 8.0F },
                  { sse_only, "fLoadingMenuShadowBias", 30.64F },
                  { sse_only, "fLoadingMenuShadowFallOff", 0.0F },
                  { "fLowHealthIModInterval", 2.0F },
                  { "fLowHealthIModStrengthMax", 1.5F },
                  { "fLowHealthIModStrengthMin", 0.8F },
                  { sse_only, "fLowestDynamicHeightRatio", 1.0F },
                  { sse_only, "fLowestDynamicWidthRatio", 0.7F },
               #pragma endregion
               #pragma region M
                  { sse_only, "fMaxFocusShadowMapDistance", 450.0F },
                  { sse_only, "fMaxHeightShadowCastingTrees", 5000.0F },
                  { "fMeshLODFadeTime", 1.0F },
                  { "fMipBias", 0.0F },
               #pragma endregion
               #pragma region N
                  { "fNear1stPersonDistance", 5.0F },
                  { "fNearDistance", 15.0F },
                  { "fNoLODFarDistanceMax", 10240.0F },
                  { "fNoLODFarDistanceMin", 100.0F },
                  { "fNoLODFarDistancePct", 1.0F },
                  { sse_only, "fNonSpecularSparklesIntensity", 0.4F },
               #pragma endregion
               #pragma region P
                  { sse_only, "fPoissonRadiusScale", 4.0F },
               #pragma endregion
               #pragma region R
                  { sse_only, "fRatioDecreasePerSeconds", 0.07F },
                  { sse_only, "fRatioIncreasePerSeconds", 0.03F },
                  { sse_only, "fReflectionMarchingRadius", 0.4F },
                  { sse_only, "fReflectionRayThickness", 0.001F },
                  { sse_only, "fReflectionsIntensityScale", 0.8F },
                  { sse_only, "fReflectionsMotionVectorScale", 1.0F },
                  { sse_only, "fReinhardWhiteScale", 1.1F },
               #pragma endregion
               #pragma region S
                  { sse_only, "fSAOBias", 2.5F },
                  { sse_only, "fSAOExpFactor", 0.11F },
                  { sse_only, "fSAOIntensity", 15.0F },
                  { sse_only, "fSAORadius", 250.0F },
                  { sse_only, "fSAOValueDiffFactor", 0.3F },
                  { sse_only, "fSAO_CS_Bias", 2.5F },
                  { sse_only, "fSAO_CS_Intensity", 10.0F },
                  { sse_only, "fSAO_CS_Radius", 250.0F },
                  { "fScopeScissorAmount", 0.3F },
                  { sse_only, "fShadowBiasScale", 1.0F },
                  { sse_only, "fShadowClampValue", 0.3F },
                  { sse_only, "fShadowDirectionalBiasScale", 0.3F },
                  { "fShadowFadeTime", 1.0F },
                  { "fShadowLODDefaultStartFade", 200.0F },
                  { "fShadowLODMaxStartFade", 300.0F },
                  { "fShadowLODMinStartFade", 100.0F },
                  { "fShadowLODRange", 200.0F },
                  { sse_only, "fShadowSparkleIntensity", 0.25F },
                  { "fSkinnedDecalLOD0", 300.0F },
                  { "fSkinnedDecalLOD1", 500.0F },
                  { "fSkinnedDecalLOD2", 800.0F },
                  { sse_only, "fSnowGeometrySpecPower", 3.0F },
                  { sse_only, "fSnowNormalSpecPower", 2.0F },
                  { sse_only, "fSnowRimLightIntensity", 0.3F },
                  { sse_only, "fSnowSSSColorB", 0.08F },
                  { sse_only, "fSnowSSSColorG", 0.0F },
                  { sse_only, "fSnowSSSColorR", 0.0F },
                  { sse_only, "fSnowSSSDepthDiff", 0.5F },
                  { sse_only, "fSnowSSSStrength", 50.0F },
                  { sse_only, "fSnowSparklesColorB", 1.0F },
                  { sse_only, "fSnowSparklesColorG", 1.0F },
                  { sse_only, "fSnowSparklesColorR", 1.0F },
                  { sse_only, "fSparklesDensity", 0.85F },
                  { sse_only, "fSparklesIntensity", 1.0F },
                  { sse_only, "fSparklesMaxDistance", 1000.0F },
                  { sse_only, "fSparklesSize", 6.0F },
                  { sse_only, "fSparklesSpecularPower", 2.0F },
                  { sse_only, "fSpecMaskBegin", 0.1F },
                  { sse_only, "fSpecMaskSpan", 0.0F },
                  { "fSpecularLODDefaultStartFade", 500.0F },
                  { "fSpecularLODMaxStartFade", 600.0F },
                  { "fSpecularLODMinStartFade", 200.0F },
                  { "fSpecularLODRange", 300.0F },
                  { sse_only, "fSpecularSparklesIntensity", 1.0F },
                  { sse_only, "fSplitOverlap", 100.0F },
                  { "fSunShadowUpdateTime", 1.0F },
                  { sse_only, "fSunStaticTimeUpdateScale", 0.1F },
                  { "fSunUpdateThreshold", 0.5F },
               #pragma endregion
               #pragma region T
                  { sse_only, "fTAAEffectThreshold", 0.1F },
                  { sse_only, "fTAAHighFreq", 0.8F },
                  { sse_only, "fTAALowFreq", 0.5F },
                  { sse_only, "fTAAPostOverlay", 0.21F },
                  { sse_only, "fTAAPostSharpen", 0.21F },
                  { sse_only, "fTAASharpen", 1.0F },
               #pragma endregion
               #pragma region V
                  { sse_only, "fVolumetricLightingCustomColorContribution", 0.0F },
                  { sse_only, "fVolumetricLightingDensityContribution", 0.3F },
                  { sse_only, "fVolumetricLightingDensityScale", 300.0F },
                  { sse_only, "fVolumetricLightingIntensity", 2.0F },
                  { sse_only, "fVolumetricLightingPhaseContribution", 0.83F },
                  { sse_only, "fVolumetricLightingPhaseScattering", 0.85F },
                  { sse_only, "fVolumetricLightingRangeFactor", 40.0F },
                  { sse_only, "fVolumetricLightingTemporalAccumulationFactor", 0.75F },
                  { sse_only, "fVolumetricLightingWindFallingSpeed", 0.3F },
                  { sse_only, "fVolumetricLightingWindSpeedScale", 15.0F },
               #pragma endregion
               #pragma region W
                  { sse_only, "fWaterSSRBlurAmount", 0.3F },
                  { sse_only, "fWaterSSRIntensity", 1.3F },
                  { sse_only, "fWaterSSRNormalPerturbationScale", 0.05F },
                  { sse_only, "fWindGrassMultiplier", 1.0F },
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
                  { sse_only, "iEnableShadowCastingFlag", 2 },
               #pragma endregion
               #pragma region L
                  { sse_only, "iLandscapeMultiNormalTilingFactor", 4 },
                  { sse_only, "iLoadingMenuShadowLightFlags", 1 },
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
                  { sse_only, "iSnowSSSCurrentColor", 2 },
                  { sse_only, "iSnowSparklesColor", 2 },
               #pragma endregion
               #pragma region T
                  { "iTrilinearThreshold", 3 },
               #pragma endregion
               #pragma region U
                  { sse_only, "iUnstableFrameTimeHistorySize", 8 },
               #pragma endregion
               #pragma region V
                  { sse_only, "iVolumetricLightingNoiseTextureDepth", 32 },
                  { sse_only, "iVolumetricLightingNoiseTextureHeight", 32 },
                  { sse_only, "iVolumetricLightingNoiseTextureWidth", 32 },
                  { sse_only, "iVolumetricLightingTextureDepthHigh", 90 },
                  { sse_only, "iVolumetricLightingTextureDepthLow", 50 },
                  { sse_only, "iVolumetricLightingTextureDepthMedium", 70 },
                  { sse_only, "iVolumetricLightingTextureFormatHigh", 1 },
                  { sse_only, "iVolumetricLightingTextureFormatLow", 0 },
                  { sse_only, "iVolumetricLightingTextureFormatMedium", 1 },
                  { sse_only, "iVolumetricLightingTextureHeightHigh", 192 },
                  { sse_only, "iVolumetricLightingTextureHeightLow", 96 },
                  { sse_only, "iVolumetricLightingTextureHeightMedium", 128 },
                  { sse_only, "iVolumetricLightingTextureWidthHigh", 320 },
                  { sse_only, "iVolumetricLightingTextureWidthLow", 160 },
                  { sse_only, "iVolumetricLightingTextureWidthMedium", 224 },
               #pragma endregion
            #pragma endregion
            #pragma region Strings
               { "sDebugText", "VATS" },
               { "sScreenShotBaseName", "ScreenShot" },
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
               { "bFootPlacementOn", true },
               { "bRigidBodyController", true },
            #pragma endregion
            #pragma region Floats
               #pragma region A
                  { "fAnkleOffset", 0.2F },
               #pragma endregion
               #pragma region C
                  { "fControllerTetherLen", 6.0F },
               #pragma endregion
               #pragma region F
                  { "fFootPlantedGain", 1.0F },
                  { "fFootRaisedGain", 0.9F },
               #pragma endregion
               #pragma region G
                  { "fGroundAscendingGain", 0.4F },
                  { "fGroundDescendingGain", 0.4F },
               #pragma endregion
               #pragma region M
                  { "fMaxFootCastMilliSec", 0.6F },
                  { "fMaxStepVertError", 3.5F },
               #pragma endregion
               #pragma region O
                  { "fOnOffGain", 0.5F },
                  { "fOriginalGroundHeightMS", -0.11F },
               #pragma endregion
               #pragma region P
                  { "fPelvisOffsetDamping", 0.2F },
                  { "fPelvisUpDownBias", 0.75F },
               #pragma endregion
               #pragma region R
                  { "fRagdollFeedback", 0.7F },
               #pragma endregion
               #pragma region V
                  { "fVertErrorGain", 0.5F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               { "iNumFramesFootEaseOut", 30 },
            #pragma endregion
         }),
         section_definition("Gameplay", {
            #pragma region Booleans
               { "bAllowDragonFlightLocationDiscovery", false },
               { "bAllowHavokGrabTheLiving", false },
               { "bEssentialTakeNoDamage", true },
               { "bHealthBarShowing", false },
               { "bInstantLevelUp", false },
               { "bTargetLockIsToggle", true },
               { "bTrackProgress", false },
            #pragma endregion
            #pragma region Floats
               { "fFootIKDistance", 1024.0F },
               { "fMagicTargetLocationExtraLargeActorRadius", 256.0F },
               { "fMagicTargetLocationNormalActorRadius", 32.0F },
               { "fMapMarkerUpdateTime", 0.05F },
               { "fPlayerHealthSaveOnPauseLimit", 0.25F },
               { "fPlayerSunGazeDelta", 0.9848080277442932F },
               { "fPlayerSunGazeStartTimer", 0.5F },
               { "fTargetLockXYRange", 7500.0F },
            #pragma endregion
            #pragma region Integers
               { "iActorsDismemberedPerFrame", 2 },
               { "iDetectionPicks", 21 },
            #pragma endregion
            #pragma region Strings
               { "sTrackProgressPath", "\\\\vault2\\Fallout\\LevelData\\" },
            #pragma endregion
         }),
         section_definition("General", {
            #pragma region Booleans
               #pragma region A
                  { sse_only, "bActivateFromSave", false },
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
                  { sse_only, "bCullingJobEnablePlaneOptimization", false },
               #pragma endregion
               #pragma region D
                  { "bDebugSpectatorThreats", false },
                  { "bDefaultCOCPlacement", false },
                  { "bDirectionalMaterial", true },
                  { "bDisableAllGore", false },
                  { "bDisableDuplicateReferenceCheck", true },
                  { "bDisableGearedUp", true },
                  { sse_only, "bDisableWarningWindows", false },
                  { "bDisplayBoundingVolumes", false },
               #pragma endregion
               #pragma region E
                  { "bEnableBoundingVolumeOcclusion", true },
                  { "bEnableFileCaching", false },
                  { sse_only, "bEnableFriendHenchman", false },
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
                  { sse_only, "bJoblistActiveWait", true },
               #pragma endregion
               #pragma region K
                  { "bKeepDLStringBlocksLoaded", false },
                  { "bKeepILStringBlocksLoaded", true },
                  { "bKeepPluginWhenMerging", false },
               #pragma endregion
               #pragma region M
                  { sse_only, "bModManagerMenuEnabled", true },
                  { "bMultiThreadMovement", true },
               #pragma endregion
               #pragma region P
                  { "bParallelAnimUpdate", false },
                  { sse_only, "bPauseWhenConstrained", true },
                  { "bPreCullActors", true },
                  { "bPreemptivelyUnloadCells", false },
                  { "bPreloadIntroSequence", true },
                  { sse_only, "bPreloadLinkedInteriors", false },
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
                  { "fNormalDoorFadeSecs", 0.4F },
                  { "fNormalDoorFadeWait", 0.01F },
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
                  { sse_only, "iMaxJobThreads", 32 },
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
                  { sse_only, "sGamerIconTextureName", "BGSUserIcon" },
               #pragma endregion
               #pragma region I
                  { "sIntroMovie", "Not enough arguments..." },
                  { "sIntroSequence", "BGS_LOGO.BIK" },
               #pragma endregion
               #pragma region L
                  { "sLanguage", "ENGLISH" },
                  { sse_only, "sLocalCharacterDataPath", "Saves\\Character\\" },
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
                  { sse_only, "strPluginsFileHeader", "# This file is used by Skyrim to keep track of your downloaded content." },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("GeneralWarnings", {
            { "sGeneralMasterMismatchWarning", "One or more plugins could not find the correct versions of the master files they depend on. Errors may occur during load or game play. Check the \"Warnings.txt\" file for more information." },
            { "sMasterMismatchWarning", "One of the files that \"%s\" is dependent on has changed since the last save." },
         }),
         section_definition("GethitShader", {
            { "fBlockedTexOffset", 0.001F },
            { "fBlurAmmount", 0.5F },
            { "fHitTexOffset", 0.005F },
         }),
         section_definition("GrabIK", {
            { "fDriveGain", 0.25F },
         }),
         section_definition("Grass", {
            #pragma region Booleans
               { "bAllowCreateGrass", false },
               { "bAllowLoadGrass", true },
               { "bDrawShaderGrass", true },
               { sse_only, "bEnableGrassFade", true },
               { "bGenerateGrassDataFiles", false },
               { "bGrassPointLighting", false },
            #pragma endregion
            #pragma region Floats
               { "fGrassDefaultStartFadeDistance", 3500.0F },
               { sse_only, "fGrassFadeInTime", 1.8F },
               { "fGrassFadeRange", 1000.0F },
               { "fGrassWindMagnitudeMax", 125.0F },
               { "fGrassWindMagnitudeMin", 5.0F },
               { "fTexturePctThreshold", 0.0F },
               { "fWaveOffsetRange", 1.75F },
            #pragma endregion
            #pragma region Integers
               { "iGrassCellRadius", 2 },
               { "iMaxGrassTypesPerTexure", 2 },
               { "iMinGrassSize", 20 },
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
                  { "fGoodPosCastCheckDepth", 0.1F },
                  { "fGoodPosCheckDepth", 0.1F },
               #pragma endregion
               #pragma region I
                  { "fInAirFallingCharGravityMult", 1.35F },
               #pragma endregion
               #pragma region J
                  { "fJumpAnimDelay", 0.75F },
               #pragma endregion
               #pragma region M
                  { "fMaxPickTime", 0.003F },
                  { "fMaxPickTimeDebug", 0.06F },
                  { "fMaxPickTimeDebugVATS", 0.6F },
                  { "fMaxPickTimeVATS", 0.03F },
                  { "fMaxTime", 0.01666666666666666F },
                  { "fMaxTimeComplex", 0.03333333333333333F },
                  { "fMoveLimitMass", 95.0F },
               #pragma endregion
               #pragma region O
                  { "fOD", 0.9F },
               #pragma endregion
               #pragma region Q
                  { "fQuadrupedPitchMult", 10.0F },
               #pragma endregion
               #pragma region R
                  { "fRF", 1000.0F },
               #pragma endregion
               #pragma region S
                  { "fSD", 0.98F },
                  { "fSE", 0.3F },
               #pragma endregion
               #pragma region T
                  { "fTimePerSubStep", 0.008F },
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
               { "iEntityBatchRemoveRate", 100 },
               { "iMinNumSubSteps", 8 },
               { "iNumThreads", 1 },
               { "iSimType", 1 },
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
            { sse_only, "fLensFlareFalloffRange", 256.0F },
            { sse_only, "fLensFlareGlobalIntensity", 1.0F },
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
                  { "fBookLight2DiffuseColorB", 0.83F },
                  { "fBookLight2DiffuseColorG", 0.95F },
                  { "fBookLight2DiffuseColorR", 0.98F },
                  { "fBookLight2DimmerValue", 1.0F },
                  { "fBookLight2Radius", 400.0F },
                  { "fBookLight2X", 10.0F },
                  { "fBookLight2Y", -75.0F },
                  { "fBookLight2Z", 10.0F },
                  { "fBookLightDiffuseColorB", 0.83F },
                  { "fBookLightDiffuseColorG", 0.95F },
                  { "fBookLightDiffuseColorR", 0.98F },
                  { "fBookLightDimmerValue", 1.75F },
                  { "fBookLightRadius", 400.0F },
                  { "fBookLightX", 100.0F },
                  { "fBookLightY", -350.0F },
                  { "fBookLightZ", 100.0F },
                  { "fBookOpenTime", 1000.0F },
                  { "fBookPosHeightPercentage", 0.445F },
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
                  { "fCrafting3DItemScale", 1.87F },
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
                  { "fInventory3DItemPosScale", 1.87F },
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
                  { "fInventoryLight2DiffuseColorB", 0.83F },
                  { "fInventoryLight2DiffuseColorG", 0.95F },
                  { "fInventoryLight2DiffuseColorR", 0.98F },
                  { "fInventoryLight2DimmerValue", 1.75F },
                  { "fInventoryLight2Radius", 0.0F },
                  { "fInventoryLightDiffuseColorB", 0.83F },
                  { "fInventoryLightDiffuseColorG", 0.95F },
                  { "fInventoryLightDiffuseColorR", 0.98F },
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
                  { "fJournalLongRepeatRate", 0.2F },
                  { "fJournalShortRepeatRate", 0.075F },
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
                  { "fLockpickLightDiffuseColorB", 0.83F },
                  { "fLockpickLightDiffuseColorG", 0.95F },
                  { "fLockpickLightDiffuseColorR", 0.98F },
                  { "fLockpickLightDimmerValue", 1.75F },
                  { "fLockpickLightRadius", 400.0F },
                  { "fLockpickLightX", 100.0F },
                  { "fLockpickLightY", -1000.0F },
                  { "fLockpickLightZ", 100.0F },
               #pragma endregion
               #pragma region M
                  { "fMagic3DItemPosScale", 1.87F },
                  { "fMagic3DItemPosScaleWide", 1.75F },
                  { "fMagic3DItemPosX", 29.0F },
                  { "fMagic3DItemPosXWide", 22.0F },
                  { "fMagic3DItemPosY", -500.0F },
                  { "fMagic3DItemPosYWide", -500.0F },
                  { "fMagic3DItemPosZ", 8.0F },
                  { "fMagic3DItemPosZWide", 6.0F },
                  { "fMaxSubtitleDistance", 1250.0F },
                  { "fMenuKeyRepeatLong", 0.5F },
                  { "fMenuKeyRepeatShort", 0.1F },
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
                  { "fRSMCameraLookAtPercent", 0.955F },
                  { "fRSMLookAtOnGain", 0.06F },
               #pragma endregion
               #pragma region S
                  { "fSafeZoneX", 15.0F },
                  { "fSafeZoneXWide", 15.0F },
                  { "fSafeZoneY", 15.0F },
                  { "fSafeZoneYWide", 15.0F },
                  { "fSleepFaderTime", 0.7F },
               #pragma endregion
               #pragma region T
                  { "fTweenLongRepeatRate", 0.2F },
                  { "fTweenShortRepeatRate", 0.1F },
               #pragma endregion
               #pragma region U
                  { "fUIAltLogoModel_TranslateX_G", 0.0F },
                  { "fUIAltLogoModel_TranslateY_G", 0.0F },
                  { "fUIAltLogoModel_TranslateZ_G", 0.0F },
                  { "fUICameraFarDistance", 20480.0F },
                  { "fUICameraNearDistance", 15.0F },
                  { "fUILogoModel_AutoRotateSpeed", 0.1F },
                  { "fUILogoModel_FadeSecs", 0.0001F },
                  { "fUILogoModel_MouseThreshold", 2.0F },
                  { "fUILogoModel_MouseToPanSpeed", 1.0F },
                  { "fUILogoModel_MouseToRotateSpeed", 0.02F },
                  { "fUILogoModel_MouseToZoomSpeed", 0.6F },
                  { "fUILogoModel_RotationPauseDuration", 0.25F },
                  { "fUILogoModel_ThumbstickToPanSpeed", 8.0F },
                  { "fUILogoModel_ThumbstickToRotateSpeed", 0.45F },
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
                  { "fUIMistMenu_LogoOnscreenPanThresholdY", 0.33F },
                  { "fUIMistMenu_LogoOnscreenZoomMaxFOV", 95.0F },
                  { "fUIMistMenu_LogoOnscreenZoomMinFOV", 60.0F },
                  { "fUIMistMenu_LogoOnscreenZoomThresholdFar", 0.1F },
                  { "fUIMistMenu_LogoOnscreenZoomThresholdNear", 3.5F },
                  { "fUIMistModel_FadeOutTime", 0.0F },
                  { "fUIMistModel_RotateZ_G", -180.0F },
                  { "fUIMistModel_TranslateX_G", 0.0F },
                  { "fUIMistModel_TranslateY_G", 0.0F },
                  { "fUIMistModel_TranslateZ_G", 0.0F },
                  { "fUIPlayerSceneLight2DiffuseColorB", 0.8F },
                  { "fUIPlayerSceneLight2DiffuseColorG", 0.81F },
                  { "fUIPlayerSceneLight2DiffuseColorR", 0.7F },
                  { "fUIPlayerSceneLight2DimmerValue", 3.0F },
                  { "fUIPlayerSceneLight2Radius", 1024.0F },
                  { "fUIPlayerSceneLight2X", 160.0F },
                  { "fUIPlayerSceneLight2Y", -96.0F },
                  { "fUIPlayerSceneLight2Z", 160.0F },
                  { "fUIPlayerSceneLight3DiffuseColorB", 1.0F },
                  { "fUIPlayerSceneLight3DiffuseColorG", 1.0F },
                  { "fUIPlayerSceneLight3DiffuseColorR", 1.0F },
                  { "fUIPlayerSceneLight3DimmerValue", 0.1F },
                  { "fUIPlayerSceneLight3Radius", 1024.0F },
                  { "fUIPlayerSceneLight3X", 128.0F },
                  { "fUIPlayerSceneLight3Y", 160.0F },
                  { "fUIPlayerSceneLight3Z", -96.0F },
                  { "fUIPlayerSceneLightDiffuseColorB", 0.82F },
                  { "fUIPlayerSceneLightDiffuseColorG", 0.96F },
                  { "fUIPlayerSceneLightDiffuseColorR", 0.96F },
                  { "fUIPlayerSceneLightDimmerValue", 1.6F },
                  { "fUIPlayerSceneLightRadius", 1500.0F },
                  { "fUIPlayerSceneLightX", -160.0F },
                  { "fUIPlayerSceneLightY", 160.0F },
                  { "fUIPlayerSceneLightZ", 128.0F },
                  { "fUIRaceSexLight2DiffuseColorB", 0.83F },
                  { "fUIRaceSexLight2DiffuseColorG", 0.95F },
                  { "fUIRaceSexLight2DiffuseColorR", 0.98F },
                  { "fUIRaceSexLight2DimmerValue", 1.75F },
                  { "fUIRaceSexLight2Radius", 1400.0F },
                  { "fUIRaceSexLight2X", 0.5F },
                  { "fUIRaceSexLight2Y", -150.0F },
                  { "fUIRaceSexLight2Z", 60.5F },
                  { "fUIRaceSexLightDiffuseColorB", 0.83F },
                  { "fUIRaceSexLightDiffuseColorG", 0.95F },
                  { "fUIRaceSexLightDiffuseColorR", 0.98F },
                  { "fUIRaceSexLightDimmerValue", 0.65F },
                  { "fUIRaceSexLightRadius", 1400.0F },
                  { "fUIRaceSexLightX", 0.5F },
                  { "fUIRaceSexLightY", -600.0F },
                  { "fUIRaceSexLightZ", 60.5F },
                  { "fUnlockDoorDelay", 1.5F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               { "iFavoriteItemQueueSize", 100 },
               { "iMaxViewCasterPicksFuzzy", 5 },
               { "iMaxViewCasterPicksGamebryo", 10 },
               { "iMaxViewCasterPicksHavok", 10 },
               { "iSubtitleSpeakerNameColor", 8947848 },
            #pragma endregion
            #pragma region Strings
               { "sCreditsFile", "Interface/Credits.txt" },
               { "sCreditsFileFrench", "Interface/Credits_French.txt" },
               { sse_only, "sCreditsFilePLRU", "Interface/Credits_PLRU.txt" },
               { "sForcedLoadScreenEditorID", "Not enough arguments..." },
               { "sPosePlayerRaceSexMenu", "OffsetBoundStandingPlayerInstant" },
               { "sUIMistMenu_DefaultLogoCameraPath", "Not enough arguments..." },
            #pragma endregion
         }),
         section_definition("Kinect", {
            { "fKinectMaxAllyTradeDistance", 60000.0F },
            { "fKinectMinConfidence", 0.3F },
            { "fKinectMinReportConfidence", 0.01F },
            { "fKinectMinReportShoutConfidence", 0.005F },
            { "fKinectMinRuleConfidence", 0.25F },
            { "fKinectMinShoutConfidence", 0.0075F },
         }),
         section_definition("Landscape", {
            { "bCurrentCellOnly", false },
            { sse_only, "bLandSpecular", true },
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
                  { sse_only, "sGamepadDisconnectedMessage", "Please connect a controller to continue." },
                  { sse_only, "sGamepadDisconnectedTitle", "Controller disconnected." },
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
            { "fDecalLODFadeEnd", 0.06F },
            { "fDecalLODFadeStart", 0.05F },
            { "fEnvmapLODFadeEnd", 0.1F },
            { "fEnvmapLODFadeStart", 0.09F },
            { "fEyeEnvmapLODEnd", 0.05F },
            { "fRefractionLODFadeEnd", 0.03F },
            { "fRefractionLODFadeStart", 0.025F },
            { "fSpecularLODFadeEnd", 0.1F },
            { "fSpecularLODFadeStart", 0.09F },
         }),
         section_definition("LOD", {
            #pragma region Booleans
               { "bDisplayLODLand", true },
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
                  { "fFadeInThreshold", 0.7F },
                  { "fFadeInTime", 1.2F },
                  { "fFadeOutThreshold", 0.3F },
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
                  { "fLODFadeOutPercent", 0.6F },
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
               { "iFadeNodeMinNearDistance", 500 },
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
               { "bWorldMapNoSkyDepthBlur", false },
            #pragma endregion
            #pragma region Floats
               #pragma region G
                  { "fGamepadCursorSensitivity", 2.0F },
               #pragma endregion
               #pragma region M
                  { "fMapLocalCursorPanSpeed", 2000.0F },
                  { "fMapLocalGamepadPanSpeed", 100.0F },
                  { "fMapLocalGamepadZoomSpeed", 0.03F },
                  { "fMapLocalHeight", 40000.0F },
                  { "fMapLocalMarkerSelectionDist", 0.03F },
                  { "fMapLocalMinFrustumWidth", 4000.0F },
                  { "fMapLocalMousePanSpeed", 20.0F },
                  { "fMapLocalMouseZoomSpeed", 0.1F },
                  { "fMapLookGamepadSpeed", 1.5F },
                  { "fMapLookMouseSpeed", 3.0F },
                  { "fMapLoopFadeTimeSeconds", 1.0F },
                  { "fMapMenuNearClipPlane", 128.0F },
                  { "fMapMenuOverlayNormalSnowStrength", 0.4F },
                  { "fMapMenuOverlayNormalStrength", 1.1F },
                  { "fMapMenuOverlayScale", 0.000035F },
                  { "fMapMenuOverlaySnowScale", 0.000045F },
                  { "fMapMoveKeyboardSpeed", 0.02F },
                  { "fMapTransitionSpeed", 0.75F },
                  { "fMapWorldCursorMoveArea", 0.9F },
                  { "fMapWorldHeightAdjustmentForce", 4.0F },
                  { "fMapWorldMaxPanSpeed", 75000.0F },
                  { "fMapWorldMaxPitch", 75.0F },
                  { "fMapWorldMinPanSpeed", 60000.0F },
                  { "fMapWorldMinPitch", 15.0F },
                  { "fMapWorldTransitionHeight", 10000.0F },
                  { "fMapWorldYawRange", 80.0F },
                  { "fMapWorldZoomSpeed", 2.0F },
                  { "fMapZoomMouseSpeed", 2.0F },
                  { "fMaxMarkerSelectionDist", 0.003F },
               #pragma endregion
               #pragma region W
                  { "fWorldMapDepthBlurScale", 0.3F },
                  { "fWorldMapFocalDepth", 45000.0F },
                  { "fWorldMapMaximumDepthBlur", 0.45F },
                  { "fWorldMapNearDepthBlurScale", 4.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               { "iRightStickRepeatRate", 250 },
            #pragma endregion
            #pragma region Strings
               { "sMapWorldDefaultWorldSpace", "Tamriel" },
               { "sWorldMapOverlayNormalSnowTexture", "Data\\Textures\\Terrain\\WorldMapOverlaySnow_n.dds" },
               { "sWorldMapOverlayNormalTexture", "Data\\Textures\\Terrain\\WorldMapOverlay_n.dds" },
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
            { sse_only, "bAssertsWithoutDebugger", false },
            { "bBlockMessageBoxes", false },
            { "bDisableAssertQueuing", true },
            { "bFaceGenWarnings", false },
            { sse_only, "bNoBreaksForAsserts", false },
            { "bShowMissingAudioWarnings", true },
            { "bShowMissingLipWarnings", true },
            { "bSkipInitializationFlows", true },
            { "bSkipProgramFlows", true },
            { "bUseWindowsMessageBox", false },
            { "iFileLogging", 0 },
            { sse_only, "sDisabledProgramFlowContexts", "Not enough arguments..." },
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
            { "fExtraTaskletBudgetMS", 1.2F },
            { "fPostLoadUpdateTimeMS", 2000.0F },
            { "fUpdateBudgetMS", 1.2F },
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
                  { "fAcceptableErrorRatio", 0.9F },
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
                  { "fFindMaxSpeedMinParamIncrementPercent", 0.1F },
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
                  { "fMinTimeToNextPoint", 0.3F },
                  { "fMinimalUsePathingCost", 409600.0F },
                  { "fMovementBlockedTimer", 0.02F },
               #pragma endregion
               #pragma region N
                  { "fNavmeshBoundsActorRadiusMultiplier", 1.0F },
                  { "fNavmeshBoundsMinTimeOfImpact", 0.0333F },
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
                  { "fPreferredTriangleMultiplier", 0.01F },
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
                  { "fTotalDisplacementThresholdRadiusMult", 0.66F },
                  { "fTotalTimePadding", 0.5F },
                  { "fTotalTimeThreshold", 1.0F },
                  { "fTweenerAnimDurationOffset", 0.1F },
                  { "fTweeningMaxPercentSpeedDelta", 0.2F },
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
               { "bFootIK", true },
               { "bGrabIK", true },
               { "bLookIK", true },
               { "bPoseMatching", true },
               { "bRagdollAnim", true },
               { "bRagdollFeedback", true },
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
                  { "fFeedbackOnOffGain", 0.3F },
                  { "fFeedbackOnOffGainTimeMS", 1000.0F },
                  { "fFeedbackTimeMS", 10000.0F },
               #pragma endregion
               #pragma region H
                  { "fHierarchyGain", 0.17F },
               #pragma endregion
               #pragma region I
                  { "fImpulseLimit", 15.0F },
               #pragma endregion
               #pragma region P
                  { "fPositionGain", 0.05F },
                  { "fPositionMaxAngularVelocity", 18.0F },
                  { "fPositionMaxLinearVelocity", 14.0F },
               #pragma endregion
               #pragma region S
                  { "fSnapGain", 0.1F },
                  { "fSnapMaxAngularDistance", 1.0F },
                  { "fSnapMaxAngularVelocity", 0.3F },
                  { "fSnapMaxLinearDistance", 0.3F },
                  { "fSnapMaxLinearVelocity", 3.0F },
               #pragma endregion
               #pragma region V
                  { "fVelocityDamping", 0.0F },
                  { "fVelocityGain", 0.6F },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("SaveGame", {
            #pragma region Booleans
               #pragma region A
                  { "bAllowProfileTransfer", false },
                  { "bAllowScriptedAutosave", true },
                  { "bAllowScriptedForceSave", true },
                  { sse_only, "bAutoSaveOnUserStale", true },
               #pragma endregion
               #pragma region C
                  { sse_only, "bCompressBuffer", false },
                  { sse_only, "bConvertNonUtilitySaves", false },
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
               { "iAutoSaveCount", 3 },
               { "iSaveGameBackupCount", 1 },
            #pragma endregion
            #pragma region Strings
               { "sSaveGameGameVersionOutdated", "This save game was created on a later version of Skyrim. Please download any updates." },
               { "sSaveGameSafeMarkerID", "1DC0A" },
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
            { sse_only, "bResetGameAfterStreamingInstall", false },
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
               { "bVATSAllowNoKill", false },
               { "bVATSDisable", false },
               { "bVATSForceRanged", false },
               { "bVATSIgnoreProjectileTest", false },
               { "bVATSMultipleCombatants", false },
               { "bVATSRangedSelective", true },
               { "bVATSSmartCameraCheckDebug", false },
               { "bVatsDebug", false },
            #pragma endregion
            #pragma region Floats
               #pragma region V
                  { "fVATSCastingAfterKillDelay", 1.2F },
                  { "fVATSFocus", 3.2F },
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
                  { "fVATSRangedTargetLowLevelMult", 0.3F },
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
               { "iWaterBlurAmount", 1 },
               { "iWaterNoiseResolution", 256 },
            #pragma endregion
            #pragma region Strings
               { "sSurfaceTexture", "water" },
            #pragma endregion
         }),
         section_definition("Weather", {
            { "bFogEnabled", true },
            { "bPrecipitation", true },
            { "fAlphaReduce", 1.0F },
            { "fSunBaseSize", 425.0F },
            { sse_only, "fSunBoost", 1.0F },
            { sse_only, "fSunGlareMultiplier", 2.0F },
            { "fSunGlareSize", 600.0F },
            { "sBumpFadeColor", "255,255,255,255" },
            { "sEnvReduceColor", "255,255,255,255" },
            { "sLerpCloseColor", "255,255,255,255" },
         }),
      });
      #pragma endregion

      #pragma region SkyrimPrefs.ini
      extern const file_definition skyrim_prefs = file_definition("SkyrimPrefs", {
         section_definition("", { // Unnamed section
            { "bCrosshairEnabled", true },
            { "bGamepadEnable", false },
            { "bSaveOnPause", true },
            { "bSaveOnRest", true },
            { "bSaveOnTravel", true },
            { "bSaveOnWait", true },
            { "fHUDOpacity", 1.0F },
            { "fSkyCellRefFadeDistance", 150000.0F },
         }),
         section_definition("AudioMenu", {
            { "fAudioMasterVolume", 1.0F },
            { "fVal0", 1.0F },
            { "fVal1", 1.0F },
            { "fVal2", 1.0F },
            { "fVal3", 1.0F },
            { "fVal4", 1.0F },
            { "fVal5", 1.0F },
            { "fVal6", 1.0F },
            { "fVal7", 1.0F },
         }),
         section_definition("Clouds", {
            { "fCloudLevel0Distance", 16384.0F },
            { "fCloudLevel1Distance", 32768.0F },
            { "fCloudLevel2Distance", 262144.0F },
            { "fCloudNearFadeDistance", 9000.0F },
         }),
         section_definition("Controls", {
            { "bAlwaysRunByDefault", true },
            { "bGamePadRumble", true },
            { "bInvertYValues", false },
            { "bUseKinect", false },
            { "fGamepadHeadingSensitivity", 1.9F },
            { "fMouseHeadingSensitivity", 0.0125F },
         }),
         section_definition("Decals", {
            { sse_only, "bDecals", true },
            { sse_only, "bSkinnedDecals", true },
         }),
         section_definition("Display", {
            #pragma region Booleans
               #pragma region B
                  { sse_only, "bBorderless", false },
               #pragma endregion
               #pragma region D
                  { "bDeferredShadows", true },
                  { "bDrawLandShadows", false },
                  { "bDrawShadows", true },
               #pragma endregion
               #pragma region E
                  { sse_only, "bEnableImprovedSnow", true },
                  { sse_only, "bEnableProjecteUVDiffuseNormals", true },
               #pragma endregion
               #pragma region F
                  { "bFXAAEnabled", false },
                  { "bFloatPointRenderTarget", true },
                  { sse_only, "bForceCreateTarget", false },
                  { "bFull Screen", false },
               #pragma endregion
               #pragma region I
                  { sse_only, "bIBLFEnable", true },
                  { sse_only, "bIndEnable", false },
               #pragma endregion
               #pragma region M
                  { "bMainZPrepass", false },
               #pragma endregion
               #pragma region S
                  { sse_only, "bSAOEnable", true },
                  { sse_only, "bSAO_CS_Enable", false },
                  { sse_only, "bScreenSpaceReflectionEnabled", true },
                  { "bShadowMaskZPrepass", false },
                  { "bShadowsOnGrass", true },
               #pragma endregion
               #pragma region T
                  { sse_only, "bToggleSparkles", true },
                  { "bTransparencyMultisampling", false },
                  { "bTreesReceiveShadows", false },
               #pragma endregion
               #pragma region U
                  { sse_only, "bUse64bitsHDRRenderTarget", false },
                  { sse_only, "bUsePrecipitationOcclusion", true },
                  { sse_only, "bUseTAA", true },
               #pragma endregion
               #pragma region V
                  { sse_only, "bVolumetricLightingEnable", true },
               #pragma endregion
            #pragma endregion
            #pragma region Floats
               #pragma region D
                  { "fDecalLOD1", 1000.0F },
                  { "fDecalLOD2", 1500.0F },
                  { sse_only, "fDynamicDOFBlurMultiplier", 0.8F },
               #pragma endregion
               #pragma region G
                  { "fGamma", 1.0F },
               #pragma endregion
               #pragma region I
                  { "fInteriorShadowDistance", 3000.0F },
               #pragma endregion
               #pragma region L
                  { "fLeafAnimDampenDistEnd", 4600.0F },
                  { "fLeafAnimDampenDistStart", 3600.0F },
                  { "fLightLODStartFade", 1000.0F },
               #pragma endregion
               #pragma region M
                  { "fMeshLODFadeBoundDefault", 256.0F },
                  { "fMeshLODFadePercentDefault", 1.2F },
                  { "fMeshLODLevel1FadeDist", 4096.0F },
                  { "fMeshLODLevel1FadeTreeDistance", 2844.0F },
                  { "fMeshLODLevel2FadeDist", 3072.0F },
                  { "fMeshLODLevel2FadeTreeDistance", 2048.0F },
               #pragma endregion
               #pragma region P
                  { sse_only, "fProjectedUVDiffuseNormalTilingScale", 0.5F },
                  { sse_only, "fProjectedUVNormalDetailTilingScale", 1.2F },
               #pragma endregion
               #pragma region S
                  { "fShadowBiasScale", 1.0F },
                  { "fShadowDistance", 2500.0F },
                  { "fShadowLODStartFade", 200.0F },
                  { "fSpecularLODStartFade", 500.0F },
               #pragma endregion
               #pragma region T
                  { "fTreesMidLODSwitchDist", 3600.0F },
               #pragma endregion
               #pragma region F
                  { sse_only, "ffocusShadowMapDoubleEveryXUnit", 450.0F },
               #pragma endregion
            #pragma endregion
            #pragma region Integers
               #pragma region B
                  { "iBlurDeferredShadowMask", 5 },
               #pragma endregion
               #pragma region M
                  { "iMaxAnisotropy", 8 },
                  { "iMaxDecalsPerFrame", 10 },
                  { "iMaxSkinDecalsPerFrame", 3 },
                  { "iMultiSample", 0 },
               #pragma endregion
               #pragma region N
                  { sse_only, "iNumFocusShadow", 4 },
                  { sse_only, "iNumSplits", 2 },
               #pragma endregion
               #pragma region R
                  { sse_only, "iReflectionResolutionDivider", 2 },
               #pragma endregion
               #pragma region S
                  { sse_only, "iSaveGameScreenShotHeighWSt", 192 },
                  { sse_only, "iSaveGameScreenShotHeight", 192 },
                  { sse_only, "iSaveGameScreenShotWidth", 256 },
                  { sse_only, "iSaveGameScreenShotWidthWS", 320 },
                  { "iScreenShotIndex", 0 },
                  { "iShadowFilter", 3 },
                  { "iShadowMapResolution", 1024 },
                  { "iShadowMaskQuarter", 4 },
                  { "iShadowMode", 3 },
                  { "iSize H", 480 },
                  { "iSize W", 640 },
               #pragma endregion
               #pragma region T
                  { "iTexMipMapMinimum", 0 },
                  { "iTexMipMapSkip", 0 },
               #pragma endregion
               #pragma region V
                  { sse_only, "iVSyncPresentInterval", 1 },
                  { sse_only, "iVolumetricLightingQuality", 1 },
               #pragma endregion
               #pragma region W
                  { "iWaterMultiSamples", 0 },
               #pragma endregion
            #pragma endregion
         }),
         section_definition("GamePlay", {
            { "bShowFloatingQuestMarkers", true },
            { "bShowQuestMarkers", true },
            { "iDifficulty", 2 },
         }),
         section_definition("General", {
            { "bEnableStoryManagerLogging", false },
            { sse_only, "fLightingOutputColourClampPostEnv", 1.0F },
            { sse_only, "fLightingOutputColourClampPostLit", 1.0F },
            { sse_only, "fLightingOutputColourClampPostSpec", 1.0F },
            { "iStoryManagerLoggingEvent", -1 },
         }),
         section_definition("Grass", {
            { "b30GrassVS", false },
            { "fGrassMaxStartFadeDistance", 7000.0F },
            { "fGrassMinStartFadeDistance", 400.0F },
            { "fGrassStartFadeDistance", 3500.0F },
         }),
         section_definition("Imagespace", {
            { "bDoDepthOfField", true },
            { sse_only, "bLensFlare", true },
         }),
         section_definition("Interface", {
            { "bDialogueSubtitles", false },
            { "bGeneralSubtitles", false },
            { "bShowCompass", true },
            { "fMouseCursorSpeed", 1.0F },
         }),
         section_definition("LOD", {
            { "fLODFadeOutMultActors", 6.0F },
            { "fLODFadeOutMultItems", 3.0F },
            { "fLODFadeOutMultObjects", 5.0F },
            { "fLODFadeOutMultSkyCell", 1.0F },
         }),
         section_definition("NavMesh", {
            { sse_only, "fCoverSideHighAlpha", 0.8F },
            { sse_only, "fCoverSideLowAlpha", 0.65F },
            { sse_only, "fEdgeDistFromVert", 10.0F },
            { sse_only, "fEdgeFullAlpha", 1.0F },
            { sse_only, "fEdgeHighAlpha", 0.75F },
            { sse_only, "fEdgeLowAlpha", 0.5F },
            { sse_only, "fEdgeThickness", 10.0F },
            { sse_only, "fLedgeBoxHalfHeight", 25.0F },
            { sse_only, "fObstacleAlpha", 0.5F },
            { sse_only, "fPointSize", 2.5F },
            { sse_only, "fTriangleFullAlpha", 0.7F },
            { sse_only, "fTriangleHighAlpha", 0.35F },
            { sse_only, "fTriangleLowAlpha", 0.2F },
         }),
         section_definition("Particles", {
            { "iMaxDesired", 750 },
         }),
         section_definition("SaveGame", {
            { "fAutosaveEveryXMins", 15.0F },
         }),
         section_definition("TerrainManager", {
            { "bShowLODInEditor", false },
            { "fBlockLevel0Distance", 20480.0F },
            { "fBlockLevel1Distance", 32768.0F },
            { "fBlockMaximumDistance", 100000.0F },
            { "fSplitDistanceMult", 0.75F },
            { "fTreeLoadDistance", 25000.0F },
         }),
         section_definition("Trees", {
            { "bRenderSkinnedTrees", true },
         }),
         section_definition("Water", {
            { "bUseWaterDepth", true },
            { "bUseWaterDisplacements", true },
            { "bUseWaterReflections", true },
            { "bUseWaterRefractions", true },
            { "iWaterReflectHeight", 512 },
            { "iWaterReflectWidth", 512 },
         }),
      });
      #pragma endregion
   }
}