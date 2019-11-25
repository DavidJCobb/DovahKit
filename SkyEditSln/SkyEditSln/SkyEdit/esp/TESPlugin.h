#pragma once
#include <cstdint>
#include <functional>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "base.h"
#include "../formstub.h"
#include "../forms/types.h"
#include "../helpers/memory.h"
#include "../helpers/miscellaneous.h"
#define COBB_ESP_USE_MAPPED_FILES 1
#ifdef COBB_ESP_USE_MAPPED_FILES
   #include "../helpers/files.h"
#endif
extern "C" {
   #include "../../zlib/zlib.h" // interproject ref
}

constexpr int MAX_ESP_FILE_GROUP_DEPTH = 6;

class TESPluginBaseReader;
class TESPluginFile;
class TESPluginSubrecord;
class TESPluginRecord;
struct LStringRef;

struct TESPluginGroupHeader {
   public:
      uint32_t signature = 0; // should always be 'GRUP'
      uint32_t size;
      uint32_t label;
      ESPGroupType type;
      union {
         struct {
            uint8_t vcDay;
            uint8_t vcMonth;
            uint8_t vcLastEditor;
            uint8_t vcCurrentEditor;
         };
         uint32_t versionControl;
      };
      uint32_t unknown;
};
struct TESPluginRecordHeader {
   public:
      enum {
         kFlag_Compressed = 0x00040000,
      };
      //
      uint32_t signature = 0;
      uint32_t size;
      uint32_t flags;
      uint32_t formID = 0;
      union {
         struct {
            uint8_t vcDay;
            uint8_t vcMonth;
            uint8_t vcLastEditor;
            uint8_t vcCurrentEditor;
         };
         uint32_t versionControl;
      };
      uint16_t version;
      uint16_t unknown;
      //
      inline bool body_is_compressed() const noexcept { return (bool)(this->flags & kFlag_Compressed); }
};

class TESPluginGroup {
   friend TESPluginBaseReader;
   protected:
      void initialize(TESPluginBaseReader* f) { this->owner = f; }
   public:
      void reset() {
         this->header.signature = 0;
      }
      void skip();
      //
      TESPluginBaseReader* owner;
      //
      TESPluginGroupHeader header;
      uint32_t pos;
      uint32_t end;
      //
      inline operator bool() const noexcept { return this->header.signature != 0; }
      inline bool exists() const noexcept { return this->header.signature != 0; }
      //
      uint32_t depth() const noexcept;
      void to_string(std::string&) const noexcept;
      //
      TESPluginGroup* getParent() const noexcept;
      //
      uint32_t getRawIDOfParentCell() const noexcept {
         switch (this->header.type) {
            case ESPGroupType::cell_children:
            case ESPGroupType::cell_persistent_children:
            case ESPGroupType::cell_temporary_children:
               return _byteswap_ulong(this->header.label);
         }
         return 0;
      }
      uint32_t getRawIDOfParentTopic() const noexcept {
         if (this->header.type == ESPGroupType::topic_children)
            return _byteswap_ulong(this->header.label);
         return 0;
      }
};
class TESPluginRecord {
   friend TESPluginBaseReader;
   friend TESPluginFile;
   friend TESPluginSubrecord;
   protected:
      TESPluginRecord(TESPluginBaseReader& file) : owner(file) {}
      //
      TESPluginBaseReader& owner;
      //
      TESPluginRecordHeader header;
      uint32_t headPos; // position in the file (start of the record)
      uint32_t bodyPos; // position in the file (start of the record body)
      uint32_t end;
      //
      cobb::generic_buffer data; // record body (uncompressed)
      uint32_t offset = 0; // offset for reading, within the record body
      //
      void reset() {
         this->data.free();
         this->offset = 0;
         this->header.signature = 0;
      }
      void go_to_offset(uint32_t offset) {
         this->offset = offset - this->bodyPos;
      }
      //
   public:
      //
      // Disallow copying to avoid bad memory management on the generic_buffer.
      //
      TESPluginRecord& operator=(const TESPluginRecord& other) = delete; // no copy
      TESPluginRecord(TESPluginRecord& other) = delete; // no copy
      //
      TESPluginSubrecord& get_current_subrecord() const noexcept;
      //
      operator bool() const noexcept { return this->header.signature != 0; }
      //
      bool is_in_bounds() const noexcept {
         return this->offset < this->data.size();
      }
      bool is_in_bounds(uint32_t room_for) const noexcept {
         return this->offset + room_for <= this->data.size();
      }
      inline uint32_t flags() const noexcept { return this->header.flags; }
      inline uint32_t formID() const noexcept { return this->header.formID; }
      inline uint32_t signature() const noexcept { return this->header.signature; }
      inline uint32_t size() const noexcept { return this->header.size; }
      //
      inline uint32_t stream_pos() const noexcept { return this->offset + this->bodyPos; }
      //
      bool read(void* destination, uint32_t size);
      inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
      template<typename T> bool read(T& field) {
         return this->read(&field, sizeof(T));
      }
      bool skip(uint32_t bytes);
      //
      void unchecked_read(void* destination, uint32_t size);
      template<typename T> void unchecked_read(T& field) {
         this->unchecked_read(&field, sizeof(T));
      }
      //
      inline bool body_is_compressed() const noexcept { return this->header.body_is_compressed(); }
      //
      TESPluginSubrecord& next_subrecord() const;
      uint32_t peek_next_subrecord_type();
};
class TESPluginSubrecord {
   friend TESPluginBaseReader;
   protected:
      TESPluginSubrecord(TESPluginBaseReader& file) : owner(file) {}
      //
      TESPluginBaseReader& owner;
      //
      struct {
         uint32_t signature = 0;
         uint32_t size = 0;
      } header;
      uint32_t pos; // position in the file
      uint32_t end; // position in the file
      //
      void _fixupFormID(uint32_t& id) const noexcept; // LoadOrder includes this header, so we can't include it from this header
      bool _read_form_id(form_id_t& field) const noexcept;
      bool _read_form_id(struct_form_id_t& field) const noexcept;
      void _unchecked_read_form_id(form_id_t& field) const noexcept; // this class's definition precedes the definition for TESPluginBaseReader, so we can't check if we're SSE from the header
      void _unchecked_read_form_id(struct_form_id_t& field) const noexcept;
      //
   public:
      TESPluginSubrecord& operator=(const TESPluginSubrecord& other) = delete; // no copy
      TESPluginSubrecord(TESPluginSubrecord& other) = delete; // no copy
   public:
      TESPluginRecord& get_containing_record() const;
      void reset() {
         this->header.signature = 0;
      }
      //
      inline uint32_t offset() const noexcept { return this->pos; }
      inline uint32_t end_pos() const noexcept { return this->end; }
      inline uint32_t signature() const noexcept { return this->header.signature; }
      inline uint32_t size() const noexcept { return this->header.size; }
      //
      inline operator bool() const { return this->header.signature != 0; }
      inline bool exists() const noexcept { return this->header.signature != 0; }
      //
      inline bool is_at_end() const {
         return this->get_containing_record().stream_pos() == this->end;
      }
      inline bool is_in_bounds() const {
         return this->get_containing_record().stream_pos() < this->end;
      }
      inline bool is_in_bounds(uint32_t size) const {
         return this->get_containing_record().stream_pos() + size <= this->end;
      }
      //
      inline uint32_t containing_record_signature() const { return this->get_containing_record().signature(); }
      //
      bool to_string(std::string& field);
      bool to_string(LStringRef& field); // TODO: implement string table support
      //
      inline bool skip_bytes(uint32_t count) const { return this->get_containing_record().skip(count); }
      //
      inline bool read(void* buffer, uint32_t size) const {
         return this->get_containing_record().read(buffer, size);
      }
      inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
      template<typename T> inline bool read(T& field) const {
         return this->get_containing_record().read(field);
      }
      template<> inline bool read(form_id_t& field) const { return this->_read_form_id(field); }
      template<> inline bool read(struct_form_id_t& field) const { return this->_read_form_id(field); }
      //
      template<typename T> inline void unchecked_read(T& field) const {
         this->get_containing_record().unchecked_read(field);
      }
      template<> inline void unchecked_read(form_id_t& field) const { this->_unchecked_read_form_id(field); }
      template<> inline void unchecked_read(struct_form_id_t& field) const { this->_unchecked_read_form_id(field); }
      //
      bool read_wstring(std::string& field); // uint16_t length; char str[length]; // length does not include a null-terminator
      bool read_wstring(std::wstring& field);
      //
      template<int length_bytes> inline bool read_length_prefixed_string(std::string& field) const noexcept {
         //
         // Read a string prefixed with a length, with no null terminator.
         //
         using int_t = cobb::bytecount_to_int_t<length_bytes>;
         field.clear();
         int_t length;
         if (this->read(length)) {
            field.resize(length);
            return this->read((void*)field.data(), length);
         }
         return false;
      }
      template<int length_bytes> inline bool skip_length_prefixed_string() const noexcept {
         //
         // Skip a string prefixed with a length, with no null terminator.
         //
         using int_t = cobb::bytecount_to_int_t<length_bytes>;
         int_t length;
         if (this->read(length))
            return this->skip_bytes(length);
         return false;
      }
      //
      void back_to_start() {
         this->get_containing_record().go_to_offset(this->pos);
      }

      //
      // The functions below allow you to manually manage bounds-checking: if you need to read multiple 
      // fields in sequence, then it might be a millisecond or two faster to do a single bounds-check 
      // at the start, and then do unchecked reads for the fields, e.g.
      //
      //    uint32_t foo;
      //    uint32_t bar;
      //    if (!subrecord.is_in_bounds(sizeof(foo) + sizeof(bar)))
      //       return false;
      //    subrecord.unchecked_read(foo);
      //    subrecord.unchecked_read(bar);
      //
      // Of course, you'll have to be careful if you go copying and pasting read code. Is that risk 
      // worth a few milliseconds per form, over thousands of forms? Sounds like it to me, but I can 
      // always redesign if it turns out to cause too many problems to be worth it.
      //
};

class TESPluginBaseReader {
   friend TESPluginFile;
   friend TESPluginGroup;
   friend TESPluginRecord;
   friend TESPluginSubrecord;
   public:
      enum class ObjectType {
         none,
         group,
         record,
      };
      //
      virtual const TESPluginFile* asFile() const noexcept = 0;
      //
   protected:
      #ifdef COBB_ESP_USE_MAPPED_FILES
         cobb::mapped_file* file = nullptr; // NOTE: an instance of TESPluginBaseReader may not necessarily own the file it has a pointer to
         uint32_t stream_position = 0;
      #else
         FILE* fileHandle = nullptr; // NOTE: an instance of TESPluginBaseReader may not necessarily own the file it has a pointer to
      #endif
      TESPluginGroup     groups[MAX_ESP_FILE_GROUP_DEPTH];
      TESPluginRecord    record;
      TESPluginSubrecord subrecord;
      uint32_t lastPotentialGroupParent = 0; // form ID: CELL, WRLD, DIAL
      //
      bool is_skyrim_special = false; // needed for TESPluginSubrecord::read and friends to handle SSE struct form IDs properly
      bool uses_string_table = false;
      //
      void read(void* buffer, uint32_t size) {
         #ifdef COBB_ESP_USE_MAPPED_FILES
            this->stream_position += this->file->read_from(this->stream_position, buffer, size);
         #else
            fread(buffer, size, 1, this->fileHandle);
         #endif
      }
      void read(char* buffer, uint32_t size) {
         this->read((void*)buffer, size);
      }
      template<typename T> void read(T& field, uint32_t size) {
         #ifdef COBB_ESP_USE_MAPPED_FILES
            this->stream_position += this->file->read_from(this->stream_position, field, size);
         #else
            fread(&field, size, 1, this->fileHandle);
         #endif
      }
      template<typename T> void read(T& field) {
         #ifdef COBB_ESP_USE_MAPPED_FILES
            this->stream_position += this->file->read_from(this->stream_position, field);
         #else
            fread(&field, sizeof(field), 1, this->fileHandle);
         #endif
      }
      void resetParseState() {
         for (uint32_t i = 0; i < std::extent<decltype(this->groups)>::value; i++)
            this->groups[i].reset();
         this->record.reset();
         this->subrecord.reset();
      }
      //
      FormStub* make_stub_for_record(TESPluginFile& file);
      void extract_editor_id_for_stub(FormStub*); // searches (the remainder of) the current record for EDID; if found, writes its value to the form stub
      //
   public:
      TESPluginBaseReader() : record(*this), subrecord(*this) {
         for (uint32_t i = 0; i < std::extent<decltype(this->groups)>::value; i++)
            this->groups[i].initialize(this);
      };
      //
      void     setPos(uint32_t pos);
      uint32_t getPos();
      void     rewind(uint32_t by);
      void skipBytes(uint32_t count);
      bool isEOF();
      bool is_good();
      //
      ObjectType nextRecordOrGroup();
      bool       nextSubrecord();
      //
      inline TESPluginGroup& getCurrentGroup() {
         for (signed int i = std::extent<decltype(this->groups)>::value - 1; i >= 0; i--) {
            auto& group = this->groups[i];
            if (group)
               return group;
         }
         //
         // We have to return a group& even if we're not in one, but groups have an 
         // operator bool, so you can do
         //
         // if (auto g = file->getCurrentGroup()) {
         //    //
         //    // ...
         //    //
         // }
         //
         return this->groups[0];
      }
      inline TESPluginRecord& getCurrentRecord() { return this->record; }
      inline TESPluginSubrecord& getCurrentSubrecord() { return this->subrecord; }
};

class TESPluginThreadedSimpleReader : public TESPluginBaseReader {
   //
   // Class for reading a top-level GRUP for a form type that cannot contain child 
   // GRUPs.
   //
   protected:
      struct QueuedGroup {
         uint32_t signature = 0;
         uint32_t pos = 0;
         //
         QueuedGroup(uint32_t s, uint32_t p) : signature(s), pos(p) {}
      };
      //
      void _load();
      static void _thread_handler(TESPluginThreadedSimpleReader* instance);
   public:
      TESPluginThreadedSimpleReader(TESPluginFile& f) : owner(f) {}
      TESPluginThreadedSimpleReader(TESPluginFile& f, bool allowNestedGroups) : owner(f), allowNestedGroups(allowNestedGroups) {}
      //
      virtual const TESPluginFile* asFile() const noexcept override { return &this->owner; }
      //
      TESPluginFile& owner;
      bool allowNestedGroups = false;
      //
      std::vector<QueuedGroup> queue;
      std::thread thread;
      //
      void add_group(uint32_t groupSignature, uint32_t groupPos);
      void start();
      void wait_for();
};
class TESPluginThreadedInteriorCellReader : public TESPluginBaseReader {
   protected:
      struct QueuedBlock {
         uint32_t blockNumber = 0;
         uint32_t pos = 0;
         //
         QueuedBlock(uint32_t bn, uint32_t p) : blockNumber(bn), pos(p) {}
      };
      //
      void _load();
      static void _thread_handler(TESPluginThreadedInteriorCellReader* instance);
   public:
      TESPluginThreadedInteriorCellReader(TESPluginFile& f) : owner(f) {}
      //
      virtual const TESPluginFile* asFile() const noexcept override { return &this->owner; }
      //
      TESPluginFile& owner;
      //
      std::vector<QueuedBlock> queue;
      std::thread thread;
      //
      void add_group(uint32_t groupSignature, uint32_t groupPos);
      void start();
      void wait_for();
};
class TESPluginThreadedWorldspaceSubBlockReader : public TESPluginBaseReader {
   protected:
      struct QueuedSubBlock {
         uint32_t worldspaceID = 0;
         int16_t blockX = 0;
         int16_t blockY = 0;
         int16_t subBlockX = 0;
         int16_t subBlockY = 0;
         uint32_t pos = 0;
         //
         QueuedSubBlock(uint32_t a, int16_t b, int16_t c, int16_t d, int16_t e, uint32_t f) : worldspaceID(a), blockX(b), blockY(c), subBlockX(d), subBlockY(e), pos(f) {};
      };
      //
      void _load();
      static void _thread_handler(TESPluginThreadedWorldspaceSubBlockReader* instance);
   public:
      TESPluginThreadedWorldspaceSubBlockReader(TESPluginFile& f) : owner(f) {}
      //
      virtual const TESPluginFile* asFile() const noexcept override { return &this->owner; }
      //
      TESPluginFile& owner;
      //
      std::vector<QueuedSubBlock> queue;
      std::thread thread;
      //
      void add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos);
      void start();
      void wait_for();
};
class TESPluginThreadedWorldspacePersistentCellChildrenReader : public TESPluginBaseReader {
   protected:
      struct QueuedGroup {
         uint32_t cellID;
         uint32_t pos;
         //
         QueuedGroup(uint32_t c, uint32_t p) : cellID(c), pos(p) {};
      };
      //
      void _load();
      static void _thread_handler(TESPluginThreadedWorldspacePersistentCellChildrenReader* instance);
   public:
      TESPluginThreadedWorldspacePersistentCellChildrenReader(TESPluginFile& f) : owner(f) {}
      //
      virtual const TESPluginFile* asFile() const noexcept override { return &this->owner; }
      //
      TESPluginFile& owner;
      //
      std::vector<QueuedGroup> queue;
      std::thread thread;
      //
      void add_group(uint32_t cellID, uint32_t pos);
      void start();
      void wait_for();
};

class TESPluginFileView : public TESPluginBaseReader {
   //
   // Here's an interesting problem: If you want to read the data of multiple FormStubs' 
   // records, from multiple threads, how do you do that? For example, if you want to 
   // build Use Info for forms in a multi-threaded manner after having loaded those 
   // forms, how would you do that?
   //
   // A FormStub relies on the TESPluginFile that created it in order to access the 
   // contents of its record (i.e. TESPluginFile::loadRecordAt(uint32_t), which means 
   // that that access is ordinarily not thread-safe: a TESPluginFile instance only 
   // maintains state for one record at a time.
   // 
   // This class was created to work around that limitation. As a subclass of the 
   // basic TESPluginBaseReader class, it is able to hold record state and a pointer 
   // to a plugin file. If you hold multiple sets of state, then you can give each set 
   // of state to a different thread, yes?
   //
   // You can pass instances of this class to an overload of TESPluginFile::loadRecordAt 
   // in order to access the record data at a given offset.
   //
   // TODO: The threaded reader classes have basically the same stuff as this one; they 
   // use a TESPluginFile& owner instead of a TESPluginFile* owner but are otherwise the 
   // same (i.e. they don't own the file they're being used to read); they could be made 
   // subclasses of this class.
   //
   public:
      virtual const TESPluginFile* asFile() const noexcept override { return this->owner; }
      //
      TESPluginFile* owner = nullptr;
};

SCOPE_ENUM(TESPluginFileFlags, enum TESPluginFileFlags {
   master = 0x0001,
   localized_string_table = 0x0080,
   light = 0x0200, SCOPED_ENUM_COMMENT("SSE only")
});
class TESPluginFile : public TESPluginBaseReader {
   friend TESPluginThreadedSimpleReader;
   friend TESPluginThreadedInteriorCellReader;
   friend TESPluginThreadedWorldspaceSubBlockReader;
   friend TESPluginThreadedWorldspacePersistentCellChildrenReader;
   public:
      using Flags = TESPluginFileFlags;
      struct MasterEntry {
         std::string master; // MAST
         uint64_t    data;   // DATA
      };
   public:
      TESPluginFile();
      ~TESPluginFile();
      //
      virtual const TESPluginFile* asFile() const noexcept override { return this; }
      //
      bool load(const char* filepath);
      bool loadRecordAt(uint32_t pos); // for FormStub
      bool loadRecordAt(uint32_t pos, TESPluginFileView* reader); // for FormStub (multi-threaded building of Use Info); the reader passed in must not be the "owner" of its mapped file
      //
   protected:
      bool _loadHeader();
      //
      std::string path;
      std::string name;
      TESPluginThreadedSimpleReader complexReader; // see constructor for initializer
      TESPluginThreadedSimpleReader simpleReaders[ESP_LOAD_SIMPLE_THREADS]; // see constructor for initializer
      TESPluginThreadedInteriorCellReader interiorCellReaders[ESP_LOAD_INT_CELL_THREADS]; // see constructor for initializer
      TESPluginThreadedWorldspaceSubBlockReader worldspaceReaders[ESP_LOAD_WORLDSPACE_THREADS]; // see constructor for initializer
      TESPluginThreadedWorldspacePersistentCellChildrenReader worldCellReaders[ESP_LOAD_WORLD_CELL_THREADS]; // see constructor for initializer
      //
      bool aborted = false;
      //
      void _insertForm(uint32_t formID, FormStub* stub);
      //
   public:
      uint32_t flags = 0;
      float    fileVersion = 0.94F;
      uint32_t recordCount = 0;
      uint32_t nextFormID;
      char     authorName[512];
      char     description[512];
      std::vector<MasterEntry> masters;
      // TODO: ONAM
      uint32_t subINTV;
      uint32_t subINCC;
      //
      inline const std::string& getFilename() const noexcept { return this->name; }
      void abort() noexcept;
};