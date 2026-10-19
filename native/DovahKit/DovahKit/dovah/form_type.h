#pragma once
#include <cstdint>

namespace dovah {
   enum class form_type : uint8_t {
      none = 0, // NONE
      file_header,              // TES4: File Header
      file_record_group,        // GRUP: File Group
      setting,                  // GMST: Game Setting
      keyword           = 0x04, // KYWD: BGSKeywprd
      location_ref_type,        // LCRT: BGSLocationRefType
      action,                   // AACT: BGSAction
      texture_set,              // TXST: BGSTextureSet
      menu_icon,                // MICN: BGSMenuIcon
      global,                   // GLOB: TESGlobal
      combat_class,             // CLAS: TESClass
      faction,                  // FACT: TESFaction
      head_part,                // HDPT: BGSHeadPart
      eyes,                     // EYES: TESEyes
      race,                     // RACE: TESRace
      sound,                    // SOUN: TESSound
      acoustic_space    = 0x10, // ASPC: BGSAcousticSpace
      skill,                    // SKIL
      magic_effect,             // MGEF: EffectSetting
      script,                   // SCPT: Script
      land_texture,             // LTEX: TESLandTexture
      enchantment,              // ENCH: EnchantmentItem
      spell,                    // SPEL: SpellItem
      scroll,                   // SCRL: ScrollItem
      activator         = 0x18, // ACTI: Activator
      talking_activator,        // TACT: BGSTalkingActivator
      armor,                    // ARMO: TESObjectARMO
      book,                     // BOOK: TESObjectBOOK
      container,                // CONT: TESObjectCONT
      door              = 0x1D, // DOOR: TESObjectDOOR
      ingredient,               // INGR: IngredientItem
      light             = 0x1F, // LIGH: TESObjectLIGH
      misc_item         = 0x20, // MISC: TESObjectMISC
      apparatus,                // APPA: BGSApparatus
      //
      static_scenery,           // STAT: TESObjectSTAT
      statik = static_scenery,
      //
      static_collection,        //       BGSStaticCollection
      movable_static,           // MSTT: BGSMovableStatic
      grass,                    // GRAS: TESGrass
      tree,                     // TREE: TESObjectTREE
      flora,                    // FLOR: TESFlora
      furniture,                // FURN: TESFurniture
      weapon            = 0x29, // WEAP: TESObjectWEAP
      ammo,                     // AMMO: TESAmmo
      actor_base,               // NPC_: TESNPC
      leveled_character,        // LVLN: TESLevCharacter
      key,                      // KEYM: TESKey
      potion,                   // ALCH: AlchemyItem
      idle_marker       = 0x2F, // IDLM: BGSIdleMarker
      note              = 0x30, // NOTE: BGSNote
      constructible_object,     // COBJ: BGSConstructibleObject
      projectile,               // PROJ: BGSProjectile
      hazard,                   // HAZD: BGSHazard
      soul_gem,                 // SLGM: TESSoulGem
      leveled_item,             // LVLI: TESLevItem
      weather,                  // WTHR: TESWeather
      climate,                  // CLMT: TESClimate
      shader_particle_geometry_data, // SPGD: BGSShaderParticleGeometryData
      //
      reference_effect,         // RFCT: BGSReferenceEffect
      visual_effect = reference_effect,
      //
      region,                   // REGN: TESRegion
      navmesh_info_map,         // NAVI: NavMeshInfoMap
      cell              = 0x3C, // CELL: TESObjectCELL
      reference         = 0x3D, // REFR: TESObjectREFR
      actor             = 0x3E, // ACHR: Actor / Character (previous games distinguished between Characters and Creatures)
      missile           = 0x3F, // PMIS: MissileProjectile
      arrow             = 0x40, // PARW: ArrowProjectile
      grenade,                  // PGRE: GrenadeProjectile
      beam,                     // PBEA: BeamProjectile
      flame,                    // PLFA: FlamePrjoectile
      cone,                     // PCON: ConeProjectile
      barrier,                  // PBAR: BarrierProjectile
      placed_hazard,            // PHZD: Hazard
      worldspace        = 0x47, // WRLD: TESWorldSpace
      land,                     // LAND: TESObjectLAND
      navmesh,                  // NAVM: NavMesh
      tlod,                     // TLOD
      topic,                    // DIAL: TESTopic
      topic_info,               // INFO: TESTopicInfo
      quest,                    // QUST: TESQuest
      idle,                     // IDLE: TESIdleForm
      package           = 0x4F, // PACK: TESPackage
      combat_style      = 0x50, // CSTY: TESCombatStyle
      loading_screen,           // LSCR: TESLoadScreen
      leveled_spell,            // LVSP: TESLevSpell
      animation_prop,           // ANIO: TESObjectANIO
      water_type,               // WATR: TESWaterForm
      effect_shader,            // EFSH: TESEffectShader
      toft,                     // TOFT
      explosion,                // EXPL: BGSExplosion
      debris,                   // DEBR: BGSDebris
      imagespace,               // IMGS: TESImageSpace
      imagespace_modifier,      // IMAD: TESImageSpaceModifier
      formlist,                 // FLST: BGSListForm
      perk,                     // PERK: BGSPerk
      body_part_data,           // BPTD: BGSBodyPartData
      addon_node,               // ADDN: BGSAddonNode
      actor_value_info  = 0x5F, // AVIF: ActorValueInfo
      camera_shot       = 0x60, // CAMS: BGSCameraShot
      camera_path,              // CPTH: BGSCameraPath
      voicetype,                // VTYP: BGSVoiceType
      material_type,            // MATT: BGSMaterialType
      impact_data,              // IPCT: BGSImpactData
      impact_data_set,          // IPDS: BGSImpactDataSet
      armor_addon,              // ARMA: TESObjectARMA
      encounter_zone,           // ECZN: BGSEncounterZone
      location,                 // LCTN: BGSLocation
      message,                  // MESG: BGSMessage
      ragdoll,                  // RGDL: BGSRagdoll
      default_object_manager,   // DOBJ
      lighting_template,        // LGTM: BGSLightingTemplate
      music_type,               // MUSC: BGSMusicType
      footstep,                 // FSTP: BGSFootstep
      footstep_set      = 0x6F, // FSTS: BGSFootstepSet
      story_branch_node = 0x70, // SMBN: BGSStoryManagerBranchNode
      story_quest_node,         // SMQN: BGSStoryManagerQuestNode
      story_event_node,         // SMEN: BGSStoryManagerEventNode
      dialogue_branch,          // DLBR: BGSDialogueBranch
      music_track,              // MUST: BGSMusicTrackFormWrapper
      dialogue_view,            // DLVW
      word_of_power,            // WOOP: TESWordOfPower
      shout,                    // SHOU: TESShout
      equip_slot,               // EQUP: BGSEquipSlot
      relationship,             // RELA: BGSRelationship
      scene,                    // SCEN: BGSScene
      association_type,         // ASTP: BGSAssociationType
      outfit,                   // OTFT: BGSOutfit
      art_object,               // ARTO: BGSArtObject
      material_object,          // MATO: BGSMaterialObject
      movement_type     = 0x7F, // MOVT: BGSMovementType
      sound_descriptor  = 0x80, // SNDR: BGSSoundDescriptorForm
      dual_cast_data,           // DUAL: BGSDualCastData
      sound_category,           // SNCT: BGSSoundCategory
      sound_output_model,       // SOPM: BGSSoundOutput
      collision_layer,          // COLL: BGSCollisionLayer
      color,                    // CLFM: BGSColorForm
      reverb_parameters,        // REVB: BGSReverbParameters
      unk87,                    // 
      //
      // New to Skyrim Special:
      //
      lens_flare,               // LENS: 
      volumetric_lighting,      // VOLI: 
      //
      // Not forms, but some game systems reserve form-type values for them:
      //
      alias,                    //       BGSBaseAlias
      reference_alias,          //       BGSRefAlias
      location_alias,           //       BGSLocAlias
      active_magic_effect,      //       ActiveMagicEffect
   };
}