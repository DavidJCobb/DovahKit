#include "./file_header_skimmer.h"
#include <fstream>
#include "lu/integral_character_code.h"
#include "../../exceptions/file_load_errors/filesystem_error.h"
#include "../../exceptions/file_load_errors/malformed_file_header.h"

namespace {
   struct _subrecord {
      protected:
         std::ifstream& stream;
         uint32_t _signature = 0;
         uint32_t body_start = 0;
         uint16_t size       = 0;
         uint32_t record_end = 0; // position of containing record's end, relative to start of file

      public:
         _subrecord(std::ifstream& s, uint32_t re) : stream(s), record_end(re) {}

         constexpr const uint32_t signature() const noexcept {
            return this->_signature;
         }

      protected:
         template<typename T>
         bool _read(T& field) noexcept {
            this->stream.read((char*)&field, sizeof(field));
            if (this->stream.gcount() < sizeof(field))
               return false;
            return true;
         }
         void _skip(uint32_t byteCount) noexcept {
            this->stream.seekg(byteCount, std::ios_base::cur);
         }
         bool _check() const noexcept {
            if (this->stream.eof() || this->stream.fail())
               return false;
            return true;
         }

      public:
         bool open() noexcept {
            if (this->_signature) {
               this->skip();
            }
            if ((uint32_t)this->file_pos() >= this->record_end)
               return false;
            this->_read(this->_signature);
            this->_signature = _byteswap_ulong(this->_signature);
            this->_read(this->size);
            this->body_start = (uint32_t)this->file_pos();
            return this->_signature && _check();
         }
         void skip() noexcept {
            this->_signature = 0;
            uint32_t offset = (uint32_t)(this->file_pos() - this->body_start);
            if (offset >= this->size)
               return;
            _skip(this->size - offset);
         }
         bool to_string(std::string& field) const noexcept {
            field.clear();
            const size_t size = this->size;
            if (!size)
               return true;
            field.resize(size);
            this->stream.read((char*)field.data(), field.size());
            if (this->stream.gcount() != field.size())
               return false;
            if (field.back() == '\0')
               field.resize(size - 1);
            return true;
         }

         inline size_t file_pos() const noexcept {
            return this->stream.tellg();
         }

         template<typename T>
         bool read(T& out) noexcept {
            uint32_t consumed = (uint32_t)(this->file_pos() - this->body_start);
            if (this->size - consumed < sizeof(T))
               return false;
            return this->_read(out);
         }
         void skip_bytes(uint32_t count) noexcept {
            uint32_t consumed = (uint32_t)(this->file_pos() - this->body_start);
            if (this->size - consumed < count)
               return;
            this->_skip(count);
         }
   };
}

namespace dovah::tes {
   void file_header_skimmer::skim(const std::filesystem::path& file_path) {
      std::ifstream stream(file_path, std::ios_base::binary);
      if (!stream) {
         throw exceptions::file_load_errors::filesystem_error(file_path, errno);
      }

      auto _throw_on_malformed = [&file_path, &stream]() {
         throw exceptions::file_load_errors::malformed_file_header(file_path, stream.tellg());
      };

      {
         lu::four_cc signature;
         const auto fuckfuckfuck = sizeof(signature);
         stream.read((char*)&signature, sizeof(signature));
         if (stream.gcount() != 4)
            _throw_on_malformed();
         if (signature == std::byteswap('TES4')) { // signatures have opposite endianness from everything else
            this->results.endianness = std::endian::little;
         } else if (signature == 'TES4') {
            this->results.endianness = std::endian::big;
         } else {
            _throw_on_malformed();
         }
      }

      auto _read = [this, &stream, &_throw_on_malformed](auto& dst) {
         using value_type = std::decay_t<decltype(dst)>;

         stream.read((char*)&dst, sizeof(dst));
         if (stream.gcount() != sizeof(dst))
            _throw_on_malformed();
         if (this->results.endianness != std::endian::native) {
            if constexpr (lu::enum_flags_type<value_type>) {
               dst = value_type::from_mask(std::byteswap((typename value_type::mask_type)dst));
            } else {
               dst = std::byteswap(dst);
            }
         }
      };
      auto _skip = [&stream](uint32_t bytes) {
         stream.seekg(bytes, std::ios_base::cur);
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
      _subrecord subrecord(stream, (uint32_t)stream.tellg() + record_size);
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