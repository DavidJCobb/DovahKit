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
// NOTES:
//
//  - xEdit takes 3 seconds to load all forms and their editor IDs. It doesn't 
//    load full form data; it basically does a similar approach to us, I think.
//
//  - xEdit is capable of caching "Use Info" (i.e. reference information) after 
//    building it once. It takes 2 seconds to load cached data for Skyrim.esm, 
//    and 66 seconds to build it from scratch.
//
//  - According to zilav, xEdit uses a single-threaded loader, and relies on 
//    file mapping (i.e. CreateFileMapping/MapViewOfFile) for its raw speed. 
//    I should give that a try -- map the entire Skyrim.esm file into memory 
//    and then run through it. If I just change the underlying file handle and 
//    fread stuff, then it shouldn't even be all that difficult to test.
//
// TODO: REFACTOR
//
//  - cobb::mapped_file should return a failure code when it can't open a file 
//    (akin to the C file functions) instead of just asserting to death.
//
//  - Make the FormType enum an enum class, and change the names of the values 
//    (the "kFormType_" prefix is unnecessary if it's an enum class).
//
//  - All loaded forms should have a reference to their owning FormStub.
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
//        - This produces no improvement.
//
//        - Currently we use two levels of nesting for TESPluginFile::formsOfType. 
//          Change that to a std::map<...> formsOfType[kFormType_Max], so we can 
//          access maps directly in the threads and iterate over them when we 
//          build form connections (see below). Plus, cutting out one layer of 
//          maps may speed things up in and of itself.
//
//        - We lose 2.5 seconds to merging multiple std::maps together, to bring 
//          the results produced by each thread into the central TESPluginFile. 
//          I wonder what we can do about that.
//
//           - Does std::map... leave room... between elements? So that insertions 
//             don't require reallocations?
//
//           - std::map is typically a red-black tree. This page describes how to 
//             implement parallel mass insertions for red-black trees without 
//             having to use locks: <https://xuezhaokun.github.io/150-algorithm/> 
//             It appears to be derivative of: <https://www.cs.umanitoba.ca/~hacamero/Research/RBTreesKim.pdf>
//
//              - If we adopt this approach, then it would entail creating a 
//                custom red-black tree built for this task, and using that 
//                instead of a std::map<uint32_t formID, FormStub*>. Each thread 
//                would then have to insert into that. (Not sure how we'd enforce 
//                thread-safety on the outer std::map<formtype_t, map>, though.)
//
//     - Can we divide up the loading of DIALs and their child GRUPs? They don't 
//       use blocks/sub-blocks like worldspaces do.
//
//  - Create cobb::zstring as a const char* that does malloc/realloc/free for you, 
//    with both a c_str() method and an implicit (const char*) conversion. Use that 
//    for editor IDs on FormStub. It should (free) when destroyed (only the owner 
//    should have the cobb::zstring; we may even want to set its copy constructor 
//    to =deleted; other parties should take the const char*).
//
//  - We'll need to eventually add a way to track Use Info akin to the CK and xEdit, 
//    so that if we delete a form at run-time, we can properly update all forms that 
//    used it. We'll want to call uses "connections" since better words are taken 
//    (i.e. "references" are game world objects and "links" may refer to linked refs).
//
//     - What's written below will need adjustment for when we handle multiple 
//       files, but we can look into handling things differently later. Basically, 
//       when we get to multiple files, we'll need multiple FormStubs for each 
//       form, because a form can be defined in one file and overridden in another. 
//       As such, we'll need to handle FormStubs connecting to FormStubLists or 
//       whatever we decide to call 'em.
//
//       The difference is between form definitions (file-local) and forms: 
//       a FormStub connects to a Form. But for now, we can implement these 
//       connections as FormStub-to-FormStub and just... tailor the data later.
//
//     - A FormStub will need two doubly-linked lists of connections: one outbound 
//       and one inbound. The list items should specify the type of connection 
//       (the subrecord signature will do); we can use this for detailed warnings 
//       when the user asks to delete a form.
//
//        - We can use multiple threads to iterate over all FormStubs and generate 
//          the outbound connections. Then, we can use a single thread to iterate 
//          over all FormStubs and generate the inbound connections. Multi-threading 
//          this should give us a speed advantage over xEdit if my code turns out 
//          as good as theirs (though it will be impossible to measure this until I 
//          write code to load every single form type).
//
//           - Make sure you check FormTypeFlags::no_connections and avoid running 
//             this process on those forms.
//
//           - We should define a struct form_connection, which just wraps a 
//             uint32_t form ID. Give it "load" and "modify" methods which take the 
//             containing form as an argument; the "load" method modifies the form 
//             ID and creates an outbound connection on the containing form's stub, 
//             while the "modify" method (for use when actually editing content) 
//             modifies the form ID and alters both inbound and outbound connections 
//             on the relevant stubs.
//
//              - The only way to really generate outbound connections is to have 
//                the FormStub load its form, have the form's load function generate 
//                the connections by calling form_connection::load, and then have 
//                the FormStub discard the rest of the loaded form data.
//
//              - void FormStub::createOutboundConnection(uint32_t signature, uint32_t formID); // only makes an outbound connection
//              - void FormStub::makeOutboundConnectionsBidirectional(); // loops over all outbounds; builds inbounds
//              - void FormStub::replaceOutboundConnection(uint32_t signature, uint32_t formID); // modifies an outbound connection; reaches out to the previously-connected form to sever its inbound connection; and reaches out to the newly-connected form to add an inbound connection
//              - void form_connection::load(TESForm& form, uint32_t value);
//              - void form_connection::modify(TESForm& form, uint32_t value);
//
//           - Inbound connections have to be single-threaded because processing any 
//             FormStub could lead to modifications to any other FormStub; however, 
//             setting up outbound connections for a FormStub doesn't involve 
//             altering data elsewhere, so we could split the FormStub list into 
//             chunks and assign each chunk to a thread.
//
//              - This requires a custom red-black tree. The whole point of a tree 
//                is that it's subdivided into halves and halves-of-halves; we can 
//                evenly divide the list into any power-of-two number of chunks. 
//                However, std::map doesn't give us the needed accessors for this, 
//                in part because the STL only wants uniform interfaces and in part 
//                because the standard doesn't require std::map to actually be a 
//                binary tree or any other specific implementation.
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
   skyrim.forEachFormOfType(FormType::Quest, [](FormStub* stub) {
      auto form = stub->load();
      if (form && form->formType == FormType::Quest) {
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
   skyrim.forEachFormOfType(FormType::ActorBase, [](FormStub* stub) {
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