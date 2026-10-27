// preconditions:
//    macro DOVAH_FORM_TYPE      defined as a `form_type` constant
//    macro DOVAH_FORM_CLASSNAME defined as the name of the class template within `dovah::form_data`
namespace dovah {
   template<typename T>
   struct form_type_for_form_data_class;

   template<form_data_params Params>
   struct form_type_for_form_data_class<form_data::DOVAH_FORM_CLASSNAME<Params>> {
      static constexpr const form_type value = DOVAH_FORM_TYPE;
   };
}
#undef DOVAH_FORM_CLASSNAME
#undef DOVAH_FORM_TYPE