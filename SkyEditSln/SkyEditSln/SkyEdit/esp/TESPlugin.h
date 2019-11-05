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

#define COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS 1
#ifdef COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS
   class FormMapHeap : public cobb::multithreaded_block_allocator<std::pair<uint32_t, FormStub*>, 3200, ESP_LOAD_TOTAL_THREADS> {
      public:
         inline static FormMapHeap& get() {
            static FormMapHeap instance;
            return instance;
         }
   };
   class FormMapAllocator : public std::allocator<std::pair<uint32_t, FormStub*>> {
      //
      // This is an interface between std::allocator and an instance of 
      // cobb::multithreaded_block_allocator. It's stateless.
      //
      pointer allocate(size_type n, std::allocator<void>::const_pointer = 0) {
         if (n > max_size())
            throw std::invalid_argument("Cannot allocate more than max_size().");
         return (pointer)FormMapHeap::get().allocate();
      }
      void deallocate(pointer p, size_type n) {
         if (n > max_size())
            throw std::invalid_argument("Cannot allocate more than max_size().");
         FormMapHeap::get().free((void*)p);
      }
      size_type max_size() const { return 1; }
      //
      // stateless; therefore all instances are interchangeable
      bool operator==(const FormMapAllocator& right) { return this == &right; }
      bool operator!=(const FormMapAllocator& right) { return this != &right; }
   };

   typedef std::map<uint32_t, FormStub*, std::less<uint32_t>, FormMapAllocator> map_of_forms;
#else
   typedef std::map<uint32_t, FormStub*> map_of_forms;
#endif
typedef std::map<formtype_t, map_of_forms> map_of_forms_by_type;

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
      inline bool body_is_compressed() { return (bool)(this->flags & kFlag_Compressed); }
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
      operator bool() const noexcept { return this->header.signature != 0; }
      //
      uint32_t depth() const;
      void to_string(std::string&) const;
      //
      TESPluginGroup* getParent() const;
};
class TESPluginRecord {
   friend TESPluginBaseReader;
   friend TESPluginFile;
   protected:
      TESPluginRecord(TESPluginBaseReader& file) : owner(file) {}
      //
      TESPluginBaseReader& owner;
      //
      TESPluginRecordHeader header;
      uint32_t headPos; // position in the file
      uint32_t bodyPos; // position in the body
      uint32_t end;
      //
      cobb::generic_buffer data;
      uint32_t offset = 0;
      //
      void reset() {
         this->data.free();
         this->offset = 0;
         this->header.signature = 0;
      }
      //
   public:
      //
      // Disallow copying to avoid bad memory management on the generic_buffer.
      //
      TESPluginRecord& operator=(const TESPluginRecord& other) = delete; // no copy
      TESPluginRecord(TESPluginRecord& other) = delete; // no copy
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
      uint32_t pos;
      uint32_t end;
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
      //
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
      inline bool read(void* buffer, uint32_t size) const {
         return this->get_containing_record().read(buffer, size);
      }
      inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
      template<typename T> inline bool read(T& field) const {
         return this->get_containing_record().read(field);
      }
      template<typename T> inline void unchecked_read(T& field) const {
         return this->get_containing_record().unchecked_read(field);
      }
      bool read_wstring(std::string& field); // uint16_t length; char str[length]; // length does not include a null-terminator

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
      enum ObjectType {
         kObjectType_None,
         kObjectType_Group,
         kObjectType_Record,
      };
   protected:
      #ifdef COBB_ESP_USE_MAPPED_FILES
         cobb::mapped_file* file = nullptr;
         uint32_t stream_position = 0;
      #else
         FILE* fileHandle = nullptr;
      #endif
      TESPluginGroup     groups[MAX_ESP_FILE_GROUP_DEPTH];
      TESPluginRecord    record;
      TESPluginSubrecord subrecord;
      uint32_t lastPotentialGroupParent = 0; // form ID: CELL, WRLD, DIAL
      //
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
      void open(const char* path);
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
      //
      TESPluginFile& owner;
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
      TESPluginFile& owner;
      //
      std::vector<QueuedSubBlock> queue;
      std::thread thread;
      //
      void add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos);
      void start();
      void wait_for();
};

struct TESPluginFileConfigFlags { // enum; a struct-wrapped enum is scoped like enum class but allows implicit casts to number types
   TESPluginFileConfigFlags() = delete;
   enum : uint32_t {
      none = 0x00000000,
      //
      // FLAG: TESPluginFileConfigFlags::do_not_free_own_stubs
      //
      // If set, TESPluginFile will not delete its FormStubs when destroyed. This flag 
      // should only be used if you can guarantee that all FormStubs are on a custom 
      // allocator, and that you will free all FormStubs via the allocator. The flag 
      // exists to deal with the fact that freeing *all* FormStubs for a file one by 
      // one is incredibly slow -- well over a minute.
      //
      do_not_free_own_stubs = 0x00000001,
   };
};
class TESPluginFile : public TESPluginBaseReader {
   friend TESPluginThreadedSimpleReader;
   friend TESPluginThreadedInteriorCellReader;
   friend TESPluginThreadedWorldspaceSubBlockReader;
   public:
      enum Flags {
         kFlag_Master = 0x0001,
         kFlag_LocalizedStringTable = 0x0080,
         kFlag_Light  = 0x0200, // SSE only
      };
   public:
      TESPluginFile();
      ~TESPluginFile();
      //
      bool load(const char* filepath);
      bool loadRecordAt(uint32_t pos); // for FormStub
      //
   protected:
      struct _form_map {
         std::mutex   lock;
         map_of_forms forms;
      };
      //
      bool _loadHeader();
      //
      std::string path;
      TESPluginThreadedSimpleReader complexReader; // see constructor for initializer
      TESPluginThreadedSimpleReader simpleReaders[ESP_LOAD_SIMPLE_THREADS]; // see constructor for initializer
      TESPluginThreadedInteriorCellReader interiorCellReaders[ESP_LOAD_INT_CELL_THREADS]; // see constructor for initializer
      TESPluginThreadedWorldspaceSubBlockReader worldspaceReaders[ESP_LOAD_WORLDSPACE_THREADS]; // see constructor for initializer
      //
      _form_map formsByType[FormType::Count];
      //
      uint32_t config = 0;
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
      std::vector<std::string> masters;
      // TODO: ONAM
      uint32_t subINTV;
      uint32_t subINCC;
      //
      FormStub* getForm(formtype_t formType, uint32_t formID) const;
      void forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)>);
      //
      void modify_config(bool set, uint32_t flags);
};