#include "./scene_fragment_data.h"
#include "../../../_common_cpp.h"

#include "../../../../notices/form_save_errors/by_form_component/papyrus/too_many_scene_phase_fragments.h"

namespace {
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::papyrus;
   }
}

namespace dovah::loaded_forms::components::papyrus {
   void scene_fragment_data::load(attachment_data& owner, tes_subrecord_reader& subrecord) {
      uint8_t flags = 0;

      if (!subrecord.read(this->unknown))
         return;
      if (!subrecord.read(flags))
         return;
      if (!subrecord.read_length_prefixed_string<2>(this->filename))
         return;

      if (flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->fragments.on_begin.emplace();
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
      if (flags & fragment_flag::has_end_fragment) {
         auto& frag = this->fragments.on_end.emplace();
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }

      phase_fragment_count_serialized_type count = 0;
      if (!subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; i++) {
         if (!subrecord.is_in_bounds(6))
            break;
         auto& frag = this->fragments.on_phase.emplace_back();
         subrecord.unchecked_read(frag.flags);
         subrecord.unchecked_read(frag.phase);
         subrecord.unchecked_read(frag.unknown05);
         subrecord.read_length_prefixed_string<2>(frag.filename);
         subrecord.read_length_prefixed_string<2>(frag.function);
      }
   }
   void scene_fragment_data::save(attachment_data& owner, tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
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

      if (this->fragments.on_phase.size() > max_phase_fragment_count) {
         auto notice = specific_save_errors::too_many_scene_phase_fragments(
            *intfc.target_stub,
            this->fragments.on_phase.size()
         );
         intfc.throw_save_error(notice);
         return;
      }
      subrecord.write((phase_fragment_count_serialized_type)this->fragments.on_phase.size());
      for (auto& frag : this->fragments.on_phase) {
         subrecord.write(frag.flags);
         subrecord.write(frag.phase);
         subrecord.write(frag.unknown05);
         subrecord.write_length_prefixed_string<2>(frag.filename);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   fragment_data_base* scene_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new scene_fragment_data;

      copy->unknown   = this->unknown;
      copy->filename  = this->filename;
      copy->fragments = this->fragments;

      return copy;
   }
}