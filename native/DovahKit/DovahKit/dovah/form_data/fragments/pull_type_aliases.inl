//
// Include this file inline within a form-data type definition.
// (Provided because subclasses don't inherit type aliases.)
// 
// Requires that the params the struct is templated on have the 
// identifier `Params`.
//

#pragma push_macro("X")
#undef  X
#define X(_typename_) using _typename_ = form_data_helpers::type_aliases<Params>::_typename_;
X(form_use)
X(localized_general_string_use)
X(localized_description_string_use)
X(localized_info_string_use)
#undef  X
#pragma pop_macro("X")

template<auto UseInfoEntryFlag>
using unique_form_use = form_data_helpers::type_aliases<Params>::template unique_form_use<UseInfoEntryFlag>;