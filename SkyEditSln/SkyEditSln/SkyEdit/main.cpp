#include <iostream> // for testing
#include <sys/timeb.h> // for benchmarks
#include <thread> // for std::thread::id
#include "esp/TESPlugin.h"
#include "forms/Quest.h"

const char* testPath = "C:/Program Files (x86)/Steam/steamapps/common/Skyrim/Data/Skyrim.esm";

std::thread::id main_thread_id;

//
// TODO: UESP has already taken the name "SkyEdit"
//
// Possible other names:
//    NordEdit
//    Dovah-Edit
//
// TODO: REFACTOR
//
//  - All loaded forms should have a reference to their owning FormStub.
//
//  - Currently, we have no way to maintain GRUP relationships after parsing is 
//    complete. A DIAL has no way to prompt the loading of its child INFOs, and 
//    more importantly, an INFO being loaded has no way to know what DIAL it 
//    belongs to. There's only one good approach (aside from simply keeping all 
//    forms in memory)...
//
//     - FormStub instances can have a pointer to a "Group Info" struct, which 
//       contains information on the non-top-level GRUPs that contained the 
//       record. We can block-allocate those structs if need be.
//
//        - Maybe...
//
//          struct GroupMetadata { // sizeof == 0xC
//             uint32_t parentFormID; // 0 for interior cells
//             union {
//                uint32_t interior;
//                struct {
//                   int16_t x; // TODO: is it XXXXYYYY or YYYYXXXX? how does endianness affect it?
//                   int16_t y;
//                } exterior;
//             } cellBlock; // 0 for non-cells
//             union {
//                uint32_t interior;
//                struct {
//                   int16_t x; // TODO: is it XXXXYYYY or YYYYXXXX? how does endianness affect it?
//                   int16_t y;
//                } exterior;
//             } cellSubBlock; // 0 for non-cells
//          }
//
//        - Note that the "cell block" and "cell sub-block" GRUP types don't 
//          store the ID of the containing cell; as such, we'll have to traverse 
//          multiple containing GRUPs to get them. That said, all "children" 
//          GRUPs (WRLD, CELL, DIAL) do store the form ID of the "parent."
//
//  - At the top of the file, we should clearly explain why record and subrecord 
//    contents have to use different "read" and "skip" functions (it's because we 
//    HAVE TO load record contents into a buffer in order to allow uniform access 
//    for compressed and uncompressed record data).
//
//  - Improvements to multi-threaded file loading:
//
//     - Modify the std::maps for file loading to use a block allocator.
//
//     - Use the "simple" loader for DIAL.
//
//     - Create a "complex" loader for interior CELLs, which divides load tasks 
//       up by block or by sub-block.
//
//     - Create a "complex" loader for worldspaces, which divides load tasks 
//       up by worldspace.
//
//  - Create cobb::zstring as a const char* that does malloc/realloc/free for you, 
//    with both a c_str() method and an implicit (const char*) conversion. Use that 
//    for editor IDs on FormStub. It should (free) when destroyed (only the owner 
//    should have the cobb::zstring; we may even want to set its copy constructor 
//    to =deleted; other parties should take the const char*).
//
//  - loaded_form_ptr doesn't actually delete the loaded form data when the refcount 
//    hits zero. We need to add code to do that (and of course it should avoid any 
//    deletions if the "is edited" flag is set); we'll want to add debug logging to 
//    it to ensure it works properly.
//
//     - I wonder if we can have its _incRef assert that the refcount is non-zero 
//       after the increment, too, to guard against overflow. We could have _decRef 
//       assert that the refcount is non-zero before decrementing, as well.
//
//  - We'll need to eventually add a way to track Use Info akin to the CK and xEdit, 
//    so that if we delete a form at run-time, we can properly update all forms that 
//    used it. We'll want to call uses "connections" since better words are taken 
//    (i.e. "references" are game world objects and "links" may refer to linked refs).
//
//     - A FormStub will need two doubly-linked lists of connections: one outbound 
//       and one inbound. The list items should specify the type of connection 
//       (the subrecord signature will do); we can use this for detailed warnings 
//       when the user asks to delete a form.
//
//        - Connection nodes could probably be block-allocated like we do with the 
//          FormStubs themselves... Maybe it's time to template that allocator.
//

int main() {
   main_thread_id = std::this_thread::get_id();
   //
   TESPluginFile skyrim;
   skyrim.modify_config(true, TESPluginFileConfigFlags::do_not_free_own_stubs); // we are responsible for force-deleting all FormStubs via the allocator
   struct timeb bench_start;
   struct timeb bench_end;
   ftime(&bench_start);
   skyrim.load(testPath);
   ftime(&bench_end);
   std::cout << "Loaded Skyrim.esm." << std::endl;
   printf("Time taken: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   std::cout << "Author: " << skyrim.authorName << std::endl;
   std::cout << "Description: " << skyrim.description << std::endl;
   skyrim.forEachFormOfType(kFormType_Quest, [](FormStub* stub) {
      auto form = stub->load();
      if (form && form->formType == kFormType_Quest) {
         auto quest = form.ptr_cast<TESQuest>();
         const char* type = TESQuest::QuestTypeToString(quest->questType);
         if (!type)
            type = "<UNKNOWN>";
         printf("[QUST:%08X]%s (%s) is a %s quest\n", stub->formID, quest->editorID.c_str(), quest->name.c_str(), type);
         if (quest->scriptData.scripts.size()) {
            quest->scriptData.forEachScript([](PapyrusScriptData::Script* script) {
               printf(" - Script: %s with %d properties\n", script->name.c_str(), script->properties.size());
               return false;
            });
         }
      } else {
         printf("[QUST:%08X] could not be loaded.\n", stub->formID);
      }
      return false;
   });
   skyrim.forEachFormOfType(kFormType_ActorBase, [](FormStub* stub) {
      auto editorID = stub->get_editor_id();
      if (editorID)
         printf("[NPC_:%08X]%s\n", stub->formID, editorID);
      else
         printf("[NPC_:%08X] has no editor ID\n", stub->formID);
      return false;
   });
   //
   auto& fsh = FormStubHeap::get();
   //FormStubHeapPrinter fsh_printer;
   //fsh.dumpStats(fsh_printer);
   //fsh.dumpStats();
   fsh.force_free_all();
   //fsh.dumpStats();
   //
   return 0;
}