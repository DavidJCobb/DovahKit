#pragma once
#include "./_base.h"
#include "../_base_macros.define.h"

namespace dovah::loaded_forms {
   class ActorBase;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class actor_base : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::actor_base };
         static constexpr const auto form_types_we_refer_to = std::array{ dovah::form_type::voicetype };
         static constexpr const auto subrecords_of_interest = std::array{ 'ACBS', 'VTCK' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         constexpr actor_base() {}

         dovah::form_stub* voicetype = nullptr;
         bool female     = false;
         bool summonable = false;
         bool unique     = false;

      public:
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool sever_outbound_references_to(const dovah::form_stub*);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::ActorBase&);
   };
}

#include "../_base_macros.undef.h"