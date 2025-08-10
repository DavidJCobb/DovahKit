#include "LensFlare.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void LensFlare::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         bool stop = false;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'CNAM':
               subrecord.read(this->color_influence);
               break;
            case 'DNAM':
               subrecord.read(this->fade_distance_radius_scale);
               break;
            case 'LFSP':
               {
                  uint32_t count = 0;
                  this->sprites.resize(count);
                  record.next_subrecord();
                  for (uint32_t i = 0; i < count; ++i) {
                     if (!record.get_current_subrecord()) {
                        static_assert(false, "TODO: Warn");
                        break;
                     }
                     auto& item = this->sprites[i];
                     if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'DNAM') {
                        subrecord.read(item.id);
                        record.next_subrecord();
                     }
                     if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'FNAM') {
                        subrecord.read(item.texture);
                        record.next_subrecord();
                     }
                     if (auto& subrecord = record.get_current_subrecord(); subrecord.signature() == 'LFSD') {
                        subrecord.read(item.data.tint.r);
                        subrecord.read(item.data.tint.g);
                        subrecord.read(item.data.tint.b);
                        subrecord.read(item.data.width);
                        subrecord.read(item.data.height);
                        subrecord.read(item.data.position);
                        subrecord.read(item.data.angular_fade);
                        subrecord.read(item.data.opacity);
                        subrecord.read(item.data.flags);
                        record.next_subrecord();
                     }
                  }
               }
               stop = true;
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
         if (stop)
            break;
      }
   }
   /*static*/ void LensFlare::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void LensFlare::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (LensFlare*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->color_influence = this->color_influence;
      copy->fade_distance_radius_scale = this->fade_distance_radius_scale;
      copy->sprites = this->sprites;
   }
   void LensFlare::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->color_influence);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->fade_distance_radius_scale);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('LFSP');
         subrecord.write((uint32_t)this->sprites.size());
         subrecord.close();
      }
      for (auto& sprite : this->sprites) {
         if (!sprite.id.empty())
            record.write_string_subrecord('DNAM', sprite.id);
         if (!sprite.texture.empty())
            record.write_string_subrecord('FNAM', sprite.texture);
         {
            auto& subrecord = record.open_next_subrecord('LFSD');
            subrecord.write(sprite.data.tint.r);
            subrecord.write(sprite.data.tint.g);
            subrecord.write(sprite.data.tint.b);
            subrecord.write(sprite.data.width);
            subrecord.write(sprite.data.height);
            subrecord.write(sprite.data.position);
            subrecord.write(sprite.data.angular_fade);
            subrecord.write(sprite.data.opacity);
            subrecord.write(sprite.data.flags);
            subrecord.close();
         }
      }
   }
   void LensFlare::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->sprites.clear();
   }
   void LensFlare::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}