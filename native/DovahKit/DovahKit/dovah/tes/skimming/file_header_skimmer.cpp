#include "./file_header_skimmer.h"
#include "lu/files/file_pointer_guard.h"
#include "lu/integral_character_code.h"
#include "../../exceptions/file_load_errors/filesystem_error.h"
#include "../../exceptions/file_load_errors/malformed_file_header.h"

namespace {
   struct _subrecord {
      protected:
         FILE*    file;
         uint32_t _signature = 0;
         uint16_t size      = 0;
         uint32_t pos; // subrecord position within the file
         uint32_t record_end = 0; // position of containing record's end, relative to start of file

      public:
         _subrecord(FILE* f, uint32_t re) : file(f), record_end(re) {}

         constexpr const uint32_t signature() const noexcept {
            return this->_signature;
         }

      protected:
         template<typename T>
         bool _read(FILE* file, T& field) noexcept {
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

      public:
         bool open() noexcept {
            if (this->_signature) {
               this->skip();
            }
            if (ftell(this->file) >= this->record_end)
               return false;
            auto f = this->file;
            _read(f, this->_signature);
            this->_signature = _byteswap_ulong(this->_signature);
            _read(f, this->size);
            this->pos = ftell(f);
            return this->_signature && _check(f);
         }
         void skip() noexcept {
            this->_signature = 0;
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

         template<typename T>
         bool read(T& out) const noexcept {
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

namespace dovah::tes {
   void file_header_skimmer::skim(const std::filesystem::path& file_path) {
      FILE*   file;
      errno_t err = fopen_s(&file, file_path.string().c_str(), "rb");
      if (!file) {
         throw exceptions::file_load_errors::filesystem_error(file_path, err);
      }

      lu::files::file_pointer_guard guard(file); // calls fclose for us

      auto _throw_on_malformed = [&file_path, file]() {
         throw exceptions::file_load_errors::malformed_file_header(file_path, ftell(file));
      };

      {
         lu::four_cc signature;
         if (fread(&signature.value, sizeof(signature.value), 1, file) != 1) {
            _throw_on_malformed();
         }
         if (signature == std::byteswap('TES4')) {
            this->results.endianness = std::endian::little;
         } else if (signature == 'TES4') {
            this->results.endianness = std::endian::big;
         } else {
            _throw_on_malformed();
         }
      }

      auto _read = [this, file, &_throw_on_malformed](auto& dst) {
         using value_type = std::decay_t<decltype(dst)>;
         if (fread(&dst, sizeof(dst), 1, file) != 1)
            _throw_on_malformed();
         if (this->results.endianness != std::endian::native) {
            if constexpr (lu::enum_flags_type<value_type>) {
               dst = value_type::from_mask(std::byteswap((typename value_type::mask_type)dst));
            } else {
               dst = std::byteswap(dst);
            }
         }
      };
      auto _skip = [file](size_t bytes) {
         fseek(file, bytes, SEEK_CUR);
      };

      uint32_t record_size = 0;
      _read(record_size);
      _read(this->results.flags);
      _skip(12);
         // 4 bytes: form ID of TES4 record
         // 4 bytes: version control bytes
         // 2 bytes: header record version
         // 2 bytes: unknown

      {  // force flags based on file extension
         auto ext = file_path.extension().string();
         if (ext.size() == 4) {
            if (_strnicmp(".esm", ext.c_str(), 4) == 0) {
               this->results.flags.set_each_of<file_header_flag::master>();
            } else if (_strnicmp(".esl", ext.c_str(), 4) == 0) {
               this->results.flags.set_each_of<file_header_flag::master, file_header_flag::light>();
            }
         }
      }

      uint32_t   last_subrecord = 0;
      _subrecord subrecord(file, ftell(file) + record_size);
      while (subrecord.open()) {
         switch (subrecord.signature()) {
            case 'HEDR':
               subrecord.read(this->results.file_version);
               subrecord.read(this->results.record_and_group_count);
               break;
            case 'CNAM': // creator
               break;
            case 'SNAM': // description
               break;
            case 'MAST':
               if (last_subrecord == 'MAST') {
                  //dovah::logging::print_line("[TESPluginHeader] Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
               }
               {
                  auto& out = this->results.dependencies.emplace_back();
                  subrecord.to_string(out);
               }
               break;
            case 'DATA':
               if (last_subrecord != 'MAST') {
                  _throw_on_malformed();
               }
               break;
         }
         last_subrecord = subrecord.signature();
      }
   }
}