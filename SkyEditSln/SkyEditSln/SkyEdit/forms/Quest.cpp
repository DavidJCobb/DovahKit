#include "Quest.h"
#include "../esp/TESPlugin.h"

void TESQuest::load(esp_istream& file) {
   while (file.nextSubrecord()) {
      uint32_t signature = file.getSubrecordType();
      uint32_t size      = file.getSubrecordSize();
      switch (signature) {
         case 'EDID':
            file.read_string_subrecord(this->editorID);
            break;
         case 'FULL':
            //
            // TODO: The file header indicates whether so-called "lstrings" are 
            // localized. If so, then this subrecord's value is an index in a 
            // string table held in another file. if not, then this subrecord's 
            // value is a string.
            //
            // Currently, we only handle the latter case, which breaks for 
            // Skyrim.esm and friends.
            //
            file.read_string_subrecord(this->name);
            break;
         //
         // TODO: others
         //
      }
   }
}