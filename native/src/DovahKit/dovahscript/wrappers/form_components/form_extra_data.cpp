#include "./form_extra_data.h"
#include <concepts>
#include <type_traits>
#include "helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/extra_data.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/ObjectReference.h"

#include "dovahscript/wrappers/form/form.h"
#include "./form_extra_data/p/primitive.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::form_extra_data;
   using wrapped_type = cls::wrapped_type;

   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

wrapped_type* cls::unwrap(wrapper& w) {
   if (w.is_collection)
      return nullptr;
   auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
   if (!form)
      return nullptr;
   switch (form->stub.form_type) {
      case dovah::form_type::cell:
         return &((dovah::loaded_forms::Cell*)form)->extra_data;
      case dovah::form_type::reference:
         return &((dovah::loaded_forms::ObjectReference*)form)->extra_data;
   }
   return nullptr;
}

#pragma region Extra data types
   #pragma region A
      // TODO: action
      // TODO: activate_parents
      // TODO: alpha_cutoff
      // TODO: ammo
      #include "dovah/forms/components/extra_data/types/a/attach_ref.h"
   #pragma endregion
   #pragma region C
      // TODO: cell_acoustic_space
      // TODO: cell_climate
      // TODO: cell_grass_data
      // TODO: cell_imagespace
      // TODO: cell_music_override
      // TODO: cell_region_list
      // TODO: cell_water_type
      #include "dovah/forms/components/extra_data/types/c/charge.h"
      #include "dovah/forms/components/extra_data/types/c/collision_data.h"
      #include "dovah/forms/components/extra_data/types/c/count.h"
   #pragma endregion
   #pragma region D
      // TODO: distant_data
   #pragma endregion
   #pragma region E
      #include "dovah/forms/components/extra_data/types/e/emittance_source.h"
      // TODO: enable_state_parent
      #include "dovah/forms/components/extra_data/types/e/encounter_zone.h"
   #pragma endregion
   #pragma region F
      #include "dovah/forms/components/extra_data/types/f/favor_cost.h"
   #pragma endregion
   #pragma region G
      #include "dovah/forms/components/extra_data/types/g/global.h"
   #pragma endregion
   #pragma region H
      #include "dovah/forms/components/extra_data/types/h/headtracking_weight.h"
      #include "dovah/forms/components/extra_data/types/h/health.h"
      #include "dovah/forms/components/extra_data/types/h/health_percent.h"
      #include "dovah/forms/components/extra_data/types/h/horse.h"
   #pragma endregion
   #pragma region I
      #include "dovah/forms/components/extra_data/types/i/ignored_by_sandbox.h"
      #include "dovah/forms/components/extra_data/types/i/interior_lock_list.h"
   #pragma endregion
   #pragma region L
      // TODO: leveled_creature_modifier
      #include "dovah/forms/components/extra_data/types/l/leveled_item_base.h"
      // TODO: light
      // TODO: linked_ref
      // TODO: linked_ref_color
      // TODO: lit_water
      #include "dovah/forms/components/extra_data/types/l/location.h"
      #include "dovah/forms/components/extra_data/types/l/location_ref_type.h"
      // TODO: lock
   #pragma endregion
   #pragma region M
      // TODO: map_marker
      #include "dovah/forms/components/extra_data/types/m/merchant_container.h"
      // TODO: multibound_bounds
      // TODO: multibound_ref
   #pragma endregion
   #pragma region N
      // TODO: navmesh_door_portal
   #pragma endregion
   #pragma region O
      // TODO: occlusion_plane
      // TODO: occlusion_plane_ref_data
      #include "dovah/forms/components/extra_data/types/o/ownership.h"
   #pragma endregion
   #pragma region P
      // TODO: package_start_location
      // TODO: patrol_ref_data
      // TODO: poison
      // TODO: portal
      // TODO: portal_origin_and_destination
      #include "dovah/forms/components/extra_data/types/p/primitive.h"
   #pragma endregion
   #pragma region R
      #include "dovah/forms/components/extra_data/types/r/radius.h"
      // TODO: ragdoll_data
      // TODO: random_teleport_marker
      // TODO: rank
      // TODO: reflector_refs
      // TODO: room_ref_data
   #pragma endregion
   #pragma region S
      #include "dovah/forms/components/extra_data/types/s/scale.h"
      #include "dovah/forms/components/extra_data/types/s/spawn_container.h"
   #pragma endregion
   #pragma region T
      // TODO: teleport
      // TODO: teleport_name
      #include "dovah/forms/components/extra_data/types/t/time_left.h"
   #pragma endregion
   #pragma region W
      // TODO: water_current_zone_data
      // TODO: water_data
      #include "dovah/forms/components/extra_data/types/w/water_environment_map.h"
   #pragma endregion
#pragma endregion

namespace {
   #pragma region Extra-data metaprogramming concepts
      namespace impl {
         template<typename T>
         concept type_has_signature = requires {
            { T::signature };
            requires std::is_same_v<decltype(T::signature), const uint32_t>;
         };
      }

      template<typename T>
      concept common_fundamental_extra_data_type = requires{
         typename T::value_type;
         requires impl::type_has_signature<T>;
         { T::default_value };
         requires std::is_same_v<decltype(T::default_value), const typename T::value_type>;
         requires std::is_base_of_v<
            extra_data_types::common_fundamental<T, T::signature, typename T::value_type, T::default_value>,
            T
         >;
      };

      template<typename T, typename ValueType>
      concept common_fundamental_extra_data_type_with_value_type = requires{
         requires common_fundamental_extra_data_type<T>;
         requires std::is_same_v<typename T::value_type, ValueType>;
      };
      template<typename T>
      concept common_fundamental_integral_extra_data_type = requires {
         requires common_fundamental_extra_data_type<T>;
         requires std::is_integral_v<typename T::value_type> && !std::is_same_v<typename T::value_type, bool>;
      };
      
      template<typename T>
      concept common_string_extra_data_type = requires {
         requires impl::type_has_signature<T>;
         requires std::is_base_of_v<
            extra_data_types::common_string<T, T::signature>,
            T
         >;
      };
      
      namespace impl {
         template<typename T>
         concept type_has_allowed_form_types = requires {
            { T::allowed_form_types };
         };

         template<type_has_allowed_form_types T>
         struct common_form_allowed_type_shorthand {
            static constexpr const auto value = T::allowed_form_types;
         };
         //
         template<typename T>
            requires (
               type_has_allowed_form_types<T> &&
               (std::tuple_size_v<std::decay_t<decltype(T::allowed_form_types)>> == 0)
            )
         struct common_form_allowed_type_shorthand<T> {
            static constexpr const auto value = dovah::form_type::none;
         };
         //
         template<typename T>
            requires (
               type_has_allowed_form_types<T> &&
               (std::tuple_size_v<std::decay_t<decltype(T::allowed_form_types)>> == 1)
            )
         struct common_form_allowed_type_shorthand<T> {
            static constexpr const auto value = T::allowed_form_types[0];
         };

         template<typename T>
         struct is_common_form : std::false_type {};
         //
         template<typename T>
            requires (
               type_has_signature<T> &&
               type_has_allowed_form_types<T>
            )
         struct is_common_form<T> {
            static constexpr const bool value = std::is_base_of_v<
               extra_data_types::common_form<T, T::signature, T::allowed_form_types>,
               T
            > || std::is_base_of_v<
               extra_data_types::common_form<T, T::signature, common_form_allowed_type_shorthand<T>::value>,
               T
            >;
         };

         template<typename T>
         struct is_common_unique_form : std::false_type {};
         //
         template<typename T>
            requires (
               type_has_signature<T> &&
               type_has_allowed_form_types<T> &&
               requires {
                  { decltype(T::form)::use_info_flag };
               }
            )
         struct is_common_unique_form<T> {
            static constexpr const auto use_info_flag = decltype(T::form)::use_info_flag;

            static constexpr const bool value = std::is_base_of_v<
               extra_data_types::common_unique_form<T, T::signature, T::allowed_form_types, use_info_flag>,
               T
            > || std::is_base_of_v<
               extra_data_types::common_unique_form<T, T::signature, common_form_allowed_type_shorthand<T>::value, use_info_flag>,
               T
            >;
         };
      }
      template<typename T>
      concept common_form_extra_data_type = impl::is_common_form<T>::value || impl::is_common_unique_form<T>::value;
   #pragma endregion

   #pragma region meta_extra_data_ops<ExtraData>
      template<typename ExtraData>
      struct meta_extra_data_ops;
   
      template<common_form_extra_data_type ExtraData>
      struct meta_extra_data_ops<ExtraData> {
         static int getter(lua_State* L) {
            auto& self = get_wrapper_for_thiscall<cls>(L);
            auto* edl  = cls::unwrap(self);
            if (!edl)
               return 0;
            auto* data = edl->get<ExtraData>();
            if (!data) {
               lua_pushnil(L);
               return 1;
            }
            return push_native_object(data->form);
         }
         static int setter(lua_State* L) {
            auto& self = get_wrapper_for_thiscall<cls>(L);
            api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
            auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
            auto* edl  = cls::unwrap(self);
            if (!edl)
               return 0;

            auto* data  = edl->get<ExtraData>();
            if (lua_isnoneornil(L, 2)) {
               if (data) {
                  self.before_edit();
                  edl->remove(data);
                  self.after_edit();
               }
               return 0;
            }
            auto* value = pull_form_stub_argument(L, 2);
            luaL_argcheck(L, !!value, 2, "nil or form expected");

            if constexpr (ExtraData::allowed_form_types.size()) {
               bool valid = false;
               for (auto ft : ExtraData::allowed_form_types) {
                  if (value->form_type == ft) {
                     valid = true;
                     break;
                  }
               }
               if (!valid) {
                  cobb::lua::argerror(L, 2, "the specified form is of the wrong type"); // TODO: state the allowed type(s)
               }
            }

            self.before_edit();
            if (!data)
               data = edl->get_or_create<ExtraData>();
            data->form.set(*form, value);
            self.after_edit();
            return 0;
         }
      };
   
      template<common_fundamental_extra_data_type_with_value_type<float> ExtraData>
      struct meta_extra_data_ops<ExtraData> {
         static int getter(lua_State* L) {
            auto& self = get_wrapper_for_thiscall<cls>(L);
            auto* edl  = cls::unwrap(self);
            if (!edl)
               return 0;
            auto* data = edl->get<ExtraData>();
            if (!data) {
               lua_pushnil(L);
               return 1;
            }
            lua_pushnumber(L, data->value);
            return 1;
         }
         static int setter(lua_State* L) {
            auto& self = get_wrapper_for_thiscall<cls>(L);
            api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
            auto* edl  = cls::unwrap(self);
            if (!edl)
               return 0;

            auto* data = edl->get<ExtraData>();
            if (lua_isnoneornil(L, 2)) {
               if (data) {
                  self.before_edit();
                  edl->remove(data);
                  self.after_edit();
               }
               return 0;
            }
            luaL_argcheck(L, lua_isnumber(L, 2), 2, "nil or number expected");

            self.before_edit();
            if (!data)
               data = edl->get_or_create<ExtraData>();
            data->value = lua_tonumber(L, 2);
            self.after_edit();
            return 0;
         }
      };
      template<common_fundamental_integral_extra_data_type ExtraData>
      struct meta_extra_data_ops<ExtraData> {
         static int getter(lua_State* L) {
            auto& self = get_wrapper_for_thiscall<cls>(L);
            auto* edl  = cls::unwrap(self);
            if (!edl)
               return 0;
            auto* data = edl->get<ExtraData>();
            if (!data) {
               lua_pushnil(L);
               return 1;
            }
            lua_pushinteger(L, data->value);
            return 1;
         }
         static int setter(lua_State* L) {
            auto& self = get_wrapper_for_thiscall<cls>(L);
            api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
            auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
            auto* edl  = cls::unwrap(self);
            if (!edl)
               return 0;

            auto* data = edl->get<ExtraData>();
            if (lua_isnoneornil(L, 2)) {
               if (data) {
                  self.before_edit();
                  edl->remove(data);
                  self.after_edit();
               }
               return 0;
            }
            luaL_argcheck(L, lua_isinteger(L, 2), 2, "nil or integer expected");

            self.before_edit();
            if (!data)
               data = edl->get_or_create<ExtraData>();
            data->value = lua_tointeger(L, 2);
            self.after_edit();
            return 0;
         }
      };
   
      template<common_string_extra_data_type ExtraData>
      struct meta_extra_data_ops<ExtraData> {
      static int getter(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* edl  = cls::unwrap(self);
         if (!edl)
            return 0;
         auto* data = edl->get<ExtraData>();
         if (!data) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushstring(L, data->value.c_str());
         return 1;
      }
      static int setter(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* edl  = cls::unwrap(self);
         if (!edl)
            return 0;

         auto* data = edl->get<ExtraData>();
         if (lua_isnoneornil(L, 2)) {
            if (data) {
               self.before_edit();
               edl->remove(data);
               self.after_edit();
            }
            return 0;
         }
         luaL_argcheck(L, lua_isstring(L, 2), 2, "nil or string expected");

         self.before_edit();
         if (!data)
            data = edl->get_or_create<ExtraData>();
         data->value = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
   };
   #pragma endregion

   template<typename ExtraDataWrapper>
   struct meta_extra_wrapper_ops {
      using extra_data_wrapper = ExtraDataWrapper;
      using extra_data_type    = extra_data_wrapper::wrapped_type;

      static int getter(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* edl  = cls::unwrap(self);
         if (!edl)
            return 0;
         auto* data = edl->get<extra_data_type>();
         if (!data) {
            lua_pushnil(L);
            return 1;
         }

         wrapper out = self;
         out.append_part(ExtraDataWrapper::wrapper_part_type);
         return core::subsystems::userdata::get().push(L, out, ExtraDataWrapper::metatable_key);
      }
      static int setter(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* edl = cls::unwrap(self);
         if (!edl)
            return 0;

         if (lua_isnoneornil(L, 2)) {
            self.before_edit();
            edl->remove<extra_data_type>(*form);
            self.after_edit();
            return 0;
         }
         if (!lua_istable(L, 2) && !lua_isuserdata(L, 2))
            cobb::lua::argerror(L, 2, "nil, table, or userdata expected");
         extra_data_wrapper::validate_table_for_assign(L, 2);
         
         self.before_edit();
         {
            auto* data = edl->get_or_create<extra_data_type>();
            extra_data_wrapper::assign(*data, L, 2);
         }
         self.after_edit();
         return 0;
      }
   };
}

namespace {
   namespace _getters {
      int ignored_by_sandbox(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* edl  = cls::unwrap(self);
         if (!edl)
            return 0;
         auto* data = edl->get<extra_data_types::ignored_by_sandbox>();
         lua_pushboolean(L, !!data);
         return 1;
      }
   }
   namespace _setters {
      int ignored_by_sandbox(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* edl  = cls::unwrap(self);
         if (!edl)
            cobb::lua::error(L, "form-extra-data-list wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");

         self.before_edit();
         if (lua_toboolean(L, 2)) {
            edl->get_or_create<extra_data_types::ignored_by_sandbox>();
         } else {
            edl->remove<extra_data_types::ignored_by_sandbox>(*form);
         }
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "attach_ref",            &meta_extra_data_ops<extra_data_types::attach_ref>::getter },
      { "charge",                &meta_extra_data_ops<extra_data_types::charge>::getter },
      { "collision_layer_uid",   &meta_extra_data_ops<extra_data_types::collision_data>::getter },
      { "count",                 &meta_extra_data_ops<extra_data_types::count>::getter },
      { "emittance_source",      &meta_extra_data_ops<extra_data_types::emittance_source>::getter },
      { "encounter_zone",        &meta_extra_data_ops<extra_data_types::encounter_zone>::getter },
      { "favor_cost",            &meta_extra_data_ops<extra_data_types::favor_cost>::getter },
      { "global",                &meta_extra_data_ops<extra_data_types::global>::getter },
      { "headtracking_weight",   &meta_extra_data_ops<extra_data_types::headtracking_weight>::getter },
      { "health",                &meta_extra_data_ops<extra_data_types::health>::getter },
      { "health_percent",        &meta_extra_data_ops<extra_data_types::health_percent>::getter },
      { "horse",                 &meta_extra_data_ops<extra_data_types::horse>::getter },
      { "ignored_by_sandbox",    &_getters::ignored_by_sandbox },
      { "interior_lock_list",    &meta_extra_data_ops<extra_data_types::interior_lock_list>::getter },
      { "leveled_item_base",     &meta_extra_data_ops<extra_data_types::leveled_item_base>::getter },
      { "location",              &meta_extra_data_ops<extra_data_types::location>::getter },
      { "location_ref_type",     &meta_extra_data_ops<extra_data_types::location_ref_type>::getter },
      { "merchant_container",    &meta_extra_data_ops<extra_data_types::merchant_container>::getter },
      { "owner",                 &meta_extra_data_ops<extra_data_types::ownership>::getter },
      { "primitive",             &meta_extra_wrapper_ops<form_extra_data_types::primitive>::getter },
      { "radius",                &meta_extra_data_ops<extra_data_types::radius>::getter },
      { "scale",                 &meta_extra_data_ops<extra_data_types::scale>::getter },
      { "spawn_container",       &meta_extra_data_ops<extra_data_types::spawn_container>::getter },
      { "time_left",             &meta_extra_data_ops<extra_data_types::time_left>::getter },
      { "water_environment_map", &meta_extra_data_ops<extra_data_types::water_environment_map>::getter },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "attach_ref",            &meta_extra_data_ops<extra_data_types::attach_ref>::setter },
      { "charge",                &meta_extra_data_ops<extra_data_types::charge>::setter },
      { "collision_layer_uid",   &meta_extra_data_ops<extra_data_types::collision_data>::setter },
      { "count",                 &meta_extra_data_ops<extra_data_types::count>::setter },
      { "emittance_source",      &meta_extra_data_ops<extra_data_types::emittance_source>::setter },
      { "encounter_zone",        &meta_extra_data_ops<extra_data_types::encounter_zone>::getter },
      { "favor_cost",            &meta_extra_data_ops<extra_data_types::favor_cost>::setter },
      { "global",                &meta_extra_data_ops<extra_data_types::global>::setter },
      { "headtracking_weight",   &meta_extra_data_ops<extra_data_types::headtracking_weight>::setter },
      { "health",                &meta_extra_data_ops<extra_data_types::health>::setter },
      { "health_percent",        &meta_extra_data_ops<extra_data_types::health_percent>::setter },
      { "horse",                 &meta_extra_data_ops<extra_data_types::horse>::setter },
      { "ignored_by_sandbox",    &_setters::ignored_by_sandbox },
      { "interior_lock_list",    &meta_extra_data_ops<extra_data_types::interior_lock_list>::setter },
      { "leveled_item_base",     &meta_extra_data_ops<extra_data_types::leveled_item_base>::setter },
      { "location",              &meta_extra_data_ops<extra_data_types::location>::setter },
      { "location_ref_type",     &meta_extra_data_ops<extra_data_types::location_ref_type>::setter },
      { "merchant_container",    &meta_extra_data_ops<extra_data_types::merchant_container>::setter },
      { "owner",                 &meta_extra_data_ops<extra_data_types::ownership>::setter },
      { "primitive",             &meta_extra_wrapper_ops<form_extra_data_types::primitive>::setter },
      { "radius",                &meta_extra_data_ops<extra_data_types::radius>::setter },
      { "scale",                 &meta_extra_data_ops<extra_data_types::scale>::setter },
      { "spawn_container",       &meta_extra_data_ops<extra_data_types::spawn_container>::setter },
      { "time_left",             &meta_extra_data_ops<extra_data_types::time_left>::setter },
      { "water_environment_map", &meta_extra_data_ops<extra_data_types::water_environment_map>::setter },
   };
}