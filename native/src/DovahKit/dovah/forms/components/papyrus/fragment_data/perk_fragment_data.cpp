#include "./perk_fragment_data.h"
#include "../../../_common_cpp.h"

#include "../../../../notices/form_save_errors/by_form_component/papyrus/too_many_perk_fragments.h"

namespace {
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::papyrus;
   }
}

namespace dovah::loaded_forms::components::papyrus {
   void perk_fragment_data::load(attachment_data& owner, tes_subrecord_reader& subrecord) {
      if (!subrecord.read(this->unknown))
         return;
      if (!subrecord.read_length_prefixed_string<2>(this->filename))
         return;

      fragment_count_serialized_type count = 0;
      if (!subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; i++) {
         if (!subrecord.is_in_bounds(5))
            break;
         auto& frag = this->fragments.emplace_back();
         subrecord.unchecked_read(frag.index);
         subrecord.unchecked_read(frag.unknown02);
         subrecord.unchecked_read(frag.unknown04);
         if (!subrecord.read_length_prefixed_string<2>(frag.filename))
            break;
         if (!subrecord.read_length_prefixed_string<2>(frag.function))
            break;
      }
   }
   void perk_fragment_data::save(attachment_data& owner, tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->unknown);
      subrecord.write_length_prefixed_string<2>(this->filename);
      if (this->fragments.size() > max_fragment_count) {
         auto notice = specific_save_errors::too_many_perk_fragments(
            *intfc.target_stub,
            this->fragments.size()
         );
         intfc.throw_save_error(notice);
         return;
      }
      subrecord.write((fragment_count_serialized_type)this->fragments.size());
      for (auto& frag : this->fragments) {
         subrecord.write(frag.index);
         subrecord.write(frag.unknown02);
         subrecord.write(frag.unknown04);
         subrecord.write_length_prefixed_string<2>(frag.filename);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   fragment_data_base* perk_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new perk_fragment_data;

      copy->unknown  = this->unknown;
      copy->filename = this->filename;

      size_t size = this->fragments.size();
      copy->fragments.resize(size);
      for (size_t i = 0; i < size; ++i) {
         copy->fragments[i] = this->fragments[i];
      }

      return copy;
   }
}