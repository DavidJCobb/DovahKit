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
//  - Split (record), (subrecord), and (group) off from TESPluginFile into separate 
//    structs. Essentially, take TESPluginRecord and friends and change them from 
//    just pointers-with-methods to actual state objects, and have getCurrentXXXXX 
//    return references to them. The structs should have a TESPluginFile& owner. 
//    The structs will still be members of TESPluginFile and not be used anywhere 
//    else, but this will make things more orderly. Probably.
//
//    We need to handle multiple levels of nesting for groups, so define a constexpr 
//    int MAX_GROUP_DEPTH at the top of the file and then give TESPluginFile some-
//    thing like TESPluginGroup groups[MAX_GROUP_DEPTH]. getCurrentGroup should 
//    REQUIRE a group index; getContainingGroup should return the deepest group that 
//    actually exists (signature != 0).
//
//     - ...And then TESPluginRecord::getContainingGroup can call that function.
//
//     = DONE: SPLITTING THE STRUCTURES OFF.
//
//     = NEXT: MULTIPLE GROUPS AND REWRITING nextRecord (see below)
//
//  - At the top of the file, we should clearly explain why record and subrecord 
//    contents have to use different "read" and "skip" functions (it's because we 
//    HAVE TO load record contents into a buffer in order to allow uniform access 
//    for compressed and uncompressed record data).
//
//  - We'll need to change how we iterate over records. When using nextRecord(), 
//    there's no way to tell if it returned false because we're at the end of a 
//    group or because we've encountered a child group. The nextRecord() function 
//    should only be used by TESPluginFile::load, though, so we can redesign it 
//    however we like e.g. nextRecordOrGroup() which returns an enum indicating 
//    what we hit.
//
//     - It'll need to peek four bytes after the end of the last record; that'll 
//       grab the signature of the next object which, if it's 'GRUP', is a group.
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
   FormStubHeap::get().dump();
   //FormStubHeap::get().forceFreeAll();
   //FormStubHeap::get().dump();
   //
   return 0;
}