#include "model.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void model::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            subrecord.to_string(this->modelPath);
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            {
               auto s = subrecord.size();
               this->textureHashes.data.resize(s);
               for (uint32_t i = 0; i < s; i++)
                  subrecord.read(this->textureHashes.data[i]);
            }
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     auto& entry = this->textureSwaps.emplace_back();
                     subrecord.read_length_prefixed_string<4>(entry.nifBlockName);
                     subrecord.read(entry.textureSet);
                     subrecord.read(entry.nifBlockIndex);
                     if (!subrecord.is_in_bounds())
                        break;
                  }
               }
            }
            break;
      }
   }
   /*static*/ void model::generateUseInfo(tes_subrecord_reader& subrecord, form_stub* stub) {
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     subrecord.skip_length_prefixed_string<4>();
                     if (subrecord.read(formID))
                        stub->add_outbound_reference(formID);
                     subrecord.skip_bytes(4);
                     if (!subrecord.is_in_bounds())
                        return;
                  }
               }
            }
            break;
      }
   }
   void model::save(tes_subrecord_writer& subrecord) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            subrecord.write(this->modelPath);
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            for (auto& byte : this->textureHashes.data) {
               subrecord.write(byte);
            }
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            subrecord.write(uint32_t(this->textureSwaps.size()));
            for (auto& entry : this->textureSwaps) {
               subrecord.write_length_prefixed_string<4>(entry.nifBlockName);
               subrecord.write(entry.textureSet);
               subrecord.write(entry.nifBlockIndex);
            }
            break;
      }
   }
   void model::save(tes_record_writer& record, uint32_t signature_path, uint32_t signature_hash, uint32_t signature_swap) {
      if (!this->modelPath.empty())
         this->save(record.open_next_subrecord(signature_path));
      if (this->has_texture_hashes())
         this->save(record.open_next_subrecord(signature_hash));
      if (!this->textureSwaps.empty())
         this->save(record.open_next_subrecord(signature_swap));
   }
   void model::clear(form_stub& my_owner) {
      this->modelPath.clear();
      this->textureHashes.data.clear();
      for (auto& entry : this->textureSwaps)
         entry.textureSet.set(&my_owner, bare_form_id_t(0));
   }
   void model::clone_from(const model& other, form_stub& my_owner) noexcept {
      this->modelPath     = other.modelPath;
      this->textureHashes = other.textureHashes;
      //
      size_t size = other.textureSwaps.size();
      if (!this->textureSwaps.empty()) {
         for (auto& entry : this->textureSwaps)
            entry.textureSet.set(&my_owner, bare_form_id_t(0));
         this->textureSwaps.clear();
      }
      this->textureSwaps.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& entry = this->textureSwaps[i];
         auto& from  = other.textureSwaps[i];
         entry.nifBlockName  = from.nifBlockName;
         entry.nifBlockIndex = from.nifBlockIndex;
         entry.textureSet.set(&my_owner, from.textureSet);
      }
   }
   void model::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      bare_form_id_t formID = target.formID;
      for (auto& entry : this->textureSwaps)
         if (entry.textureSet == formID)
            entry.textureSet.set(&my_owner, nullptr);
   }
   void model::get_outbound_formIDs(std::vector<form_id_t*>& out) const noexcept {
      for (auto& entry : this->textureSwaps)
         out.push_back(const_cast<form_id_t*>(&entry.textureSet));
   }
}