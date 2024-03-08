#pragma once
#include <array>
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
      reference_effect,         // RFCT: BGSReferenceEffect
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
   using form_type_integral = std::underlying_type_t<form_type>;

   constexpr bool form_type_is_reference(form_type ft) noexcept {
      switch (ft) {
         case form_type::actor:
         case form_type::reference:
         case form_type::arrow:
         case form_type::barrier:
         case form_type::beam:
         case form_type::cone:
         case form_type::flame:
         case form_type::grenade:
         case form_type::placed_hazard:
         case form_type::missile:
            return true;
      }
      return false;
   }
   constexpr bool form_signature_is_reference(uint32_t signature) noexcept {
      switch (signature) {
         case 'ACHR':
         case 'REFR':
         case 'PMIS':
         case 'PARW':
         case 'PGRE':
         case 'PBEA':
         case 'PFLA':
         case 'PCON':
         case 'PBAR':
         case 'PHZD':
            return true;
      }
      return false;
   }

   constexpr bool form_type_is_base_form(form_type ft) noexcept {
      switch (ft) {
         case form_type::activator:
         case form_type::actor_base:
         case form_type::apparatus:
         case form_type::armor:
         case form_type::book:
         case form_type::container:
         case form_type::door:
         case form_type::flora:
         case form_type::furniture:
         case form_type::ingredient:
         case form_type::key:
         case form_type::light:
         case form_type::misc_item:
         case form_type::movable_static:
         case form_type::potion:
            // TODO: projectile?
         case form_type::scroll:
         case form_type::soul_gem:
         case form_type::sound:
         case form_type::statik:
         case form_type::talking_activator:
         case form_type::tree:
         case form_type::weapon:
            return true;
      }
      return false;
   }
   constexpr bool form_signature_is_base_form(uint32_t signature) noexcept {
      switch (signature) {
         case 'ACTI':
         case 'NPC_':
         case 'APPA':
         case 'ARMO':
         case 'BOOK':
         case 'CONT':
         case 'DOOR':
         case 'FLOR':
         case 'FURN':
         case 'INGR':
         case 'KEYM':
         case 'LIGH':
         case 'MISC':
         case 'MSTT':
         case 'ALCH':
            // TODO: projectile?
         case 'SCRL':
         case 'SLGM':
         case 'SOUN':
         case 'STAT':
         case 'TACT':
         case 'TREE':
         case 'WEAP':
            return true;
      }
      return false;
   }

   struct form_type_info {
      struct flag {
         flag() = delete;
         enum type : uint8_t {
            none = 0,
            no_editor_id      = 0x01, // Forms of this type cannot have editor IDs.
            no_connections    = 0x02, // Forms of this type cannot refer to or be referred to by other forms.
            can_have_children = 0x04, // Forms of this type can have child forms. (Used to optimize saving.)
            empty_if_deleted  = 0x08, // Forms of this type don't save any subrecords if they're flagged as deleted.
            is_skyrim_special = 0x10, // Forms of this type don't exist in Skyrim Classic.
            is_singleton      = 0x20, // Only one form of this type can exist. All definitions are coalesced into a "singleton" form.
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      
      const char*    name        = "<unknown>";
      uint32_t       signature   = 0;
      enum form_type form_type   = dovah::form_type::none;
      enum form_type parent_type = dovah::form_type::none;
      flags_t        flags       = flag::none;
      
      static constexpr const form_type_info& lookup(enum form_type) noexcept;
      static constexpr enum form_type signature_to_form_type(uint32_t signature) noexcept;
      static constexpr bool signature_is_suspicious(uint32_t signature) noexcept;

      inline constexpr bool is_reference() const noexcept { return form_type_is_reference(this->form_type); }
   };

   constexpr const bool is_valid_form_type(enum form_type ft) noexcept {
      return form_type_info::lookup(ft).form_type == ft;
   }

   constexpr std::array<form_type_info, 138> form_types = {{
      {  // form types not in this list are effectively 'NONE'
         .name        = "None/Unknown",
         .signature   = 'NONE',
         .form_type   = form_type::none,
         .flags       = form_type_info::flag::no_editor_id
      },
      {
         .name        = "File Header",
         .signature   = 'TES4',
         .form_type   = form_type::file_header,
         .flags       = form_type_info::flag::no_connections,
      },
      {
         .name        = "File Record Group",
         .signature   = 'GRUP',
         .form_type   = form_type::file_record_group,
         .flags       = form_type_info::flag::no_connections,
      },
      {
         .name        = "GameSetting",
         .signature   = 'GMST',
         .form_type   = form_type::setting,
         .flags       = form_type_info::flag::no_connections,
      },
      {
         .name        = "Keyword",
         .signature   = 'KYWD',
         .form_type   = form_type::keyword,
      },
      {
         .name        = "LocRefType",
         .signature   = 'LCRT',
         .form_type   = form_type::location_ref_type,
         .parent_type = form_type::keyword,
      },
      {
         .name        = "Action",
         .signature   = 'AACT',
         .form_type   = form_type::action,
      },
      {
         .name        = "TextureSet",
         .signature   = 'TXST',
         .form_type   = form_type::texture_set,
      },
      {
         .name        = "Menu Icon",
         .signature   = 'MICN',
         .form_type   = form_type::menu_icon,
      },
      {
         .name        = "Global",
         .signature   = 'GLOB',
         .form_type   = form_type::global,
      },
      {
         .name        = "Class",
         .signature   = 'CLAS',
         .form_type   = form_type::combat_class,
      },
      {
         .name        = "Faction",
         .signature   = 'FACT',
         .form_type   = form_type::faction,
      },
      {
         .name        = "HeadPart",
         .signature   = 'HDPT',
         .form_type   = form_type::head_part,
      },
      //
      // HAIR - TESHair - removed in patch 1.2
      //
      {
         .name        = "Eyes",
         .signature   = 'EYES',
         .form_type   = form_type::eyes,
      },
      {
         .name        = "Race",
         .signature   = 'RACE',
         .form_type   = form_type::race,
      },
      {
         .name        = "Sound",
         .signature   = 'SOUN',
         .form_type   = form_type::sound,
      },
      {
         .name        = "AcousticSpace",
         .signature   = 'ASPC',
         .form_type   = form_type::acoustic_space,
      },
      {
         .name        = "Skill",
         .signature   = 'SKIL',
         .form_type   = form_type::skill,
      },
      {
         .name        = "MagicEffect",
         .signature   = 'MGEF',
         .form_type   = form_type::magic_effect,
      },
      {
         .name        = "Script (TES4)",
         .signature   = 'SCPT',
         .form_type   = form_type::script,
      },
      {
         .name        = "LandTexture",
         .signature   = 'LTEX',
         .form_type   = form_type::land_texture,
      },
      {
         .name        = "Enchantment",
         .signature   = 'ENCH',
         .form_type   = form_type::enchantment,
      },
      {
         .name        = "Spell",
         .signature   = 'SPEL',
         .form_type   = form_type::spell,
      },
      {
         .name        = "Scroll",
         .signature   = 'SCRL',
         .form_type   = form_type::scroll,
      },
      {
         .name        = "Activator",
         .signature   = 'ACTI',
         .form_type   = form_type::activator,
      },
      {
         .name        = "Talking Activator",
         .signature   = 'TACT',
         .form_type   = form_type::talking_activator,
         .parent_type = form_type::activator,
      },
      {
         .name        = "Armor",
         .signature   = 'ARMO',
         .form_type   = form_type::armor,
      },
      {
         .name        = "Book",
         .signature   = 'BOOK',
         .form_type   = form_type::book,
      },
      {
         .name        = "Container",
         .signature   = 'CONT',
         .form_type   = form_type::container,
      },
      {
         .name        = "Door",
         .signature   = 'DOOR',
         .form_type   = form_type::door,
      },
      {
         .name        = "Ingredient",
         .signature   = 'INGR',
         .form_type   = form_type::ingredient,
      },
      {
         .name        = "Light",
         .signature   = 'LIGH',
         .form_type   = form_type::light,
      },
      {
         .name        = "Misc. Item",
         .signature   = 'MISC',
         .form_type   = form_type::misc_item,
      },
      {
         .name        = "Apparatus",
         .signature   = 'APPA',
         .form_type   = form_type::apparatus,
         .parent_type = form_type::misc_item,
      },
      {
         .name        = "Static",
         .signature   = 'STAT',
         .form_type   = form_type::statik,
      },
      {
         .name        = "Static Collection",
         .signature   = 'SCOL',
         .form_type   = form_type::static_collection,
      },
      {
         .name        = "MovableStatic",
         .signature   = 'MSTT',
         .form_type   = form_type::movable_static,
      },
      {
         .name        = "Grass",
         .signature   = 'GRAS',
         .form_type   = form_type::grass,
      },
      {
         .name        = "Tree",
         .signature   = 'TREE',
         .form_type   = form_type::tree,
      },
      //
      // CLDC - BGSCloudCluster - removed in patch 1.2
      //
      {
         .name        = "Flora",
         .signature   = 'FLOR',
         .form_type   = form_type::flora,
         .parent_type = form_type::activator,
      },
      {
         .name        = "Furniture",
         .signature   = 'FURN',
         .form_type   = form_type::furniture,
         .parent_type = form_type::activator,
      },
      {
         .name        = "Weapon",
         .signature   = 'WEAP',
         .form_type   = form_type::weapon,
      },
      {
         .name        = "Ammo",
         .signature   = 'AMMO',
         .form_type   = form_type::ammo,
      },
      {
         .name        = "ActorBase",
         .signature   = 'NPC_',
         .form_type   = form_type::actor_base,
      },
      {
         .name        = "Leveled Actor",
         .signature   = 'LVLN',
         .form_type   = form_type::leveled_character,
      },
      {
         .name        = "Key",
         .signature   = 'KEYM',
         .form_type   = form_type::key,
         .parent_type = form_type::misc_item,
      },
      {
         .name        = "Potion",
         .signature   = 'ALCH',
         .form_type   = form_type::potion,
      },
      {
         .name        = "Idle Marker",
         .signature   = 'IDLM',
         .form_type   = form_type::idle_marker,
      },
      {
         .name        = "Note",
         .signature   = 'NOTE',
         .form_type   = form_type::note,
      },
      {
         .name        = "Constructible Object",
         .signature   = 'COBJ',
         .form_type   = form_type::constructible_object,
         .parent_type = form_type::misc_item,
      },
      {
         .name        = "Projectile",
         .signature   = 'PROJ',
         .form_type   = form_type::projectile,
      },
      {
         .name        = "Hazard",
         .signature   = 'HAZD',
         .form_type   = form_type::hazard,
      },
      {
         .name        = "Soul Gem",
         .signature   = 'SLGM',
         .form_type   = form_type::soul_gem,
         .parent_type = form_type::misc_item,
      },
      {
         .name        = "Leveled Item",
         .signature   = 'LVLI',
         .form_type   = form_type::leveled_item,
      },
      {
         .name        = "Weather",
         .signature   = 'WTHR',
         .form_type   = form_type::weather,
      },
      {
         .name        = "Climate",
         .signature   = 'CLMT',
         .form_type   = form_type::climate,
      },
      {
         .name        = "Shader Particle Geometry",
         .signature   = 'SPGD',
         .form_type   = form_type::shader_particle_geometry_data,
      },
      {
         .name        = "VisualEffect",
         .signature   = 'RFCT',
         .form_type   = form_type::reference_effect,
      },
      {
         .name        = "Region",
         .signature   = 'REGN',
         .form_type   = form_type::region,
      },
      {
         .name        = "Navmesh Info Map",
         .signature   = 'NAVI',
         .form_type   = form_type::navmesh_info_map,
         .flags       = form_type_info::flag::is_singleton,
      },
      {
         .name        = "Cell",
         .signature   = 'CELL',
         .form_type   = form_type::cell,
         .flags       = form_type_info::flag::can_have_children,
      },
      {
         .name        = "ObjectReference",
         .signature   = 'REFR',
         .form_type   = form_type::reference,
      },
      {
         .name        = "Actor",
         .signature   = 'ACHR',
         .form_type   = form_type::actor,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Missile Projectile",
         .signature   = 'PMIS',
         .form_type   = form_type::missile,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Arrow Projectile",
         .signature   = 'PARW',
         .form_type   = form_type::arrow,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Grenade Projectile",
         .signature   = 'PGRE',
         .form_type   = form_type::grenade,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Beam Projectile",
         .signature   = 'PBEA',
         .form_type   = form_type::beam,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Flame Projectile",
         .signature   = 'PFLA',
         .form_type   = form_type::flame,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Cone Projectile",
         .signature   = 'PCON',
         .form_type   = form_type::cone,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Barrier Projectile",
         .signature   = 'PBAR',
         .form_type   = form_type::barrier,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Placed Hazard",
         .signature   = 'PHZD',
         .form_type   = form_type::placed_hazard,
         .parent_type = form_type::reference,
      },
      {
         .name        = "Worldspace",
         .signature   = 'WRLD',
         .form_type   = form_type::worldspace,
         .flags       = form_type_info::flag::can_have_children,
      },
      {
         .name        = "Landscape",
         .signature   = 'LAND',
         .form_type   = form_type::land,
         .flags       = form_type_info::flag::no_editor_id,
      },
      {
         .name        = "Navmesh",
         .signature   = 'NAVM',
         .form_type   = form_type::navmesh,
      },
      {
         .name        = "Unknown (TLOD)",
         .signature   = 'TLOD',
         .form_type   = form_type::tlod,
      },
      {
         .name        = "Dialogue Topic",
         .signature   = 'DIAL',
         .form_type   = form_type::topic,
         .flags       = form_type_info::flag::can_have_children,
      },
      {
         .name        = "Dialogue Topic Info",
         .signature   = 'INFO',
         .form_type   = form_type::topic_info,
         .flags       = form_type_info::flag::empty_if_deleted,
      },
      {
         .name        = "Quest",
         .signature   = 'QUST',
         .form_type   = form_type::quest,
         .flags       = form_type_info::flag::empty_if_deleted,
      },
      {
         .name        = "Idle",
         .signature   = 'IDLE',
         .form_type   = form_type::idle,
      },
      {
         .name        = "Package",
         .signature   = 'PACK',
         .form_type   = form_type::package,
      },
      {
         .name        = "Combat Style",
         .signature   = 'CSTY',
         .form_type   = form_type::combat_style,
      },
      {
         .name        = "Loading Screen",
         .signature   = 'LSCR',
         .form_type   = form_type::loading_screen,
      },
      {
         .name        = "Leveled Spell",
         .signature   = 'LVSP',
         .form_type   = form_type::leveled_spell,
      },
      {
         .name        = "Animation Prop",
         .signature   = 'ANIO',
         .form_type   = form_type::animation_prop,
      },
      {
         .name        = "Water Type",
         .signature   = 'WATR',
         .form_type   = form_type::water_type,
      },
      {
         .name        = "EffectShader",
         .signature   = 'EFSH',
         .form_type   = form_type::effect_shader,
      },
      {
         .name        = "Unknown (TOFT)",
         .signature   = 'TOFT',
         .form_type   = form_type::toft,
      },
      {
         .name        = "Explosion",
         .signature   = 'EXPL',
         .form_type   = form_type::explosion,
      },
      {
         .name        = "Debris",
         .signature   = 'DEBR',
         .form_type   = form_type::debris,
      },
      {
         .name        = "ImageSpace",
         .signature   = 'IMGS',
         .form_type   = form_type::imagespace,
      },
      {
         .name        = "ImageSpace Modifier",
         .signature   = 'IMAD',
         .form_type   = form_type::imagespace_modifier,
      },
      {
         .name        = "FormList",
         .signature   = 'FLST',
         .form_type   = form_type::formlist,
      },
      {
         .name        = "Perk",
         .signature   = 'PERK',
         .form_type   = form_type::perk,
      },
      {
         .name        = "Body Part Data",
         .signature   = 'BPTD',
         .form_type   = form_type::body_part_data,
      },
      {
         .name        = "Add-on Node",
         .signature   = 'ADDN',
         .form_type   = form_type::addon_node,
      },
      {
         .name        = "ActorValue Info",
         .signature   = 'AVIF',
         .form_type   = form_type::actor_value_info,
      },
      {
         .name        = "Camera Shot",
         .signature   = 'CAMS',
         .form_type   = form_type::camera_shot,
      },
      {
         .name        = "Camera Path",
         .signature   = 'CPTH',
         .form_type   = form_type::camera_path,
      },
      {
         .name        = "Voicetype",
         .signature   = 'VTYP',
         .form_type   = form_type::voicetype,
      },
      {
         .name        = "Material Type",
         .signature   = 'MATT',
         .form_type   = form_type::material_type,
      },
      {
         .name        = "Impact Data",
         .signature   = 'IPCT',
         .form_type   = form_type::impact_data,
      },
      {
         .name        = "Impact Data Set",
         .signature   = 'IPDS',
         .form_type   = form_type::impact_data_set,
      },
      {
         .name        = "Armor Addon",
         .signature   = 'ARMA',
         .form_type   = form_type::armor_addon,
      },
      {
         .name        = "Encounter Zone",
         .signature   = 'ECZN',
         .form_type   = form_type::encounter_zone,
      },
      {
         .name        = "Location",
         .signature   = 'LCTN',
         .form_type   = form_type::location,
      },
      {
         .name        = "Message",
         .signature   = 'MESG',
         .form_type   = form_type::message,
      },
      {
         .name        = "Ragdoll",
         .signature   = 'RGDL',
         .form_type   = form_type::ragdoll,
      },
      {
         .name        = "Default Objects",
         .signature   = 'DOBJ',
         .form_type   = form_type::default_object_manager,
         .flags       = form_type_info::flag::is_singleton,
      },
      {
         .name        = "Lighting Template",
         .signature   = 'LGTM',
         .form_type   = form_type::lighting_template,
      },
      {
         .name        = "MusicType",
         .signature   = 'MUSC',
         .form_type   = form_type::music_type,
      },
      {
         .name        = "Footstep",
         .signature   = 'FSTP',
         .form_type   = form_type::footstep,
      },
      {
         .name        = "Footstep Set",
         .signature   = 'FSTS',
         .form_type   = form_type::footstep_set,
      },
      {
         .name        = "Story Manager Branch Node",
         .signature   = 'SMBN',
         .form_type   = form_type::story_branch_node,
      },
      {
         .name        = "Story Manager Quest Node",
         .signature   = 'SMQN',
         .form_type   = form_type::story_quest_node,
      },
      {
         .name        = "Story Manager Event Node",
         .signature   = 'SMEN',
         .form_type   = form_type::story_event_node,
      },
      {
         .name        = "Dialogue Branch",
         .signature   = 'DLBR',
         .form_type   = form_type::dialogue_branch,
         .flags       = form_type_info::flag::empty_if_deleted,
      },
      {
         .name        = "Music Track",
         .signature   = 'MUST',
         .form_type   = form_type::music_track,
      },
      {
         .name        = "Dialogue View",
         .signature   = 'DLVW',
         .form_type   = form_type::dialogue_view,
      },
      {
         .name        = "Word Of Power",
         .signature   = 'WOOP',
         .form_type   = form_type::word_of_power,
      },
      {
         .name        = "Shout",
         .signature   = 'SHOU',
         .form_type   = form_type::shout,
      },
      {
         .name        = "Equip Slot",
         .signature   = 'EQUP',
         .form_type   = form_type::equip_slot,
      },
      {
         .name        = "Relationship",
         .signature   = 'RELA',
         .form_type   = form_type::relationship,
      },
      {
         .name        = "Scene",
         .signature   = 'SCEN',
         .form_type   = form_type::scene,
      },
      {
         .name        = "Association Type",
         .signature   = 'ASTP',
         .form_type   = form_type::association_type,
      },
      {
         .name        = "Outfit",
         .signature   = 'OTFT',
         .form_type   = form_type::outfit,
      },
      {
         .name        = "Art Object",
         .signature   = 'ARTO',
         .form_type   = form_type::art_object,
      },
      {
         .name        = "Material Object",
         .signature   = 'MATO',
         .form_type   = form_type::material_object,
      },
      {
         .name        = "Movement Type",
         .signature   = 'MOVT',
         .form_type   = form_type::movement_type,
      },
      {
         .name        = "Sound Descriptor",
         .signature   = 'SNDR',
         .form_type   = form_type::sound_descriptor,
      },
      {
         .name        = "Dual-Cast Data",
         .signature   = 'DUAL',
         .form_type   = form_type::dual_cast_data,
      },
      {
         .name        = "Sound Category",
         .signature   = 'SNCT',
         .form_type   = form_type::sound_category,
      },
      {
         .name        = "Sound Output Model",
         .signature   = 'SOPM',
         .form_type   = form_type::sound_output_model,
      },
      {
         .name        = "Collision Layer",
         .signature   = 'COLL',
         .form_type   = form_type::collision_layer,
      },
      {
         .name        = "Color",
         .signature   = 'CLFM',
         .form_type   = form_type::color,
      },
      {
         .name        = "Reverb Parameters",
         .signature   = 'REVB',
         .form_type   = form_type::reverb_parameters,
      },
      //
      // New to Skyrim Special:
      //
      {
         .name        = "Lens Flare",
         .signature   = 'LENS',
         .form_type   = form_type::lens_flare,
         .flags       = form_type_info::flag::is_skyrim_special,
      },
      {
         .name        = "Volumetric Lighting Information",
         .signature   = 'VOLI',
         .form_type   = form_type::volumetric_lighting,
         .flags       = form_type_info::flag::is_skyrim_special,
      },
   }};
   constexpr std::array<uint32_t, 120> group_sequence_list = {{
      'GMST',
      'KYWD',
      'LCRT',
      'AACT',
      'TXST',
      'GLOB',
      'CLAS',
      'FACT',
      'HDPT',
      'HAIR',
      'EYES',
      'RACE',
      'SOUN',
      'ASPC',
      'MGEF',
      'SCPT',
      'LTEX',
      'ENCH',
      'SPEL',
      'SCRL',
      'ACTI',
      'TACT',
      'ARMO',
      'BOOK',
      'CONT',
      'DOOR',
      'INGR',
      'LIGH',
      'MISC',
      'APPA',
      'STAT',
      'SCOL',
      'MSTT',
      'PWAT',
      'GRAS',
      'TREE',
      'CLDC',
      'FLOR',
      'FURN',
      'WEAP',
      'AMMO',
      'NPC_',
      'PLYR',
      'LVLN',
      'KEYM',
      'ALCH',
      'IDLM',
      'COBJ',
      'PROJ',
      'HAZD',
      'SLGM',
      'LVLI',
      'WTHR',
      'CLMT',
      'SPGD',
      'RFCT',
      'REGN',
      'NAVI',
      'CELL',
      'WRLD',
      'DIAL',
      'QUST',
      'IDLE',
      'PACK',
      'CSTY',
      'LSCR',
      'LVSP',
      'ANIO',
      'WATR',
      'EFSH',
      'EXPL',
      'DEBR',
      'IMGS',
      'IMAD',
      'FLST',
      'PERK',
      'BPTD',
      'ADDN',
      'AVIF',
      'CAMS',
      'CPTH',
      'VTYP',
      'MATT',
      'IPCT',
      'IPDS',
      'ARMA',
      'ECZN',
      'LCTN',
      'MESG',
      'RGDL',
      'DOBJ',
      'LGTM',
      'MUSC',
      'FSTP',
      'FSTS',
      'SMBN',
      'SMQN',
      'SMEN',
      'DLBR',
      'MUST',
      'DLVW',
      'WOOP',
      'SHOU',
      'EQUP',
      'RELA',
      'SCEN',
      'ASTP',
      'OTFT',
      'ARTO',
      'MATO',
      'VOLI', // SSE
      'MOVT',
      'SNDR',
      'DUAL',
      'SNCT',
      'SOPM',
      'COLL',
      'CLFM',
      'REVB',
      'LENS', // SSE
   }};

   #pragma region form_type_info constexpr member implementations
   /*static*/ constexpr const form_type_info& form_type_info::lookup(enum form_type ft) noexcept {
      constexpr const size_t form_types_are_contiguous_up_to = []() -> size_t {
         for (size_t i = 0; i < form_types.size(); ++i)
            if ((size_t)form_types[i].form_type != i)
               return i;
         return form_types.size();
      }();

      if constexpr (form_types_are_contiguous_up_to == form_types.size()) {
         if ((size_t)ft < form_types.size())
            return form_types[(size_t)ft];
      } else {
         if ((size_t)ft < form_types_are_contiguous_up_to)
            return form_types[(size_t)ft];
         for (const auto& info : form_types)
            if (info.form_type == ft)
               return info;
      }
      return form_types[0];
   }
   /*static*/ constexpr form_type form_type_info::signature_to_form_type(uint32_t signature) noexcept {
      for (const auto& info : form_types)
         if (info.signature == signature)
            return info.form_type;
      return form_type::none;
   }
   /*static*/ constexpr bool form_type_info::signature_is_suspicious(uint32_t signature) noexcept {
      for (int i = 0; i < 4; ++i) {
         unsigned char c = (signature >> (0x08 * i)) & 0xFF;
         if (c != '_' && !(c >= '0' && c <= '9') && !(c >= 'A' && c <= 'Z'))
            return true;
      }
      return false;
   }
   #pragma endregion
}