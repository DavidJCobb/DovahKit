#include "./head_part.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/HeadPart.h"
#include "dovah/core.h"

namespace {
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void head_part::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
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
                     this->sex = {};
                  } else if (female) {
                     this->sex = dovah::sex::female;
                  } else {
                     this->sex = dovah::sex::male;
                  }
               }
            }
            break;
         case 'PNAM':
            {
               head_part_type v;
               if (subrecord.read(v)) {
                  this->type = (std::decay_t<decltype(this->type)>)v; // uh, okay, MSVC. sure.
               }
            }
            break;
         case 'RNAM':
            this->_read_form_stub_from_subrecord(this->race_list, subrecord, dovah::form_type::formlist);
            break;
      }
   }
   bool head_part::sever_outbound_references_to(const dovah::form_stub* target) {
      if (this->race_list == target) {
         this->race_list = nullptr;
         return true;
      }
      return false;
   }
   bool head_part::update(const dovah::loaded_forms::HeadPart& src) {
      using loaded_form_type = dovah::loaded_forms::HeadPart;

      const auto prior = *this;

      auto flags = src.flags;
      this->is_extra    = flags & loaded_form_type::head_part_flag::is_extra_part;
      this->is_playable = flags & loaded_form_type::head_part_flag::playable;
      //
      bool female = flags & loaded_form_type::head_part_flag::female;
      bool male   = flags & loaded_form_type::head_part_flag::male;
      if (female == male) {
         this->sex = {};
      } else if (female) {
         this->sex = dovah::sex::female;
      } else {
         this->sex = dovah::sex::male;
      }

      this->type = (std::decay_t<decltype(this->type)>)src.type; // uh, okay, MSVC. sure.

      this->_update_form_stub_from_loaded(this->race_list, src.valid_races, dovah::form_type::formlist);

      return prior == *this;
   }
}