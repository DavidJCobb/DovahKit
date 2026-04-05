#include "./topic_info_fragment_data.h"
#include "../../../_common_cpp.h"

#include "../../../../notices/form_load_warnings/by_form_component/papyrus/inconsistent_fragment_scriptname.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::papyrus;
   }
}

namespace dovah::loaded_forms::components::papyrus {
   void topic_info_fragment_data::load(attachment_data& owner, tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint8_t flags = 0;

      if (subrecord.is_in_bounds(2)) {
         subrecord.unchecked_read(this->unknown);
         subrecord.unchecked_read(flags);
      }
      subrecord.read_length_prefixed_string<2>(this->filename);
      if (flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->fragments.on_begin.emplace();
         if (subrecord.read(frag.unknown)) {
            if (subrecord.read_length_prefixed_string<2>(frag.script)) {
               subrecord.read_length_prefixed_string<2>(frag.function);
               if (frag.script != this->filename) {
                  specific_load_warnings::inconsistent_fragment_scriptname notice(
                     const_cast<form_stub&>(intfc.target_stub),
                     specific_load_warnings::inconsistent_fragment_scriptname::fragment_type::on_begin,
                     this->filename,
                     frag.script
                  );
                  intfc.log_load_warning(notice);
               }
            }
         }
      }
      if (flags & fragment_flag::has_end_fragment) {
         auto& frag = this->fragments.on_end.emplace();
         if (subrecord.read(frag.unknown)) {
            if (subrecord.read_length_prefixed_string<2>(frag.script)) {
               subrecord.read_length_prefixed_string<2>(frag.function);
               if (frag.script != this->filename) {
                  specific_load_warnings::inconsistent_fragment_scriptname notice(
                     const_cast<form_stub&>(intfc.target_stub),
                     specific_load_warnings::inconsistent_fragment_scriptname::fragment_type::on_end,
                     this->filename,
                     frag.script
                  );
                  intfc.log_load_warning(notice);
               }
            }
         }
      }
   }
   void topic_info_fragment_data::save(attachment_data& owner, tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      uint8_t flags = 0;
      if (this->fragments.on_begin.has_value()) {
         flags |= fragment_flag::has_begin_fragment;
      }
      if (this->fragments.on_end.has_value()) {
         flags |= fragment_flag::has_end_fragment;
      }

      subrecord.write(this->unknown);
      subrecord.write(flags);
      subrecord.write_length_prefixed_string<2>(this->filename);
      if (auto& frag_opt = this->fragments.on_begin; frag_opt.has_value()) {
         auto& frag = frag_opt.value();
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
      if (auto& frag_opt = this->fragments.on_end; frag_opt.has_value()) {
         auto& frag = frag_opt.value();
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   fragment_data_base* topic_info_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new topic_info_fragment_data;
      copy->unknown   = this->unknown;
      copy->filename  = this->filename;
      copy->fragments = this->fragments;
      return copy;
   }
}