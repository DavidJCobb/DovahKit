#include "TESPluginHeader.h"
#include <filesystem>
#include "../output.h"

namespace {
   template<typename T> bool _read(FILE* file, T& field) {
      if (fread(&field, sizeof(T), 1, file) != 1)
         return false;
      return true;
   }
   void _skip(FILE* file, uint32_t byteCount) {
      fseek(file, byteCount, SEEK_CUR);
   }
   bool _check(FILE* file) {
      if (feof(file) || ferror(file))
         return false;
      return true;
   }
   //
   struct _subrecord {
      FILE*    file;
      uint32_t signature = 0;
      uint16_t size;
      uint32_t pos;
      //
      _subrecord(FILE* f) : file(f) {}
      //
      bool open() {
         if (this->signature) {
            this->skip();
         }
         auto f = this->file;
         _read(f, this->signature);
         this->signature = _byteswap_ulong(this->signature);
         _read(f, this->size);
         this->pos = ftell(f);
         return this->signature && _check(f);
      }
      void skip() {
         this->signature = 0;
         auto f   = this->file;
         auto pos = ftell(f);
         uint32_t offset = pos - this->pos;
         if (offset >= this->size)
            return;
         _skip(f, this->size - offset);
      }
      bool to_string(std::string& field) const {
         field.clear();
         auto length = this->size;
         field.resize(length);
         auto target = const_cast<char*>(field.data());
         if (field[length - 1] == '\0') // C++ std::strings + direct reading + null terminators = horrible, horrible mess
            field.resize(length - 1);
         if (fread(target, length, 1, file) != 1)
            return false;
         return true;
      }
   };
}

bool TESPluginHeader::load(const char* path) {
   FILE* file;
   fopen_s(&file, path, "rb");
   if (!file) {
      _DEBUGMSG("[TESPluginHeader] ERROR: Failed to open: %s", path);
      return false;
   }
   this->name = std::filesystem::path(path).filename().string();
   uint32_t recordSize;
   {
      uint32_t signature;
      if (!_read(file, signature))
         return false;
      if (_byteswap_ulong(signature) != 'TES4')
         return false;
   }
   if (!_read(file, recordSize))
      return false;
   if (!_read(file, this->flags))
      return false;
   _skip(file, 8); // form ID of TES4 record; version control bytes
   uint16_t recordVersion;
   if (!_read(file, recordVersion))
      return false;
   _skip(file, 2); // unknown field

   //
   // TODO: Force ESM flag if the file is *.esm.
   //

   //
   uint32_t   last_subrecord = 0;
   _subrecord subrecord(file);
   while (subrecord.open()) {
      switch (subrecord.signature) {
         case 'CNAM': // creator
            subrecord.to_string(this->authorName);
            break;
         case 'SNAM': // description
            subrecord.to_string(this->description);
            break;
         case 'MAST':
            if (last_subrecord && last_subrecord != 'DATA') {
               _DEBUGMSG("[TESPluginHeader] Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
            }
            {
               std::string out;
               subrecord.to_string(out);
               this->masters.push_back(out);
            }
            break;
         case 'DATA':
            if (last_subrecord != 'MAST') {
               if (last_subrecord)
                  _DEBUGMSG("[TESPluginHeader] Error: Unexpected 'DATA' subrecord in the file header following %s.", FMT_SIGNATURE(last_subrecord));
               else
                  _DEBUGMSG("[TESPluginHeader] Error: Unexpected 'DATA' subrecord at the start of the file header.");
               return false;
            }
            break;
      }
      last_subrecord = subrecord.signature;
   }
   fclose(file);
   return true;
}