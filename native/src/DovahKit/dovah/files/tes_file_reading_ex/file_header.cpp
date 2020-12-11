#include "file_header.h"
#include <filesystem>
#include "../../../helpers/files.h"
#include "../../detailed_notice.h"
#include "../../logging.h"
#include "../../notice_code_list.h"

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
   bool file_header_reader::load(const char* path, detailed_notice* error) noexcept {
      if (error) {
         error->code = notice_code::none;
         error->set_cause_file(std::filesystem::path(path).filename().string());
      }
      //
      FILE*   file;
      errno_t err = fopen_s(&file, path, "rb");
      if (!file) {
         if (error) {
            error->set_errno(err);
            switch (err) {
               case ENFILE:
               case EMFILE:
               case EINVAL:
               case ELOOP:
               case ENAMETOOLONG:
                  error->code = notice_code::filesystem_error;
                  break;
               case EACCES:
               case EBUSY:
                  error->code = notice_code::locked_file;
                  break;
               case ENOENT:
               default:
                  error->code = notice_code::missing_file;
            }
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
            error->code = notice_code::malformed_file;
            error->set_file_offset(ftell(file));
            return false;
         }
      }
      if (!_read(file, recordSize) || !_read(file, this->flags)) {
         error->code = notice_code::malformed_file;
         error->set_file_offset(ftell(file));
         return false;
      }
      _skip(file, 8); // form ID of TES4 record; version control bytes
      if (!_read(file, this->header_record_version)) {
         error->code = notice_code::malformed_file;
         error->set_file_offset(ftell(file));
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
                  error->code = notice_code::malformed_file;
                  error->set_file_offset(ftell(file));
                  return false;
               }
               break;
         }
         last_subrecord = subrecord.signature;
      }
      return true;
   }
}