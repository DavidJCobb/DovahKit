
#define FAIL_UNLESS_TABLE_LIKE(L, pos) \
   switch (lua_type((L), (pos))) { \
      case LUA_TTABLE: \
      case LUA_TUSERDATA: \
         break; \
      default: \
         FAIL("table or userdata expected"); \
   }