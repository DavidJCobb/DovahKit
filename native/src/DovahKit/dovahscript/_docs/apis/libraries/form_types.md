
# form_types

A list of userdata describing every form type in Skyrim. API functions that take form types as an argument expect to receive one of these objects.

## Form type information

### Methods

<dl>
   <dt>form_types[x]:is(form)</dt>
   <dd>
      Returns true if <var>form</var> is a form of the given type, or false otherwise.
   </dd>
</dl>

### Properties

<dl>
   <dt>form_types[x].name</dt>
   <dd>
      A string: the same name you can use to access the form type object. For example, <code>form_types.static.name</code> is "static".
   </dd>
   <dt>form_types[x].signature</dt>
   <dd>
      Returns a four-character code, or FourCC, representing the form type, e.g. "STAT" or "NPC_". These are the same codes used by Skyrim internally and within its file format, and by DovahKit and the Creation Kit in some parts of their respective UIs.
   </dd>
</dl>

## List of all form types

* form_types.none
* form_types.setting
* form_types.keyword
* form_types.location_ref_type
* form_types.action
* form_types.texture_set
* form_types.menu_icon
* form_types.global
* form_types.combat_class
* form_types.faction
* form_types.head_part
* form_types.eyes
* form_types.race
* form_types.sound
* form_types.acoustic_space
* form_types.skill
* form_types.magic_effect
* form_types.script
* form_types.land_texture
* form_types.enchantment
* form_types.spell
* form_types.scroll
* form_types.activator
* form_types.talking_activator
* form_types.armor
* form_types.book
* form_types.container
* form_types.door
* form_types.ingredient
* form_types.light
* form_types.misc_item
* form_types.apparatus
* form_types.static
* form_types.static_collection
* form_types.movable_static
* form_types.grass
* form_types.tree
* form_types.flora
* form_types.furniture
* form_types.weapon
* form_types.ammo
* form_types.actor_base
* form_types.leveled_character
* form_types.key
* form_types.potion
* form_types.idle_marker
* form_types.note
* form_types.constructible_object
* form_types.projectile
* form_types.hazard
* form_types.soul_gem
* form_types.leveled_item
* form_types.weather
* form_types.climate
* form_types.shader_particle_geometry_data
* form_types.reference_effect
* form_types.region
* form_types.navmesh_info_map
* form_types.cell
* form_types.reference
* form_types.actor
* form_types.missile
* form_types.arrow
* form_types.grenade
* form_types.beam
* form_types.flame
* form_types.cone
* form_types.barrier
* form_types.placed_hazard
* form_types.worldspace
* form_types.land
* form_types.navmesh
* form_types.topic
* form_types.topic_info
* form_types.quest
* form_types.idle
* form_types.package
* form_types.combat_style
* form_types.loading_screen
* form_types.leveled_spell
* form_types.animation_prop
* form_types.water_type
* form_types.effect_shader
* form_types.explosion
* form_types.debris
* form_types.imagespace
* form_types.imagespace_modifier
* form_types.formlist
* form_types.perk
* form_types.body_part_data
* form_types.addon_node
* form_types.actor_value_info
* form_types.camera_shot
* form_types.camera_path
* form_types.voicetype
* form_types.material_type
* form_types.impact_data
* form_types.impact_data_set
* form_types.armor_addon
* form_types.encounter_zone
* form_types.location
* form_types.message
* form_types.ragdoll
* form_types.default_object_manager
* form_types.lighting_template
* form_types.music_type
* form_types.footstep
* form_types.footstep_set
* form_types.story_branch_node
* form_types.story_quest_node
* form_types.story_event_node
* form_types.dialogue_branch
* form_types.music_track
* form_types.dialogue_view
* form_types.word_of_power
* form_types.shout
* form_types.equip_slot
* form_types.relationship
* form_types.scene
* form_types.association_type
* form_types.outfit
* form_types.art_object
* form_types.material_object
* form_types.movement_type
* form_types.sound_descriptor
* form_types.dual_cast_data
* form_types.sound_category
* form_types.sound_output_model
* form_types.collision_layer
* form_types.color
* form_types.reverb_parameters
* form_types.unk87

The following form types are only valid in Skyrim Special Edition. You can access them even if your script is running in a Skyrim Classic load order, but you cannot create or look up forms of these types under those conditions.

* form_types.lens_flare
* form_types.volumetric_lighting