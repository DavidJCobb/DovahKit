
#define TRY_ASSIGN_FIELD(_subject, _field_native_name, _field_lua_name, L, pos, _pull) \
   lua_getfield((L), (pos), _field_lua_name); \
   if (lua_isnoneornil(L, -1)) { \
      lua_pop((L), 1); \
   } else { \
      auto result = _pull(L, -1); \
      lua_pop((L), 1); \
      if (result.has_value()) \
         _subject._field_native_name = result.value(); \
      else \
         FAIL(result.error()); \
   }

#define TRY_ASSIGN_FIELD_WITH_DIAGNOSTIC(_subject, _field_native_name, _field_lua_name, L, pos, _pull) \
   lua_getfield((L), (pos), _field_lua_name); \
   if (lua_isnoneornil(L, -1)) { \
      lua_pop((L), 1); \
   } else { \
      auto result = _pull(L, -1); \
      lua_pop((L), 1); \
      if (result.has_value()) \
         _subject._field_native_name = result.value(); \
      else \
         FAIL(std::format("problem with `" _field_lua_name "`: {}", result.error())); \
   }
