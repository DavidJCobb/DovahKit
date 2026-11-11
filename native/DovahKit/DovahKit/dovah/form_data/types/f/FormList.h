#pragma once
#include "dovah/types/managed_form_use.h"
#include "../_base.h"

#define DOVAH_FORM_TYPE      formlist // `dovah::form_type` enumeration member
#define DOVAH_FORM_CLASSNAME FormList // identifier in `dovah::form_data` namespace

namespace dovah::form_data {
   template<form_data_params Params>
   struct FormList : public _base<Params> {
      #include "dovah/form_data/fragments/pull_type_aliases.inl"
      
      // TODO: Papyrus VMAD
      vector<form_use> contents; // LNAM[]

      #define CLASSNAME DOVAH_FORM_CLASSNAME
      #define TYPENAME  CLASSNAME<FORM_DATA_PARAMS>
      #define PER_FIELD(X) \
         X(contents)
      #include "dovah/form_data/fragments/enable_construction_via_table.inl"
      #include "dovah/form_data/fragments/define_visitors.inl"
   };
}

#include "dovah/form_data/fragments/declare_tabled_load_functions.inl"
#include "dovah/form_data/fragments/define_form_type_constant.inl"

#undef DOVAH_FORM_CLASSNAME
#undef DOVAH_FORM_TYPE