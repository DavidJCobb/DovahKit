#include "./cached_head_part_info.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/HeadPart.h"
#include "dovah/core.h"

namespace {
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

namespace dovahkit::subsystems::form_info_cache {
   void cached_head_part_info::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      using loaded_form_type = dovah::loaded_forms::HeadPart;

      switch (subrecord.signature()) {
         case 'DATA':
            {
               uint8_t flags = 0;
               if (subrecord.read(flags)) {
                  this->is_extra    = flags & loaded_form_type::head_part_flag::is_extra_part;
                  this->is_playable = flags & loaded_form_type::head_part_flag::playable;

                  bool female = flags & loaded_form_type::head_part_flag::female;
                  bool male   = flags & loaded_form_type::head_part_flag::male;
                  if (female == male) {
                     this->sex = sex::any;
                  } else if (female) {
                     this->sex = sex::female;
                  } else {
                     this->sex = sex::male;
                  }
               }
            }
            break;
         case 'PNAM':
            {
               loaded_form_type::head_part_type v;
               if (subrecord.read(v)) {
                  this->type = (std::decay_t<decltype(this->type)>)v; // uh, okay, MSVC. sure.
               }
            }
            break;
         case 'RNAM':
            {
               dovah::form_reference_t ref;
               if (subrecord.read(ref)) {
                  auto* stub = this->race_list = ref.get_form_stub();
                  if (stub && stub->form_type != dovah::form_type::formlist)
                     this->race_list = nullptr;
               }
            }
            break;
      }
   }
   bool cached_head_part_info::sever_outbound_references_to(const dovah::form_stub* target) {
      if (this->race_list == target) {
         this->race_list = nullptr;
         return true;
      }
      return false;
   }
   bool cached_head_part_info::update(const dovah::loaded_forms::HeadPart& src) {
      using loaded_form_type = dovah::loaded_forms::HeadPart;

      const auto prior = *this;

      auto flags = src.flags;
      this->is_extra    = flags & loaded_form_type::head_part_flag::is_extra_part;
      this->is_playable = flags & loaded_form_type::head_part_flag::playable;
      //
      bool female = flags & loaded_form_type::head_part_flag::female;
      bool male   = flags & loaded_form_type::head_part_flag::male;
      if (female == male) {
         this->sex = sex::any;
      } else if (female) {
         this->sex = sex::female;
      } else {
         this->sex = sex::male;
      }

      this->type = (std::decay_t<decltype(this->type)>)src.type; // uh, okay, MSVC. sure.

      if (auto* stub = src.valid_races.get_form_stub(); stub && stub->form_type == dovah::form_type::formlist) {
         this->race_list = stub;
      } else {
         this->race_list = nullptr;
      }

      return prior == *this;
   }
}