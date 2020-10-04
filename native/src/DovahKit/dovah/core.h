#pragma once
#include <array>
#include <cstdint>

namespace dovah {
   class form_stub;
   namespace tes_file_reading {
      class subrecord;
   }
   namespace tes_file_writing {
      class subrecord;
   }

   using bare_form_id_t = uint32_t;

   struct form_type {
      form_type() = delete;
      enum type : uint8_t {
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
         alias,                    //       BGSBaseAlias
         reference_alias,          //       BGSRefAlias
         location_alias,           //       BGSLocAlias
         active_magic_effect,      //       ActiveMagicEffect
      };
   };
   using form_type_t = std::underlying_type_t<form_type::type>;

   inline constexpr uint32_t hardcoded_form_id_mask = 0x000007FF; // Mask for form IDs that are hardcoded forms.
   inline constexpr uint32_t plugin_form_id_mask    = 0x00FFF800; // Mask for form IDs that are not hardcoded forms.
   inline constexpr uint32_t minimum_plugin_form_id = 0x00000800; // Minimum non-load-order-prefixed form ID for a non-hardcoded form.
   inline constexpr uint32_t form_id_prefix_mask    = 0xFF000000; // Mask to get the load order prefix of a form ID.

   struct form_type_info {
      struct flag {
         flag() = delete;
         enum type : uint32_t {
            none = 0,
            no_editor_id      = 0x01, // Forms of this type cannot have editor IDs.
            no_connections    = 0x02, // Forms of this type cannot refer to or be referred to by other forms.
            can_have_children = 0x04, // Forms of this type can have child forms. (Used to optimize saving.)
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      //
      uint32_t    signature;
      uint8_t     formType;
      const char* name  = "<unknown>";
      flags_t     flags = flag::none;
      //
      static const form_type_info& lookup(form_type_t) noexcept;
      static form_type_t signature_to_form_type(uint32_t signature) noexcept;
      static bool signature_is_suspicious(uint32_t signature) noexcept;

      static bool form_type_is_reference(form_type_t ft) noexcept;
      static bool signature_is_reference(uint32_t signature) noexcept;

      static bool form_type_is_base_form(form_type_t ft) noexcept;
      static bool signature_is_base_form(uint32_t signature) noexcept;

      inline bool is_reference() const noexcept { return form_type_is_reference(this->formType); }
   };
   extern std::array<form_type_info, 140> form_types;
   extern std::array<uint32_t,       120> group_sequence_list; // the order in which record groups appear

   struct form_id_t {
      //
      // This struct exists for a few reasons:
      //
      //  - For file I/O, the subrecord-reading and -writing classes can have "read" functions 
      //    templated on this struct, in order to automate form ID fixup.
      //
      //  - When working with loaded forms, this struct can automate the task of fixing up use 
      //    info when assigning to form IDs.
      //
      // Accordingly, this struct should be used in the following situations only:
      //
      //  - When reading form IDs from a file, or saving them to a file.
      //
      //  - For all form ID members on a loaded-form class.
      //
      // When you wish to change the form ID's value (e.g. to make changes to a loaded form), 
      // you must call this struct's (set) member function, passing two arguments: the 
      // (form_stub) that owns the loaded-form that the form_id_t instance is a member of; and 
      // the form ID or (form_stub) that you want the form_id_t instance to be set to.
      //
      //   [[ NOTE: We can't disable form_id_t::operator=(const form_id_t&), as doing so ]] 
      //   [[ would make it impossible to pass these things as arguments. Be careful not ]] 
      //   [[ to assign one form_id_t to another when cloning a form.                    ]]
      //
      // Note that this struct DOES NOT have a destructor that severs use info. This is by 
      // design, so that a loaded form can be unloaded without severing the use info. However, 
      // that means that if you want to, say, clear a std::vector<form_id_t> on a loaded form 
      // as part of some modification you are making to that form, then you must first call  
      // (form_id_t::set) on each form ID in the vector, passing (bare_form_id_t(0)). Naturally, 
      // this also applies to vectors of structs that have form_id_t members.
      //
      friend class tes_file_reading::subrecord;
      friend class tes_file_writing::subrecord;
      protected:
         uint32_t value = 0;
      public:
         form_id_t() {};
         form_id_t(uint32_t i) : value(i) {};
         //
         inline operator uint32_t() const noexcept { return this->value; };
         //
         inline bool operator>(const uint32_t& other) { return this->value > other; };
         inline bool operator<(const uint32_t& other) { return this->value < other; };
         inline bool operator>=(const uint32_t& other) { return this->value >= other; };
         inline bool operator<=(const uint32_t& other) { return this->value <= other; };
         inline bool operator==(const uint32_t& other) { return this->value == other; };
         inline bool operator!=(const uint32_t& other) { return this->value != other; };
         //
         inline bool operator>(const form_id_t& other) { return this->value > other.value; };
         inline bool operator<(const form_id_t& other) { return this->value < other.value; };
         inline bool operator>=(const form_id_t& other) { return this->value >= other.value; };
         inline bool operator<=(const form_id_t& other) { return this->value <= other.value; };
         inline bool operator==(const form_id_t& other) { return this->value == other.value; };
         inline bool operator!=(const form_id_t& other) { return this->value != other.value; };
         //
         void set(form_stub* owner, bare_form_id_t set_to); // set (this->value) and update use info for the old and new forms
         void set(form_stub* owner, form_stub* set_to);     // set (this->value) and update use info for the old and new forms
         //
      protected:
         inline form_id_t& operator=(const uint32_t& other) { this->value = other; return *this; };
         inline form_id_t& operator=(const int& other) { this->value = other; return *this; };
   };
   struct base_form_id_t : form_id_t {
      //
      // This is a special-case subclass of form_id_t which should be used for REFR/NAME, 
      // a.k.a. a reference's, uh, reference to its base form.
      //
      public:
         void set(form_stub* owner, bare_form_id_t set_to);
         void set(form_stub* owner, form_stub* set_to);
   };
   struct dialogue_form_id_t : form_id_t {
      //
      // This is a special-case subclass of form_id_t which should be used for DIAL/QNAM 
      // and INFO/PNAM.
      //
      public:
         void set(form_stub* owner, bare_form_id_t set_to);
         void set(form_stub* owner, form_stub* set_to);
   };
   struct struct_form_id_t : form_id_t {
      //
      // In some cases, the game reads entire structs from the file by blindly copying bytes. 
      // If these structs contain form IDs, then they will differ in Skyrim Special. Skyrim 
      // treats references from one form to another as unions of form IDs and form pointers; 
      // loading happens in two stages, with the first stage pulling form IDs into the places 
      // where the pointers would be, and the second stage replacing all form IDs with pointers. 
      // Skyrim Special is 64-bit, so its pointers are eight bytes instead of four bytes; as 
      // such, structs that are blindly copied will have their layouts change, with four 
      // padding bytes following each four-byte form ID.
      //
      public:
         uint32_t padding = 0;
   };

   namespace loaded_forms {
      class Form;
   }
}