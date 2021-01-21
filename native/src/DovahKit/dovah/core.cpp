#include "core.h"
#include "form_stub.h"
#include "forms/form.h"

namespace dovah {
   #pragma region huge arrays
   std::array<form_type_info, 138> form_types = {{
      { 'NONE', form_type::none, "None/Unknown", form_type_info::flag::no_editor_id }, // form types not in this list are effectively 'NONE'
      { 'TES4', form_type::file_header, "File Header", form_type_info::flag::no_connections },
      { 'GRUP', form_type::file_record_group, "File Record Group", form_type_info::flag::no_connections },
      { 'GMST', form_type::setting, "GameSetting", form_type_info::flag::no_connections }, // Skyrim's loader handles this as a special case; there is no factory for this form type.
      { 'KYWD', form_type::keyword, "Keyword" },
      { 'LCRT', form_type::location_ref_type, "LocRefType" },
      { 'AACT', form_type::action, "Action" },
      { 'TXST', form_type::texture_set, "TextureSet" },
      { 'MICN', form_type::menu_icon, "Menu Icon" }, // BGSMenuIcon
      { 'GLOB', form_type::global,   "Global" },
      { 'CLAS', form_type::combat_class,    "Class" },
      { 'FACT', form_type::faction,  "Faction" },
      { 'HDPT', form_type::head_part, "HeadPart" },
      // HAIR - TESHair - removed in patch 1.2
      { 'EYES', form_type::eyes,     "Eyes" },
      { 'RACE', form_type::race,     "Race" },
      { 'SOUN', form_type::sound,    "Sound" },
      { 'ASPC', form_type::acoustic_space, "AcousticSpace" },
      { 'SKIL', form_type::skill,       "Skill" },
      { 'MGEF', form_type::magic_effect, "MagicEffect" },
      { 'SCPT', form_type::script,      "Script (TES4)" },
      { 'LTEX', form_type::land_texture, "LandTexture" },
      { 'ENCH', form_type::enchantment, "Enchantment" },
      { 'SPEL', form_type::spell,       "Spell" },
      { 'SCRL', form_type::scroll,      "Scroll" }, // as in, magic scrolls
      { 'ACTI', form_type::activator,   "Activator" },
      { 'TACT', form_type::talking_activator, "Talking Activator" },
      { 'ARMO', form_type::armor,       "Armor" },
      { 'BOOK', form_type::book,        "Book" },
      { 'CONT', form_type::container,   "Container" },
      { 'DOOR', form_type::door,        "Door" },
      { 'INGR', form_type::ingredient,  "Ingredient" },
      { 'LIGH', form_type::light,       "Light" },
      { 'MISC', form_type::misc_item,    "Misc. Item" },
      { 'APPA', form_type::apparatus,   "Apparatus" },
      { 'STAT', form_type::statik,      "Static" },
      { 'SCOL', form_type::static_collection, "Static Collection" },
      { 'MSTT', form_type::movable_static, "MovableStatic" },
      { 'GRAS', form_type::grass, "Grass" },
      { 'TREE', form_type::tree, "Tree" },
      // CLDC - BGSCloudCluster - removed in patch 1.2
      { 'FLOR', form_type::flora, "Flora" },
      { 'FURN', form_type::furniture, "Furniture" },
      { 'WEAP', form_type::weapon, "Weapon" },
      { 'AMMO', form_type::ammo, "Ammo" },
      { 'NPC_', form_type::actor_base, "ActorBase" },
      { 'LVLN', form_type::leveled_character, "Leveled Actor" },
      { 'KEYM', form_type::key, "Key" },
      { 'ALCH', form_type::potion, "Potion" },
      { 'IDLM', form_type::idle_marker, "Idle Marker" },
      { 'NOTE', form_type::note, "Note" },
      { 'COBJ', form_type::constructible_object, "Constructible Object" },
      { 'PROJ', form_type::projectile, "Projectile" },
      { 'HAZD', form_type::hazard, "Hazard" },
      { 'SLGM', form_type::soul_gem, "Soul Gem" },
      { 'LVLI', form_type::leveled_item, "Leveled Item" },
      { 'WTHR', form_type::weather, "Weather" },
      { 'CLMT', form_type::climate, "Climate" },
      { 'SPGD', form_type::shader_particle_geometry_data, "Shader Particle Geometry" },
      { 'RFCT', form_type::reference_effect, "VisualEffect" }, // "ReferenceEffect," internally
      { 'REGN', form_type::region, "Region" },
      { 'NAVI', form_type::navmesh_info_map,  "Navmesh Info Map", form_type_info::flag::is_singleton },
      { 'CELL', form_type::cell,  "Cell", form_type_info::flag::can_have_children },
      { 'REFR', form_type::reference, "ObjectReference" },
      { 'ACHR', form_type::actor, "Actor" },
      { 'PMIS', form_type::missile, "Placed Missile Projectile" },
      { 'PARW', form_type::arrow,   "Placed Arrow Projectile" },
      { 'PGRE', form_type::grenade, "Placed Grenade Projectile" },
      { 'PBEA', form_type::beam,    "Placed Beam Projectile" },
      { 'PFLA', form_type::flame,   "Placed Flame Projectile" },
      { 'PCON', form_type::barrier, "Placed Barrier Projectile" },
      { 'PBAR', form_type::cone,    "Placed Cone Projectile" },
      { 'PHZD', form_type::placed_hazard,            "Placed Hazard" },
      { 'WRLD', form_type::worldspace, "Worldspace", form_type_info::flag::can_have_children },
      { 'LAND', form_type::land, "Landscape", form_type_info::flag::no_editor_id },
      { 'NAVM', form_type::navmesh, "Navmesh" },
      { 'TLOD', form_type::tlod, "Unknown (TLOD)" },
      { 'DIAL', form_type::topic, "Dialogue Topic", form_type_info::flag::can_have_children },
      { 'INFO', form_type::topic_info, "Dialogue Topic Info", form_type_info::flag::empty_if_deleted },
      { 'QUST', form_type::quest, "Quest", form_type_info::flag::empty_if_deleted },
      { 'IDLE', form_type::idle, "Idle" },
      { 'PACK', form_type::package, "Package" },
      { 'CSTY', form_type::combat_style, "Combat Style" },
      { 'LSCR', form_type::loading_screen, "Loading Screen" },
      { 'LVSP', form_type::leveled_spell, "Leveled Spell" },
      { 'ANIO', form_type::animation_prop, "Animation Prop" }, // a.k.a. AnimObject
      { 'WATR', form_type::water_type, "Water Type" },
      { 'EFSH', form_type::effect_shader, "EffectShader" },
      { 'TOFT', form_type::toft, "Unknown (TOFT)" },
      { 'EXPL', form_type::explosion, "Explosion" },
      { 'DEBR', form_type::debris, "Debris" },
      { 'IMGS', form_type::imagespace, "ImageSpace" },
      { 'IMAD', form_type::imagespace_modifier, "ImageSpace Modifier" },
      { 'FLST', form_type::formlist, "FormList" },
      { 'PERK', form_type::perk, "Perk" },
      { 'BPTD', form_type::body_part_data, "Body Part Data" },
      { 'ADDN', form_type::addon_node, "Add-on Node" },
      { 'AVIF', form_type::actor_value_info, "ActorValue Info" },
      { 'CAMS', form_type::camera_shot, "Camera Shot" },
      { 'CPTH', form_type::camera_path, "Camera Path" },
      { 'VTYP', form_type::voicetype, "Voicetype" },
      { 'MATT', form_type::material_type, "Material Type" },
      { 'IPCT', form_type::impact_data, "Impact Data" },
      { 'IPDS', form_type::impact_data_set, "Impact Data Set" },
      { 'ARMA', form_type::armor_addon, "Armor Addon" },
      { 'ECZN', form_type::encounter_zone, "Encounter Zone" },
      { 'LCTN', form_type::location, "Location" },
      { 'MESG', form_type::message, "Message" },
      { 'RGDL', form_type::ragdoll, "Ragdoll" }, // BGSRagdoll
      { 'DOBJ', form_type::default_object_manager, "Default Objects", form_type_info::flag::is_singleton }, // Skyrim's loader handles this as a special case; there is no factory for this form type.
      { 'LGTM', form_type::lighting_template, "Lighting Template" },
      { 'MUSC', form_type::music_type, "MusicType" },
      { 'FSTP', form_type::footstep, "Footstep" },
      { 'FSTS', form_type::footstep_set, "Footstep Set" },
      { 'SMBN', form_type::story_branch_node, "Story Manager Branch Node" },
      { 'SMQN', form_type::story_quest_node, "Story Manager Quest Node" },
      { 'SMEN', form_type::story_event_node, "Story Manager Event Node" },
      { 'DLBR', form_type::dialogue_branch, "Dialogue Branch", form_type_info::flag::empty_if_deleted },
      { 'MUST', form_type::music_track, "Music Track" },
      { 'DLVW', form_type::dialogue_view, "Dialogue View" }, // CK only; not loaded by the game (the form factory table has a null entry for this form type).
      { 'WOOP', form_type::word_of_power, "Word Of Power" },
      { 'SHOU', form_type::shout, "Shout" },
      { 'EQUP', form_type::equip_slot, "Equip Slot" },
      { 'RELA', form_type::relationship, "Relationship" },
      { 'SCEN', form_type::scene, "Scene" },
      { 'ASTP', form_type::association_type, "Association Type" },
      { 'OTFT', form_type::outfit, "Outfit" },
      { 'ARTO', form_type::art_object, "Art Object" },
      { 'MATO', form_type::material_object, "Material Object" },
      { 'MOVT', form_type::movement_type, "Movement Type" },
      { 'SNDR', form_type::sound_descriptor, "Sound Descriptor" },
      { 'DUAL', form_type::dual_cast_data, "Dual-Cast Data" },
      { 'SNCT', form_type::sound_category, "Sound Category" },
      { 'SOPM', form_type::sound_output_model, "Sound Output Model" },
      { 'COLL', form_type::collision_layer, "Collision Layer" }, // BGSCollisionLayer
      { 'CLFM', form_type::color, "Color" },
      { 'REVB', form_type::reverb_parameters, "Reverb Parameters" },
      //
      // New to Skyrim Special:
      //
      { 'LENS', form_type::lens_flare, "Lens Flare", form_type_info::flag::is_skyrim_special },
      { 'VOLI', form_type::volumetric_lighting, "Volumetric Lighting Information", form_type_info::flag::is_skyrim_special },
   }};
   std::array<uint32_t, 120> group_sequence_list = {{
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
   #pragma endregion

   extern bool game_supports_light_plugins(game g) {
      switch (g) {
         case game::skyrim_special:
            return true;
      }
      return false;
   }

   /*static*/ file_prefix file_prefix::from_form_id(bare_form_id_t id, bool is_skyrim_classic) noexcept {
      if (!id)
         return file_prefix();
      uint8_t prefix = id >> 0x18;
      if (is_skyrim_classic || prefix != 0xFE)
         return file_prefix::make_heavy(prefix);
      return file_prefix::make_light((id >> 0xC) & 0xFFF);
   }

   #pragma region form_type_info
   /*static*/ const form_type_info& form_type_info::lookup(form_type_t ft) noexcept {
      for (uint8_t i = 0; i < form_types.size(); i++) {
         auto& info = form_types[i];
         if (info.formType == ft)
            return info;
      }
      return form_types[0];
   }
   /*static*/ form_type_t form_type_info::signature_to_form_type(uint32_t signature) noexcept {
      for (uint8_t i = 0; i < form_types.size(); i++) {
         auto& info = form_types[i];
         if (info.signature == signature)
            return info.formType;
      }
      return 0;
   }

   namespace {
      inline bool _isNumber(char c) { return c >= '0' && c <= '9'; }
      inline bool _isLetter(char c) { return c >= 'A' && c <= 'Z'; }
   }
   /*static*/ bool form_type_info::signature_is_suspicious(uint32_t signature) noexcept {
      unsigned char c;
      c = signature & 0xFF;
      if (!_isNumber(c) && !_isLetter(c) && c != '_')
         return true;
      c = (signature >> 0x08) & 0xFF;
      if (!_isNumber(c) && !_isLetter(c) && c != '_')
         return true;
      c = (signature >> 0x10) & 0xFF;
      if (!_isNumber(c) && !_isLetter(c) && c != '_')
         return true;
      c = (signature >> 0x18) & 0xFF;
      if (!_isNumber(c) && !_isLetter(c) && c != '_')
         return true;
      return false;
   }

   /*static*/ bool form_type_info::form_type_is_reference(form_type_t ft) noexcept {
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
   /*static*/ bool form_type_info::signature_is_reference(uint32_t signature) noexcept {
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

   /*static*/ bool form_type_info::form_type_is_base_form(form_type_t ft) noexcept {
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
   /*static*/ bool form_type_info::signature_is_base_form(uint32_t signature) noexcept {
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
   #pragma endregion

   #pragma region form_reference_t
   bare_form_id_t form_reference_t::formID() const noexcept {
      return this->stub ? this->stub->formID : 0;
   }
   void form_reference_t::clear_if(loaded_forms::Form& owner, form_stub& clear_if) {
      if (this->stub == &clear_if)
         this->set(owner, nullptr);
   }
   void form_reference_t::set(loaded_forms::Form& owner, form_stub* set_to) {
      if (this->stub == set_to)
         return;
      if (owner.stub) {
         bare_form_id_t old = this->stub ? this->stub->formID : 0;
         owner.stub->replace_outbound_reference(old, set_to, this->use_info_flags);
      }
      this->stub = set_to;
   }
   void form_reference_t::set(loaded_forms::Form& owner, const form_reference_t& set_to) {
      this->set(owner, set_to.stub);
   }
   bool form_reference_t::form_type_matches(form_type_t ft) const noexcept {
      if (!this->stub)
         return true;
      return this->stub->formType == ft;
   }
   void form_reference_t::unmanaged_set(form_stub* set_to) {
      this->stub = set_to;
   }
   //
   base_form_reference_t::base_form_reference_t() : form_reference_t(use_info_entry::flag::object_reference) {};
   base_form_reference_t::base_form_reference_t(form_stub* s) : form_reference_t(use_info_entry::flag::object_reference, s) {};
   //
   dialogue_form_reference_t::dialogue_form_reference_t() : form_reference_t(use_info_entry::flag::dialogue) {};
   dialogue_form_reference_t::dialogue_form_reference_t(form_stub* s) : form_reference_t(use_info_entry::flag::dialogue, s) {};
   //
   void struct_form_reference_t::set(loaded_forms::Form& owner, const struct_form_reference_t& set_to) {
      form_reference_t::set(owner, set_to.stub);
      this->padding = set_to.padding;
   }

   void clear_form_reference_list(std::vector<form_reference_t>& list, loaded_forms::Form& owner) {
      for (auto& id : list)
         id.set(owner, nullptr);
      list.clear();
   }
   void remove_form_from_reference_list(std::vector<form_reference_t>& list, form_stub& target, loaded_forms::Form& owner) {
      bool edits = false;
      for (auto& id : list) {
         if (id == &target) {
            id.set(owner, nullptr);
            edits = true;
         }
      }
      if (edits) {
         list.erase(
            std::remove_if(list.begin(), list.end(), [](const form_reference_t& id) { return id == nullptr; }),
            list.end()
         );
      }
   }
   void copy_form_reference_list(loaded_forms::Form& target_owner, std::vector<form_reference_t>& target, std::vector<form_reference_t>& source) {
      size_t size = source.size();
      if (target.size() < size)
         target.resize(size);
      for (size_t i = 0; i < size; ++i)
         target[i].set(target_owner, source[i]);
      if (target.size() > size) {
         for (size_t i = size; i < target.size(); ++i)
            target[i].set(target_owner, nullptr);
         target.resize(size);
      }
   }
   #pragma endregion
}