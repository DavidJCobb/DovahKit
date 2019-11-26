#include "TESPluginHeader.h"
#include <filesystem>
#include "LoadOrder.h"
#include "../output.h"
#include "../helpers/files.h"

namespace {
   template<typename T> bool _read(FILE* file, T& field) noexcept {
      if (fread(&field, sizeof(T), 1, file) != 1)
         return false;
      return true;
   }
   void _skip(FILE* file, uint32_t byteCount) noexcept {
      fseek(file, byteCount, SEEK_CUR);
   }
   bool _check(FILE* file) noexcept {
      if (feof(file) || ferror(file))
         return false;
      return true;
   }
   //
   struct _subrecord {
      FILE*    file;
      uint32_t signature = 0;
      uint16_t size;
      uint32_t pos; // subrecord position within the file
      uint32_t record_end = 0; // position of containing record's end, relative to start of file
      //
      _subrecord(FILE* f, uint32_t re) : file(f), record_end(re) {}
      //
      bool open() noexcept {
         if (this->signature) {
            this->skip();
         }
         if (ftell(this->file) >= this->record_end)
            return false;
         auto f = this->file;
         _read(f, this->signature);
         this->signature = _byteswap_ulong(this->signature);
         _read(f, this->size);
         this->pos = ftell(f);
         return this->signature && _check(f);
      }
      void skip() noexcept {
         this->signature = 0;
         auto f   = this->file;
         auto pos = ftell(f);
         uint32_t offset = pos - this->pos;
         if (offset >= this->size)
            return;
         _skip(f, this->size - offset);
      }
      bool to_string(std::string& field) const noexcept {
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

bool TESPluginHeader::load(const char* path) noexcept {
   FILE*   file;
   errno_t err = fopen_s(&file, path, "rb");
   if (!file) {
      LoadOrder::get().logError([&path, err](FatalLoadError& error) {
         error.file       = std::filesystem::path(path).filename().string();
         error.parseError = "Failed to parse the file header. ";
         error.parseError += FILE_ERROR_CODE_TO_STRING(err);
         switch (err) {
            case ENFILE:
            case EMFILE:
            case EINVAL:
            case ELOOP:
            case ENAMETOOLONG:
               error.code = LoadErrorCode::filesystem_error;
               break;
            case EACCES:
            case EBUSY:
               error.code = LoadErrorCode::locked_file;
               break;
            case ENOENT:
            default:
               error.code = LoadErrorCode::missing_file;
         }
      });
      return false;
   }
   cobb::file_guard guard(file); // calls fclose for us
   //
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
   if (this->name.size() > 4) {  // Force flags based on file extension.
      const char* extension = this->name.data() + this->name.size() - 4;
      if (_strnicmp(".esm", extension, 4) == 0) {
         this->flags |= Flags::master;
      } else if (_strnicmp(".esl", extension, 4) == 0) {
         this->flags |= Flags::master | Flags::light;
      }
   }
   //
   uint32_t   last_subrecord = 0;
   _subrecord subrecord(file, ftell(file) + recordSize);
   while (subrecord.open()) {
      switch (subrecord.signature) {
         case 'CNAM': // creator
            subrecord.to_string(this->authorName);
            break;
         case 'SNAM': // description
            subrecord.to_string(this->description);
            break;
         case 'MAST':
            if (last_subrecord == 'MAST') {
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
               LoadOrder::get().logError([this, &path, file, last_subrecord](FatalLoadError& error) {
                  error.code = LoadErrorCode::malformed_file;
                  error.file = std::filesystem::path(path).filename().string();
                  error.fileOffset = ftell(file);
                  if (last_subrecord) {
                     error.parseError = "Failed initial read of the file header. Unexpected 'DATA' subrecord in the file following another master.";
                     error.dependency = *this->masters.rbegin();
                  } else
                     error.parseError = "Failed initial read of the file header. Unexpected 'DATA' subrecord at the start of the file header.";
               });
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
   return true;
}