#include "file_header.h"
#include <filesystem>
#include "../../../helpers/files.h"
#include "../../logging.h"

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
      this->error = file_read_error();
   }
   bool file_header_reader::load(const char* path) noexcept {
      this->error.code = file_read_error::error_code::none;
      this->error.file = std::filesystem::path(path).filename().string();
      //
      FILE*   file;
      errno_t err = fopen_s(&file, path, "rb");
      if (!file) {
         this->error.message = "Failed to parse the file header. ";
         this->error.message += dovah::logging::file_error_code_to_string(err);
         switch (err) {
            case ENFILE:
            case EMFILE:
            case EINVAL:
            case ELOOP:
            case ENAMETOOLONG:
               error.code = file_read_error::error_code::filesystem_error;
               break;
            case EACCES:
            case EBUSY:
               error.code = file_read_error::error_code::locked_file;
               break;
            case ENOENT:
            default:
               error.code = file_read_error::error_code::missing_file;
         }
         return false;
      }
      cobb::file_guard guard(file); // calls fclose for us
      //
      this->name = std::filesystem::path(path).filename().string();
      uint32_t recordSize;
      {
         uint32_t signature;
         bool     read = _read(file, signature);
         if (!read || _byteswap_ulong(signature) != 'TES4') {
            this->error.code       = file_read_error::error_code::malformed_file;
            this->error.fileOffset = ftell(file);
            if (!read)
               this->error.message = "Failed to read the file header's record signature.";
            else
               this->error.message = "Expected a record with signature 'TES4'; got something else.";
            return false;
         }
      }
      if (!_read(file, recordSize) || !_read(file, this->flags)) {
         this->error.code       = file_read_error::error_code::malformed_file;
         this->error.fileOffset = ftell(file);
         this->error.message    = "Failed to read the file header's record header.";
         return false;
      }
      _skip(file, 8); // form ID of TES4 record; version control bytes
      if (!_read(file, this->header_record_version)) {
         this->error.code       = file_read_error::error_code::malformed_file;
         this->error.fileOffset = ftell(file);
         this->error.message    = "Failed to read the file header's record header.";
         return false;
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
                  this->error.code       = file_read_error::error_code::malformed_file;
                  this->error.fileOffset = ftell(file);
                  if (last_subrecord) {
                     this->error.message    = "Failed initial read of the file header. Unexpected 'DATA' subrecord in the file following another master.";
                     this->error.dependency = *this->masters.rbegin();
                  } else
                     this->error.message = "Failed initial read of the file header. Unexpected 'DATA' subrecord at the start of the file header.";
                  if (last_subrecord)
                     dovah::logging::print_line("[TESPluginHeader] Error: Unexpected 'DATA' subrecord in the file header following %s.", dovah::logging::format_signature(last_subrecord));
                  else
                     dovah::logging::print_line("[TESPluginHeader] Error: Unexpected 'DATA' subrecord at the start of the file header.");
                  return false;
               }
               break;
         }
         last_subrecord = subrecord.signature;
      }
      return true;
   }
}