#include <iostream> // for testing
#include <sys/timeb.h> // for benchmarks
#include "esp/TESPlugin.h"
#include "forms/Quest.h"

const char* testPath = "C:/Program Files (x86)/Steam/steamapps/common/Skyrim/Data/Skyrim.esm";

//
// TODO: UESP has already taken the name "SkyEdit"
//
// Possible other names:
//    NordEdit
//    Dovah-Edit
//
// TODO: REFACTOR
//
//  - See last Github commit text for more changes to make.
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
//  - Look into multi-threading the loading of top-level GRUPs whose form types 
//    cannot have nested GRUPs. I think that's gonna be the only way we get a 
//    significant improvement in loading perf past this point.
//
//     - Hold up, partner! FormStubHeap isn't thread-safe, and if we fix that 
//       by just slapping a lock on it, then we've effectively made a single-
//       threaded program that likes to fantasize from time to time. Fortunately, 
//       I have a plan to fix this:
//
//        - The main thread should skim through all GRUPs and coordinate with 
//          five other threads: the NESTABLE thread, which handles all top-level 
//          GRUPs for form types that can have nested GRUPs (i.e. CELL, DIAL, 
//          and WRLD); and four SIMPLE threads, which each handle a single top-
//          level GRUP at a time (only the ones that can't have nested GRUPs).
//
//        - We need to define a multithreaded_block_allocator that has five 
//          linked lists of Blocks -- one for each thread that will be allowed 
//          to access the allocator -- and each linked list is paired with a 
//          thread ID. In essence, each thread (the one NESTABLE thread and the 
//          four SIMPLE threads) has its own heap.
//
//           - The multi-threaded block allocator needs to lock if it's asked 
//             to allocate a block by an unrecognized thread ID. If there's 
//             room for another thread, then the multi-threaded block allocator 
//             needs to set that up; otherwise, it needs to use malloc.
//
//           - The multi-threaded block allocator needs to lock every time it's 
//             asked to free a FormStub, NO MATTER WHAT THREAD IS ASKING. One 
//             thread can ask to free a FormStub that was originally allocated 
//             by a different thread; that's valid behavior. Moreover, if we 
//             fall back to malloc whenever an unrecognized thread allocates, 
//             then we need to fall back to free if the FormStub we're trying 
//             to free isn't on any of our block lists.
//
//           - Threads need to tell the multi-threaded block allocator when 
//             they close, so that it knows which block lists are available 
//             again. Alternatively, instead of using threads directly, we 
//             need to use std::async and "futures" i.e. <https://stackoverflow.com/questions/42418360/how-to-check-if-thread-has-finished-work-in-c11-and-above> 
//             so that we can check whether a thread has closed after the 
//             fact.
//
//              - ...or if the standard library seems like too much for our 
//                purposes, we could write a thinner thread wrapper for 
//                ourselves.
//
//              - So, our allocator would check if the allocation is coming 
//                from a known thread ID. If it isn't, then we lock the 
//                whole allocator and check whether we have any free slots 
//                (thread ID 0). If not, we check whether any slots represent 
//                threads that are known to have closed (in which case they're 
//                free).
//
//     - First, we have to figure out how to read from different points in the 
//       same file from different threads.
//
//     - We'll want to be careful to account for redundant GRUPs e.g. two ACTI 
//       groups; those will have to be parsed in order.
//
//     - TESPluginFile::formsByType is not necessarily thread-safe. Its type is 
//       std::map<formtype_t, std::map<uint32_t, FormStub*>>; I think what we'll 
//       want to do is do formsByType[i] for every possible form type before load, 
//       to force the creation of all inner maps. Then, we have each thread build 
//       its own std::map<uint32_t, FormStub*> and when the thread is done, it can 
//       use std::swap to overwrite formsByType[i] with the thread-local map.
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

#include "helpers/threading.h"
//
// TODO: Test cobb::multithreaded_block_allocator
//
void _thread_test_sub(void* config, cobb::thread& thread) {
   uint32_t index = *(uint32_t*)config;
   //
   std::this_thread::sleep_for(std::chrono::seconds(index));
   std::cout << "Done thread ID " << std::this_thread::get_id() << " which handled index " << index << ".\n";
}
void _thread_test() {
   std::shared_ptr<cobb::thread> threads[5];
   uint32_t states[5];
   struct timeb bench_start;
   struct timeb bench_end;
   printf("Running threading test...\n");
   ftime(&bench_start);
   for (uint32_t i = 0; i < std::extent<decltype(threads)>::value; i++) {
      states[i]  = i;
      threads[i] = cobb::spawn_thread(_thread_test_sub, &states[i]);
   }
   cobb::wait_for_all_threads(std::extent<decltype(threads)>::value, threads);
   ftime(&bench_end);
   printf("Time taken: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   for (uint32_t i = 0; i < std::extent<decltype(threads)>::value; i++) {
      printf(" - Confirming state for thread %d; should be 0 (dead): %d\n", i, threads[i]->alive);
   }
}

int main() {
   _thread_test();
   //
   TESPluginFile skyrim;
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
   FormStubHeapPrinter fsh_printer;
   fsh.dumpStats(fsh_printer);
   //
   return 0;
}