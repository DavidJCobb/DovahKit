#pragma push_macro("HEADER")
#pragma push_macro("IMPLEMENTATION")
#undef HEADER
#undef IMPLEMENTATION

#define HEADER(_name) \
   namespace dovahscript::wrappers { \
      struct _name : public _placed_projectile { \
         static constexpr string_list_t superclass_list = { metatable_key }; \
         static constexpr const char* metatable_key = "dovah.classes." #_name; \
         static constexpr const char* class_name = #_name ; \
         static method_list_t metatable_methods; \
         static method_list_t metatable_getters; \
         static method_list_t metatable_setters; \
      }; \
   }

#define IMPLEMENTATION(_name) \
   namespace { \
      using cls = dovahscript::wrappers:: _name ; \
   } \
   namespace dovahscript::wrappers { \
      /*static*/ cls::method_list_t cls::metatable_methods = no_functions; \
      /*static*/ cls::method_list_t cls::metatable_getters = no_functions; \
      /*static*/ cls::method_list_t cls::metatable_setters = no_functions; \
   }
