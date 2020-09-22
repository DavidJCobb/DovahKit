#include "keyword_list.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void keyword_list::load(tes_subrecord_reader& subrecord) {
      uint32_t  keywordSize = 0;
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'KSIZ':
            if (subrecord.read(keywordSize))
               this->forms.reserve(keywordSize);
            break;
         case 'KWDA':
            if (!keywordSize)
               keywordSize = subrecord.size() / 4;
            for (uint32_t i = 0; i < keywordSize; i++)
               if (subrecord.read(formID))
                  this->forms.push_back(formID);
            break;
      }
   }
   /*static*/ void keyword_list::generateUseInfo(tes_subrecord_reader& subrecord, form_stub* stub) {
      uint32_t  keywordSize = 0;
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'KSIZ':
            break;
         case 'KWDA':
            keywordSize = subrecord.size() / 4;
            for (uint32_t i = 0; i < keywordSize; i++)
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
            break;
      }
   }
   void keyword_list::save(tes_record_writer& record) {
      uint32_t size = this->forms.size();
      if (!size)
         return;
      auto& KSIZ = record.open_next_subrecord('KSIZ');
      KSIZ.write(size);
      KSIZ.close();
      auto& KWDA = record.open_next_subrecord('KWDA');
      for (auto& k : this->forms)
         KWDA.write(k);
      KWDA.close();
   }
}