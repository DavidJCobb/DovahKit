#define MAKE_FORM_INFO_CACHE_DATA_TEMPLATES \
   static constexpr const bool form_type_is_of_interest(dovah::form_type ft) { \
      for (auto v : form_types_of_interest) \
         if (v == ft) \
            return true; \
      return false; \
   } \
   \
   static constexpr const bool form_type_is_referred_to(dovah::form_type ft) { \
      for (auto v : form_types_we_refer_to) \
         if (v == ft) \
            return true; \
      return false; \
   } \
   \
   template<typename LoadedFormClass> \
   static constexpr const bool form_class_is_of_interest = form_type_is_of_interest(LoadedFormClass::form_type);