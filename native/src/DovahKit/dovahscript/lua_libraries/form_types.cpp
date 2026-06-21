#include "form_types.h"
#include <cassert>

// For `form_types.foo:is(form)`
#include "../core/classes.h"
#include "../wrapper.h"
#include "../wrappers/form/form.h"

namespace {
   static constexpr const char* registry_key_for_list = "dovahscript.internal.form_type_singletons";
   
   int _test_form_type_from_upvalue(lua_State* L) {
      auto  ft = (dovah::form_type) lua_tointeger(L, lua_upvalueindex(1));
      auto* ud = (dovahscript::wrapper*) dovahscript::classes::cast_to_class(L, 1, dovahscript::wrappers::form::metatable_key);
      if (!ud) {
         lua_pushboolean(L, false);
         return 1;
      }
      if (ud->depth > 0) {
         lua_pushboolean(L, false);
         return 1;
      }
      auto* stub = ud->stub;
      if (!stub) {
         lua_pushboolean(L, false);
         return 1;
      }
      if (ft == dovah::form_type::reference) {
         lua_pushboolean(L, dovah::form_type_is_reference(stub->form_type));
         return 1;
      }
      lua_pushboolean(L, stub->form_type == ft);
      return 1;
   }

   void _offer_single_form_type(lua_State* L, int stack_pos, const char* name, ::dovah::form_type ft) {
      auto& info = dovah::form_type_info::lookup(ft);
      //
      lua_newuserdatauv(L, 0, 1);  // create form-type singleton
      int ui = lua_gettop(L);
      lua_pushinteger(L, (lua_Integer)ft);      //
      lua_setiuservalue(L, ui, 1); // embed form type enum value
      //
      lua_createtable(L, 0, 2);    // create metatable
      int mi = ui + 1;
      assert(lua_gettop(L) == mi);
      //
      lua_createtable(L, 0, 3); // create __index table
      int ii = mi + 1;
      //
      {
         char signature[5];
         signature[0] = (info.signature >> 0x18) & 0xFF;
         signature[1] = (info.signature >> 0x10) & 0xFF;
         signature[2] = (info.signature >> 0x08) & 0xFF;
         signature[3] = info.signature & 0xFF;
         signature[4] = '\0';
         lua_pushstring(L, signature);
         lua_setfield(L, ii, "signature");
      }
      lua_pushinteger(L, (lua_Integer)ft);
      lua_pushcclosure(L, &_test_form_type_from_upvalue, 1);
      lua_setfield(L, ii, "is");
      lua_pushstring(L, name);
      lua_setfield(L, ii, "name");
      //
      assert(lua_gettop(L) == ii);
      lua_setfield(L, mi, "__index");
      lua_pushstring(L, "form_type");
      lua_setfield(L, mi, "__name");
      //
      lua_setmetatable(L, ui);
      lua_setfield(L, stack_pos, name);
      assert(lua_gettop(L) == ui - 1);
   }
}

namespace dovahscript::lua_libraries::form_types {
   #define __dovah_define(name) _offer_single_form_type(L, table, #name, form_type::name );
   //
   extern void import(lua_State* L) {
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
      _offer_single_form_type(L, table, "placed_missile", form_type::missile);
      _offer_single_form_type(L, table, "placed_arrow",   form_type::arrow);
      _offer_single_form_type(L, table, "placed_grenade", form_type::grenade);
      _offer_single_form_type(L, table, "placed_beam",    form_type::beam);
      _offer_single_form_type(L, table, "placed_flame",   form_type::flame);
      _offer_single_form_type(L, table, "placed_cone",    form_type::cone);
      _offer_single_form_type(L, table, "placed_barrier", form_type::barrier);
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
      //__dovah_define(unk87);
      //
      // Skyrim Special form types:
      //
      __dovah_define(lens_flare);
      __dovah_define(volumetric_lighting);
      //
      // Create C-side map of raw form-type values to singletons:
      //
      lua_settop(L, table);
      lua_createtable(L, 120, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, LUA_REGISTRYINDEX, registry_key_for_list);
      lua_pushnil(L);
      while (lua_next(L, table) != 0) {
         constexpr int ik = -2;
         constexpr int iv = -1;
         if (lua_type(L, iv) == LUA_TUSERDATA) {
            lua_getiuservalue(L, iv, 1);
            int  isnum;
            auto value = lua_tointegerx(L, -1, &isnum);
            lua_pop(L, 1);
            if (isnum) {
               lua_rawseti(L, table + 1, value); // list[value] = iv; // and pops the value
               continue;
            }
         }
         lua_pop(L, 1);
      }
      //
      lua_settop(L, table - 1);
   }
   //
   #undef __dovah_define
   
   extern dovah::form_type pull(lua_State* L, int stack_pos, bool& valid) {
      valid = false;
      if (lua_type(L, stack_pos) != LUA_TUSERDATA) {
         return dovah::form_type::none;
      }
      if (lua_getmetatable(L, stack_pos) == 0) { // push metatable
         return dovah::form_type::none;
      }
      auto type = lua_getfield(L, -1, "__name"); // push meta name
      if (type != LUA_TSTRING) {
         lua_pop(L, 2);
         return dovah::form_type::none;
      }
      valid = (_stricmp(lua_tostring(L, -1), "form_type") == 0);
      lua_pop(L, 2); // pop meta name
      if (!valid) {
         return dovah::form_type::none;
      }
      type = lua_getiuservalue(L, stack_pos, 1); // push meta value
      int isnum;
      int result = lua_tointegerx(L, -1, &isnum);
      lua_pop(L, 1); // pop uservalue
      if (!isnum) {
         valid = false;
         return dovah::form_type::none;
      }
      return (dovah::form_type) result;
   }
   extern void push(lua_State* L, dovah::form_type ft) {
      lua_getfield(L, LUA_REGISTRYINDEX, registry_key_for_list);
      lua_rawgeti(L, -1, (lua_Integer)ft);
      lua_remove(L, -2);
   }
}