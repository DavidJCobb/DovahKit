#include "form_type_values.h"
#include "../../../Lua/lua.hpp"

namespace editor_script {
   namespace {
      void _offer_single_form_type(lua_State* L, int stack_pos, const char* name, ::dovah::form_type_t ft) {
         lua_pushinteger(L, ::dovah::form_type_info::lookup(ft).signature);
         lua_setfield   (L, stack_pos, name);
      }
   }

   #define __dovah_define(name) _offer_single_form_type(L, table, #name, form_type::##name );
   //
   void expose_form_types_to_lua(lua_State* L) {
      lua_createtable(L, 0, 120);
      auto table = lua_gettop(L);
      lua_pushvalue(L, table);
      lua_setglobal(L, "form_types");
      //
      using form_type = dovah::form_type;
      //
      __dovah_define(none);
      //__dovah_define(file_header);
      //__dovah_define(file_record_group);
      __dovah_define(setting);
      __dovah_define(keyword);
      __dovah_define(location_ref_type);
      __dovah_define(action);
      __dovah_define(texture_set);
      __dovah_define(menu_icon);
      __dovah_define(global);
      __dovah_define(combat_class);
      __dovah_define(faction);
      __dovah_define(head_part);
      __dovah_define(eyes);
      __dovah_define(race);
      __dovah_define(sound);
      __dovah_define(acoustic_space);
      __dovah_define(skill);
      __dovah_define(magic_effect);
      __dovah_define(script);
      __dovah_define(land_texture);
      __dovah_define(enchantment);
      __dovah_define(spell);
      __dovah_define(scroll);
      __dovah_define(activator);
      __dovah_define(talking_activator);
      __dovah_define(armor);
      __dovah_define(book);
      __dovah_define(container);
      __dovah_define(door);
      __dovah_define(ingredient);
      __dovah_define(light);
      __dovah_define(misc_item);
      __dovah_define(apparatus);
      _offer_single_form_type(L, table, "static", form_type::statik);
      __dovah_define(static_collection);
      __dovah_define(movable_static);
      __dovah_define(grass);
      __dovah_define(tree);
      __dovah_define(flora);
      __dovah_define(furniture);
      __dovah_define(weapon);
      __dovah_define(ammo);
      __dovah_define(actor_base);
      __dovah_define(leveled_character);
      __dovah_define(key);
      __dovah_define(potion);
      __dovah_define(idle_marker);
      __dovah_define(note);
      __dovah_define(constructible_object);
      __dovah_define(projectile);
      __dovah_define(hazard);
      __dovah_define(soul_gem);
      __dovah_define(leveled_item);
      __dovah_define(weather);
      __dovah_define(climate);
      __dovah_define(shader_particle_geometry_data);
      __dovah_define(reference_effect);
      __dovah_define(region);
      __dovah_define(navmesh_info_map);
      __dovah_define(cell);
      __dovah_define(reference);
      __dovah_define(actor);
      __dovah_define(missile);
      __dovah_define(arrow);
      __dovah_define(grenade);
      __dovah_define(beam);
      __dovah_define(flame);
      __dovah_define(cone);
      __dovah_define(barrier);
      __dovah_define(placed_hazard);
      __dovah_define(worldspace);
      __dovah_define(land);
      __dovah_define(navmesh);
      //__dovah_define(tlod);
      __dovah_define(topic);
      __dovah_define(topic_info);
      __dovah_define(quest);
      __dovah_define(idle);
      __dovah_define(package);
      __dovah_define(combat_style);
      __dovah_define(loading_screen);
      __dovah_define(leveled_spell);
      __dovah_define(animation_prop);
      __dovah_define(water_type);
      __dovah_define(effect_shader);
      //__dovah_define(toft);
      __dovah_define(explosion);
      __dovah_define(debris);
      __dovah_define(imagespace);
      __dovah_define(imagespace_modifier);
      __dovah_define(formlist);
      __dovah_define(perk);
      __dovah_define(body_part_data);
      __dovah_define(addon_node);
      __dovah_define(actor_value_info);
      __dovah_define(camera_shot);
      __dovah_define(camera_path);
      __dovah_define(voicetype);
      __dovah_define(material_type);
      __dovah_define(impact_data);
      __dovah_define(impact_data_set);
      __dovah_define(armor_addon);
      __dovah_define(encounter_zone);
      __dovah_define(location);
      __dovah_define(message);
      __dovah_define(ragdoll);
      __dovah_define(default_object_manager);
      __dovah_define(lighting_template);
      __dovah_define(music_type);
      __dovah_define(footstep);
      __dovah_define(footstep_set);
      __dovah_define(story_branch_node);
      __dovah_define(story_quest_node);
      __dovah_define(story_event_node);
      __dovah_define(dialogue_branch);
      __dovah_define(music_track);
      __dovah_define(dialogue_view);
      __dovah_define(word_of_power);
      __dovah_define(shout);
      __dovah_define(equip_slot);
      __dovah_define(relationship);
      __dovah_define(scene);
      __dovah_define(association_type);
      __dovah_define(outfit);
      __dovah_define(art_object);
      __dovah_define(material_object);
      __dovah_define(movement_type);
      __dovah_define(sound_descriptor);
      __dovah_define(dual_cast_data);
      __dovah_define(sound_category);
      __dovah_define(sound_output_model);
      __dovah_define(collision_layer);
      __dovah_define(color);
      __dovah_define(reverb_parameters);
      __dovah_define(unk87);
      //
      // Skyrim Special form types:
      //
      __dovah_define(lens_flare);
      __dovah_define(volumetric_lighting);
      //
      lua_settop(L, table - 1);
   }
   //
   #undef __dovah_define

   dovah::form_type_t get_form_type_from_stack(lua_State* L, int stack_pos, bool& valid) {
      uint32_t arg = lua_tointeger(L, stack_pos);
      auto     ft  = dovah::form_type_info::signature_to_form_type(arg);
      //
      valid = true;
      if (!ft)
         valid = dovah::form_types[0].signature == arg;
      //
      return ft;
   }
   void push_form_type_to_stack(lua_State* L, dovah::form_type_t ft) {
      lua_pushinteger(L, ::dovah::form_type_info::lookup(ft).signature);
   }
}