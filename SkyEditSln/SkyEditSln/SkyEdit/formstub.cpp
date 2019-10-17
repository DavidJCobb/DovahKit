#include "formstub.h"
#include "esp/TESPlugin.h"
#include "helpers/miscellaneous.h"
#include "output.h"
#include <cassert>
#include <cstddef>

#include "forms/Quest.h"

FormStub::~FormStub() {
   if (this->editorID) {
      free(this->editorID);
      this->editorID = nullptr;
   }
}
char* FormStub::allocate_editor_id(size_t length) {
   if (this->editorID)
      free(this->editorID);
   this->editorID = (char*)malloc(length);
   return this->editorID;
}
loaded_form_ptr<TESForm> FormStub::load() {
   if (!this->form && this->file) {
      //_DEBUGMSG("stub is loading...");
      auto file = this->file;
      if (this->file->loadRecordAt(this->offset)) {
         auto record = this->file->getCurrentRecord();
         auto formType = signatureToFormType(record.signature());
         switch (formType) {
            case kFormType_Quest:
               {
                  auto q = new TESQuest();
                  q->load(record);
                  this->form = q;
               }; break;
         }
      } else
         _DEBUGMSG("...stub failed.");
   }
   return loaded_form_ptr<TESForm>(this);
}
void FormStub::set_edited(bool v) {
   cobb::edit_bit(this->refcount, kRefcountFlag_Edited, v);
}

void* FormStubHeap::Block::allocate() {
   auto i = this->info.presence.find_first_clear();
   if (i < 0)
      return nullptr;
   std::ptrdiff_t start = (std::ptrdiff_t) &this->buffer;
   std::ptrdiff_t addr  = start + (sizeof(element_type) * i);
   this->info.presence.set(i);
   return (void*)addr;
}
void* FormStubHeap::allocate() {
   if (!this->firstBlock) {
      this->firstBlock = new Block;
   }
   Block* block = this->firstBlock;
   Block* last  = block;
   void* out = block->allocate();
   while (!out) {
      block = block->info.next;
      if (block)
         last = block;
      else
         break;
      out = block->allocate();
   }
   if (out)
      return out;
   if (!block) {
      assert(last, "Couldn't figure out how to create a new block.");
      auto next = new Block;
      last->info.next = next;
      next->info.prev = last;
      out = next->allocate();
   }
   return out;
}
void FormStubHeap::free(void* mem) {
   assert(this->firstBlock, "Cannot free; nothing was allocated.");
   Block* block = this->firstBlock;
   do {
      std::ptrdiff_t m_addr  = (std::ptrdiff_t)mem;
      std::ptrdiff_t b_start = (std::ptrdiff_t)&block->buffer;
      std::ptrdiff_t b_end   = b_start + sizeof(block->buffer);
      if (m_addr >= b_start && m_addr < b_end) {
         m_addr -= b_start;
         uint16_t index = m_addr / sizeof(element_type);
         assert(m_addr % sizeof(element_type) == 0, "Cannot free; element is not aligned.");
         assert(block->info.presence.test(index),   "You're freeing something that was already free!");
         block->info.presence.reset(index);
         //
         if (block != this->firstBlock && block->info.presence.none()) {
            //
            // This block is no longer in use. Delete it.
            //
            auto p = block->info.prev;
            auto n = block->info.next;
            if (p)
               p->info.next = n;
            if (n)
               n->info.prev = p;
            delete block;
         }
         //
         return;
      }
   } while (block = block->info.next);
   assert(false, "Cannot free; element not found on our heap.");
}

/*static*/ void* FormStub::operator new(std::size_t sz) {
   if (sz != sizeof(FormStub))
      return ::operator new(sz);
   return FormStubHeap::get().allocate();
}
/*static*/ void FormStub::operator delete(void* ptr, std::size_t sz) {
   if (sz != sizeof(FormStub))
      return ::operator delete(ptr, sz);
   return FormStubHeap::get().free(ptr);
   
}

void FormStubHeap::dump() {
   _DEBUGMSG("=================================================================================");
   _DEBUGMSG("Dumping stats for the FormStubHeap...");
   uint32_t blockCount = 0;
   uint32_t slotCount = 0;
   uint32_t slotsUsed = 0;
   uint32_t editorIDSizes = 0;
   for (auto block = this->firstBlock; block; block = block->info.next) {
      blockCount++;
      slotCount += ce_countPerBlock;
      //
      auto& presence = block->info.presence;
      for (uint16_t i = 0; i < ce_countPerBlock; i++) {
         if (presence.test(i)) {
            slotsUsed++;
            //
            std::ptrdiff_t addr = (std::ptrdiff_t)block->buffer + sizeof(FormStub) * i;
            FormStub* stub = (FormStub*)addr;
            auto id = stub->get_editor_id();
            if (id)
               editorIDSizes += strlen(id) + 1;
         }
      }
   }
   _DEBUGMSG("Blocks: %d", blockCount);
   _DEBUGMSG("Total Slots: %d used out of %d", slotsUsed, slotCount);
   _DEBUGMSG("Memory Usage:");
   _DEBUGMSG(" - %d bytes overhead for block metadata", (sizeof(BlockInfo) * blockCount));
   _DEBUGMSG(" - %d bytes allocated for FormStub storage", sizeof(element_type) * slotCount);
   _DEBUGMSG(" - %d bytes in use for FormStub instances", sizeof(element_type) * slotsUsed);
   _DEBUGMSG(" - %d bytes' worth of editor ID text held elsewhere", editorIDSizes);
   //
   _DEBUGMSG("Overview by block:");
   blockCount = 0;
   for (auto block = this->firstBlock; block; block = block->info.next) {
      _DEBUGMSG(" - Block %d:", blockCount);
      blockCount++;
      //
      auto& presence = block->info.presence;
      _DEBUGPRINT("    - ");
      for (uint16_t i = 0; i < ce_countPerBlock; i++) {
         if (presence.test(i))
            _DEBUGPRINT("1");
         else
            _DEBUGPRINT("0");
      }
      _DEBUGPRINT("\n");
   }
   _DEBUGMSG("All blocks listed.");
   _DEBUGMSG("=================================================================================");

}
void FormStubHeap::forceFreeAll() {
   _DEBUGMSG("=================================================================================");
   _DEBUGMSG("Forcibly freeing all FormStubs...");
   auto last = this->firstBlock;
   if (!last) {
      _DEBUGMSG("There are none to free.");
      _DEBUGMSG("=================================================================================");
      return;
   }
   while (last->info.next)
      last = last->info.next;
   //
   auto prev = last->info.prev;
   do {
      auto& presence = last->info.presence;
      //
      // Deleting an element can delete the containing Block, so we need to get all of 
      // the pointers first -- that way, the Block doesn't get deleted out from under 
      // us.
      //
      element_type* pointers[ce_countPerBlock];
      for (uint32_t i = 0; i < ce_countPerBlock; i++) {
         if (presence.test(i)) {
            std::ptrdiff_t start = (std::ptrdiff_t) & last->buffer;
            std::ptrdiff_t addr = start + (sizeof(element_type) * i);
            //
            pointers[i] = (element_type*)addr;
         } else
            pointers[i] = nullptr;
      }
      for (uint32_t i = 0; i < ce_countPerBlock; i++) {
         if (pointers[i]) {
            delete pointers[i];
            pointers[i] = nullptr;
         }
      }
      last = prev;
      if (prev)
         prev = prev->info.prev;
   } while (last);
   _DEBUGMSG("Done.");
   _DEBUGMSG("=================================================================================");
}