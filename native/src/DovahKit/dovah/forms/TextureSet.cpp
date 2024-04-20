#include "TextureSet.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   TextureSet::~TextureSet() {
      if (auto*& p = this->decal_data) {
         delete p;
         p = nullptr;
      }
   }

   void TextureSet::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'TX00':
               subrecord.read(this->textures.diffuse);
               break;
            case 'TX01':
               subrecord.read(this->textures.normal);
               break;
            case 'TX02':
               subrecord.read(this->textures.environment_mask);
               break;
            case 'TX03':
               subrecord.read(this->textures.glow_map);
               break;
            case 'TX04':
               subrecord.read(this->textures.height);
               break;
            case 'TX05':
               subrecord.read(this->textures.cubemap);
               break;
            case 'TX06':
               subrecord.read(this->textures.multilayer);
               break;
            case 'TX07':
               subrecord.read(this->textures.backlight);
               break;
            case 'DNAM':
               subrecord.read(this->texture_flags);
               break;
            case 'DODT':
               if (!this->decal_data)
                  this->decal_data = new components::decal_data;
               this->decal_data->load(subrecord, intfc);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void TextureSet::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'TX00':
               break;
            case 'TX01':
               break;
            case 'TX02':
               break;
            case 'TX03':
               break;
            case 'TX04':
               break;
            case 'TX05':
               break;
            case 'TX06':
               break;
            case 'TX07':
               break;
            case 'DNAM':
               break;
            case 'DODT':
               break;
            default:
               break;
         }
      }
   }
   void TextureSet::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto* copy = (TextureSet*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->textures.diffuse     = this->textures.diffuse;
      copy->textures.normal      = this->textures.normal;
      copy->textures.environment_mask = this->textures.environment_mask;
      copy->textures.glow_map    = this->textures.glow_map;
      copy->textures.height      = this->textures.height;
      copy->textures.cubemap     = this->textures.cubemap;
      copy->textures.multilayer  = this->textures.multilayer;
      copy->textures.backlight   = this->textures.backlight;
      copy->texture_flags = this->texture_flags;
      //
      if (auto* p = this->decal_data) {
         copy->decal_data = new components::decal_data;
         *copy->decal_data = *p;
      } else {
         if (auto* q = copy->decal_data)
            delete q;
         copy->decal_data = nullptr;
      }
      copy->bounds = this->bounds;
   }
   bool TextureSet::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (!this->bounds.is_zero()) {
         auto& OBND = record.open_next_subrecord('OBND');
         this->bounds.save(OBND, intfc);
         OBND.close();
      }
      if (!this->textures.diffuse.empty())
         record.write_string_subrecord('TX00', this->textures.diffuse);
      if (!this->textures.normal.empty())
         record.write_string_subrecord('TX01', this->textures.normal);
      if (!this->textures.environment_mask.empty())
         record.write_string_subrecord('TX02', this->textures.environment_mask);
      if (!this->textures.glow_map.empty())
         record.write_string_subrecord('TX03', this->textures.glow_map);
      if (!this->textures.height.empty())
         record.write_string_subrecord('TX04', this->textures.height);
      if (!this->textures.cubemap.empty())
         record.write_string_subrecord('TX05', this->textures.cubemap);
      if (!this->textures.multilayer.empty())
         record.write_string_subrecord('TX06', this->textures.multilayer);
      if (!this->textures.backlight.empty())
         record.write_string_subrecord('TX07', this->textures.backlight);
      //
      auto& DNAM = record.open_next_subrecord('DNAM');
      DNAM.write(this->texture_flags);
      DNAM.close();
      //
      if (auto* data = this->decal_data) {
         auto& DODT = record.open_next_subrecord('DODT');
         data->save(DODT, intfc);
         DODT.close();
      }
      //
      return true;
   }
   void TextureSet::_clear_impl() noexcept {
      this->textures.diffuse.clear();
      this->textures.normal.clear();   // or gloss
      this->textures.environment_mask.clear(); // or subsurface tint
      this->textures.glow_map.clear();   // or glow map
      this->textures.height.clear();
      this->textures.cubemap.clear();
      this->textures.multilayer.clear();
      this->textures.backlight.clear(); // or backlight mask
      this->texture_flags = 0;
      //
      if (auto*& p = this->decal_data) {
         delete p;
         p = nullptr;
      }
      this->bounds = components::object_bounds();
      this->script_data.clear(*this);
   }
   void TextureSet::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}