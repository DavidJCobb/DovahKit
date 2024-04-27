#include "file_header.h"
#include <filesystem>
#include "../../../helpers/files.h"
#include "../../logging.h"

#include "../../exceptions/file_load_failed.h"
#include "../../notices/file_load_errors/filesystem_error.h"
#include "../../notices/file_load_errors/malformed_file_header.h"

#include <QDebug>

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
         if (fread(target, length, 1, this->file) != 1)
            return false;
         return true;
      }
      //
      template<typename T> bool read(T& out) const noexcept {
         uint32_t offset = ftell(this->file) - this->pos;
         if (this->size - offset < sizeof(T))
            return false;
         if (fread(&out, sizeof(T), 1, this->file) != 1)
            return false;
         return true;
      }
      void skip_bytes(uint32_t count) noexcept {
         uint32_t offset = ftell(this->file) - this->pos;
         if (this->size - offset < count)
            return;
         fseek(this->file, count, SEEK_CUR);
      }
   };
}

namespace dovah::tes_file_reading {
   void file_header_reader::clear() {
      this->name.clear();
      this->flags = 0;
      this->record_and_group_count = 0;
      this->author.clear();
      this->description.clear();
      this->masters.clear();
   }
   void file_header_reader::load(const char* path) {
      this->name = std::filesystem::path(path).filename().string();
      
      FILE*   file;
      errno_t err = fopen_s(&file, path, "rb");
      if (!file) {
         auto error = std::make_unique<dovah::notices::file_load_errors::filesystem_error>();
         auto ex    = dovah::exceptions::file_load_failed();
         
         error->filename = this->name;
         error->errno_value = err;
         
         ex.details.file_load_error = std::move(error);
         throw ex;
      }

      cobb::file_guard guard(file); // calls fclose for us

      auto _throw_on_malformed = [this, file]() {
         auto error = std::make_unique<dovah::notices::file_load_errors::malformed_file_header>();
         auto ex    = dovah::exceptions::file_load_failed();
         
         error->filename    = this->name;
         error->file_offset = ftell(file);
         
         ex.details.file_load_error = std::move(error);
         throw ex;
      };
      
      uint32_t recordSize;
      {
         uint32_t signature;
         bool     read = _read(file, signature);
         if (!read || _byteswap_ulong(signature) != 'TES4') {
            _throw_on_malformed();
         }
      }
      if (!_read(file, recordSize) || !_read(file, this->flags)) {
         _throw_on_malformed();
      }
      _skip(file, 8); // form ID of TES4 record; version control bytes
      if (!_read(file, this->header_record_version)) {
         _throw_on_malformed();
      }
      _skip(file, 2); // unknown field
      if (this->name.size() > 4) {  // Force flags based on file extension.
         const char* extension = this->name.data() + this->name.size() - 4;
         if (_strnicmp(".esm", extension, 4) == 0) {
            this->flags |= flag::master;
         } else if (_strnicmp(".esl", extension, 4) == 0) {
            this->flags |= flag::master | flag::light;
         }
      }
      //
      uint32_t   last_subrecord = 0;
      _subrecord subrecord(file, ftell(file) + recordSize);
      while (subrecord.open()) {
         switch (subrecord.signature) {
            case 'HEDR':
               subrecord.skip_bytes(4);
               subrecord.read(this->record_and_group_count);
               break;
            case 'CNAM': // creator
               subrecord.to_string(this->author);
               break;
            case 'SNAM': // description
               subrecord.to_string(this->description);
               break;
            case 'MAST':
               if (last_subrecord == 'MAST') {
                  dovah::logging::print_line("[TESPluginHeader] Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
               }
               {
                  std::string out;
                  subrecord.to_string(out);
                  this->masters.push_back(out);
               }
               break;
            case 'DATA':
               if (last_subrecord != 'MAST') {
                  _throw_on_malformed();
               }
               break;
         }
         last_subrecord = subrecord.signature;
      }
   }
}