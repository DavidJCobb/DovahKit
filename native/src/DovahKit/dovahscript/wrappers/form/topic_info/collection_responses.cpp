#include "./collection_responses.h"
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"

#include "dovah/forms/TopicInfo.h"
#include "../topic_info.h"
#include "./response.h"

#include "dovah/forms/TopicInfo.h"
#include "dovahscript/api_helpers/native_lists/member_function_spec.h"
#include "dovahscript/api_helpers/native_lists/common/pull_collection.h"
#include "dovahscript/api_helpers/native_lists/define_metatable.h"
#include "dovahscript/wrapper.h"

namespace {
   using namespace dovahscript;
   using containing_form_type = dovah::loaded_forms::TopicInfo;
   using wrapper_spec         = dovahscript::wrapper_likes::native_lists::topic_info_responses;

   struct member_function_spec : public api_helpers::native_lists::member_function_spec {
      using collection_wrapped_type = decltype(containing_form_type::responses);
      using value_wrapper_type      = wrappers::topic_info_response;
      using value_stored_type       = typename collection_wrapped_type::value_type;
      using value_working_type      = value_stored_type;

      static constexpr const bool allow_insertions_past_end = true;

      static constexpr const auto pull_collection = &api_helpers::native_lists::common::pull_collection<wrapper_spec::metatable_key>;

      static collection_wrapped_type* unwrap_collection(wrapper& self) {
         auto* form = self.get_loaded_form_data<containing_form_type>();
         if (!form)
            return nullptr;
         return &form->responses;
      }

      static void validate_value(lua_State* L, int pos, const dovah::loaded_forms::Form& form, const value_stored_type& dst) {
         value_wrapper_type::verify_table_can_overwrite(L, pos, form, dst);
      }
      static void validate_value(lua_State* L, int pos, const dovah::loaded_forms::Form& form) {
         value_wrapper_type::verify_table_for_insertion(L, pos, form);
      }
      static void overwrite_value(lua_State* L, int src_pos, value_stored_type& dst, dovah::loaded_forms::Form& dst_form) {
         value_wrapper_type::overwrite_with_table(dst_form, dst, L, src_pos);
      }
      static int push_value(lua_State* L, wrapper& collection, size_t zero_based_item_index) {
         assert(collection.is_collection);
         assert(collection.stub);
         wrapper out = collection;
         out.into_collection(zero_based_item_index);
         return core::subsystems::userdata::get().push(L, out, value_wrapper_type::metatable_key);
      }

      static std::vector<uint8_t> prepare_for_insertion(
         const collection_wrapped_type& dst_list,
         size_t count_to_insert,
         lua_State* L,
         std::optional<int> value_pos
      ) {
         std::optional<uint8_t> specified_uid;
         if (value_pos.has_value()) {
            lua_getfield(L, value_pos.value(), "unique_id");
            if (!lua_isnoneornil(L, -1)) {
               if (lua_isinteger(L, -1))
                  specified_uid = lua_tointeger(L, -1);
            }
            lua_pop(L, 1);
         }

         std::vector<uint8_t> result;
         size_t count_to_default = count_to_insert;
         if (specified_uid.has_value()) {
            --count_to_default;
            if (!count_to_default) { // fast path: inserting just one value
               result.push_back(specified_uid.value());
               return result;
            }
         }

         cobb::bitset<256> used_ids;
         for (const auto& item : dst_list)
            used_ids.set(item.id);
         if (specified_uid.has_value())
            used_ids.set(specified_uid.value());

         bool zero_is_taken = used_ids.test(0);
         used_ids.set(0);

         for (size_t i = 0; i < count_to_default; ++i) {
            auto id = used_ids.find_first_clear();
            if (id >= 0) {
               result.push_back(id);
               continue;
            }
            //
            // All good IDs are taken. Abort.
            //
            if (!zero_is_taken) {
               //
               // ...though if zero isn't taken, use that and then abort.
               //
               result.push_back(0);
            }
            break;
         }
         if (specified_uid.has_value())
            result.push_back(specified_uid.value());

         if (result.empty())
            cobb::lua::error(L, "this `topic_info` form has no more unique IDs left for new responses");
         if (result.size() < count_to_default)
            cobb::lua::error(L, "this `topic_info` form doesn't have enough unique IDs left for the %u new responses you are attempting to create", (int)count_to_insert);
         if (result.back() == 0)
            cobb::lua::warning(L, "one of the new responses will end up using unique ID 0; this ID is a sentinel value used when recording lines in the Creation Kit");
         return result;
      }
      static void apply_preparation(value_working_type& dst, uint8_t unique_id) {
         dst.id = unique_id;
      }
   };
}

namespace dovahscript::wrapper_likes::native_lists {
   /*static*/ void wrapper_spec::define_metatable(lua_State* L) {
      api_helpers::native_lists::define_metatable<metatable_key, class_name, member_function_spec>(L);
   }
   /*static*/ int wrapper_spec::push(lua_State* L, const wrapper& parent) {
      wrapper out = parent;
      out.append_part(signature);
      out.is_collection = true;
      return core::subsystems::userdata::get().push(L, out, metatable_key.data());
   }
}