#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "base.h"
#include "TESPlugin.h"
#include "../formstub.h"
#include "../forms/types.h"
#include "../helpers/miscellaneous.h"
extern "C" {
   #include "../../zlib/zlib.h" // interproject ref
}

class TESPluginSaver;

class TESPluginRecordSaver {
   friend TESPluginSaver;
   protected:
      TESPluginSaver& owner;
      TESPluginRecordHeader header;
      uint32_t headPos = 0;
      uint32_t bodyPos = 0;
      //
   public:
      void setFlags(uint32_t);
      void modifyFlag(uint32_t mask, bool);
      void setVersionControl(uint8_t, uint8_t, uint8_t, uint8_t);
      void setVersion(uint16_t);
      void setUnknown(uint16_t);
      //
      void close() noexcept;
      //
      void write(void* source, uint32_t size) noexcept;
      template<typename T> void write(const T& field) noexcept {
         this->write(&field, sizeof(T));
      }
      template<int length_bytes> inline bool write_length_prefixed_string(std::string& field) noexcept {
         using int_t = cobb::bytecount_to_int_t<length_bytes>;
         auto size = field.size();
         int_t length;
         if (size > std::numeric_limits<int_t>::max())
            length = std::numeric_limits<int_t>::max();
         else
            length = size;
         this->write(field.data(), length);
      }
      //
      inline uint32_t get_current_size() const noexcept { return this->header.size; }
      inline uint32_t get_signature() const noexcept { return this->header.signature; }
};
class TESPluginSubrecordSaver {
   friend TESPluginSaver;
   protected:
      TESPluginSaver& owner;
      struct {
         uint32_t signature = 0;
         uint32_t size = 0;
      } header;
      uint32_t pos;
      //
   public:
      TESPluginRecordSaver& get_containing_record() const;
      void close();
      //
      void write(void* source, uint32_t size) noexcept;
      template<typename T> void write(const T& field) noexcept {
         this->write(&field, sizeof(T));
      }
      template<int length_bytes> inline bool write_length_prefixed_string(std::string& field) noexcept {
         using int_t = cobb::bytecount_to_int_t<length_bytes>;
         auto size = field.size();
         int_t length;
         if (size > std::numeric_limits<int_t>::max())
            length = std::numeric_limits<int_t>::max();
         else
            length = size;
         this->write(field.data(), length);
      }
      //
      inline uint32_t get_current_size() const noexcept { return this->header.size; }
      inline uint32_t get_signature() const noexcept { return this->header.signature; }
};

class TESPluginSaver {
   friend TESPluginRecordSaver;
   friend TESPluginSubrecordSaver;
   protected:
      uint32_t groupStartPos[7];
      TESPluginRecordSaver    record;
      TESPluginSubrecordSaver subrecord;
   public:
      void openRecord(uint32_t signature, uint32_t formID);
      void openSubrecord();

      void write(void* source, uint32_t size) {
         assert(false && "IMPLEMENT ME!");
      }
      void write_to(uint32_t pos, void* source, uint32_t size);
};