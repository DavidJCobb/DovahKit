#include <iostream> // for testing
#include <sys/timeb.h> // for benchmarks
#include <thread> // for std::thread::id
#include <filesystem>
#include "esp/LoadOrder.h"
#include "esp/TESPlugin.h"
#include "forms/loaded/Quest.h"

const char* TEST_PLUGIN_PATH = "C:/Program Files (x86)/Steam/steamapps/common/Skyrim/Data/";

std::thread::id main_thread_id;

//
// TODO: UESP has already taken the name "SkyEdit"
//
// Possible other names:
//    Dovah-Edit
//    NordEdit
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
//
// TODO: REFACTOR
//
//  - cobb::mapped_file should return a failure code when it can't open a file 
//    (akin to the C file functions) instead of just asserting to death.
//
//  - All loaded forms should have a reference to their owning FormStub.
//
//  - At the top of the file, we should clearly explain why record and subrecord 
//    contents have to use different "read" and "skip" functions (it's because we 
//    HAVE TO load record contents into a buffer in order to allow uniform access 
//    for compressed and uncompressed record data).
//
//  - Cleanup:
//
//     - Make the ESPGroupType enum a scoped implicit-castable enum.
//
//     - Make the PapyrusPropertyType enum a scoped implicit-castable enum.
//
//     - Either replace TESPluginFile::_insertForm with direct calls to the 
//       analogous method on LoadOrder, or have TESPluginFile::_insertForm delete 
//       the form stub and do nothing if the file has been told to abort (see 
//       below).
//
//  - Loading:
//
//     - The form type list is still incomplete, but the only gaps are form 
//       types that don't appear in any official files. I assume SKSE pulled 
//       them from TESV.EXE's map of signatures to form type enum values. Do 
//       they even have classes? Does the game even load them?
//
//        - There are some form types whose names are known, but whose 
//          layouts are not; they do not appear in any vanilla game files. 
//          Does the game even load them?
//
//     - TESPluginHeader and TESPluginFile need to force the "is master" flag 
//       if the file's extension is ESM or ESL.
//
//     - The TESPluginFile reading code should fail if we encounter a group 
//       nested too deeply (i.e. deeper than MAX_ESP_FILE_GROUP_DEPTH).
//
//     - Test all error messages that run through LoadOrder::logError.
//
//        - Probably best if we hand-make some intentionally malformed files; 
//          and write code to test all of them all in one go.
//
//        - If a file's dependency doesn't exist, then we should indicate 
//          which file had the dependency. Currently we only log the name of 
//          the missing file.
//
//        - Errors relating to bad records and subrecords do get logged 
//          properly, but don't halt the load process. See code comments in 
//          and around TESPluginBaseReader::nextRecordOrGroup and in 
//          TESPluginBaseReader::nextSubrecord for details.
//
//     - LoadOrder::load needs to verify that the files' masters haven't 
//       changed once we begin the final load. The only way to do that is 
//       to split TESPluginFile's load process so that LoadOrder can load 
//       the header, check the masters, and then proceed with loading the 
//       file if there is no error.
//
//     - Modify loading code for forms and subrecords (e.g. Papyrus): never 
//       assert; instead, if a subrecord has invalid data, add its signature 
//       and contents to a list on LoadedForms::Form and abort loading of the 
//       subrecord (e.g. error anywhere in VMAD -> discard in-memory VMAD 
//       data).
//
//        - Audit all remaining calls to (assert) and (_DEBUGMSG).
//
//        - TESPluginSubrecord::back_to_start has been added for the purpose 
//          of letting a client go back to the start of the subrecord, in 
//          order to load its full contents after having read enough of it 
//          to know that it is malformed.
//
//     - We need to be able to set an active file.
//
//        - All forms and overrides loaded from the active file should be 
//          stored both in LoadOrder::formsByType and in a separate map 
//          just for active file forms, so that we know what forms to save 
//          when saving the active file.
//
//           - If something else overrides a form that the active file 
//             overrode -- that is, a form defined in A overridden by 
//             active file B and then overridden again by some file C 
//             later in the load order -- then we'll... need to do... ah, 
//             something, I think.
//
//              - Wondering if we should enforce the active file being 
//                the last file in the load order.
//
//        - Wondering if we should retain FormStubs for forms overridden by 
//          the active file; it would allow us to offer a "Revert" context 
//          menu item on all forms in the active file. Could give each 
//          FormStub a field (FormStub* overrides = nullptr).
//
//  - Use Info:
//
//     - Every FormStub needs two linked lists: one for outbounds references 
//       to other forms, and another for inbound references from other forms.
//
//     - We need to start adding more classes for loaded forms.
//
//        - Each class needs two load methods: an instance method that actually 
//          grabs all data for the form; and a static method that just sets up 
//          outbound references. The latter should just ignore subrecords that 
//          don't contain references to other forms; if a subrecord contains 
//          references to other forms as well as other data, skip bytes and pay 
//          attention only to those outbound form IDs.
//
//     - Once we have code to load at least *most* form types' outbound refs, 
//       we need to write code to actually *do* that i.e. code in LoadOrder to 
//       loop over every loaded form and set up Use Info. There are two ways we 
//       can do this:
//
//       BI-DIRECTIONAL (SINGLE-THREADED SINGLE-PASS):
//
//       In a single thread, loop over every FormStub. Set up both outbound and 
//       inbound references in one go. xEdit builds Use Info on a single thread 
//       and takes 66 seconds to do it, though I think they build Use Info for 
//       all files in the load order and not just conflict-winning records.
//
//       TWO-PASS (MULTI-THREADED OUTBOUND, SINGLE-THREADED INBOUND):
//
//       Divide the list of all forms into multiple sublists, and assign each 
//       sub-list to a thread. Each thread should work to build outbound refs. 
//       Then, after this operation is complete, use a single thread to go over 
//       the full list of forms and build inbound refs based on the outbound 
//       ref data.
//
//     - Once we have code to build Use Info, we'll need to be able to cache 
//       it for faster loading, like xEdit does. Cached use info for a file 
//       should match the filename and should be tagged with both a hash of 
//       the file's contents and the version of SkyEdit that generated it.
//
//  - Constraints:
//
//     - FOPEN_MAX, the maximum number of files we're allowed to have open 
//       via cstdio functions, is 20 in my dev environment -- not enough for 
//       the full load order. However, we don't open ESP files with fopen; we 
//       use Windows's "mapped file" API. Does that have a similar limit?
//
//  - Polishing:
//
//     - We need a way to reset LoadOrder, so that we can load a new order.
//
//        - Move the call to FormStubHeap::force_free_all from main.cpp to 
//          LoadOrder's reset method, once it HAS a reset method.
//



//
//  - Add a load order singleton.
//
//     - Like the Creation Kit, we will load a list of user-selected files, and 
//       one file may optionally be designated as the "active file" to which 
//       changes will be made.
//
//        - Do not allow the loading of any files that have the active file as 
//          a dependency. If they override records in the active file, things 
//          could get very messy -- particularly if any overrides refer to forms 
//          that exist only in the dependent file(s).
//
//  - cobb::wavl_tree
//
//     - We don't need, but may want, analogues to std::map::lower_bound and 
//       std::map::upper_bound.
//
//     - Consider renaming (value_type) to (mapped_type) and then typedeffing 
//       (value_type) to refer to the pair; this will be consistent with std::map.
//
//     - Do we want to alter iterators to satisfy LegacyBidirectionalIterator 
//       constraints? <https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator>
//
//  - Improvements to multi-threaded file loading:
//
//     - See if cobb::wavl_tree will produce a performance improvement.
//
//        - First, test to ensure that the methods added for STL parity work 
//          correctly -- iterators, at, operator[], etc.. If those aren't working 
//          properly, then we can't do a clean swap from std::map to wavl_tree.
//
//        - It almost certainly won't unless we hook it up to a multi-threaded 
//          block allocator. Here's how we can do that:
//
//           = Move cobb::wavl_tree::node to cobb::wavl_node (templated on the 
//             key and value) and add a typedef to the tree so we don't have to 
//             change everything from "node." This is needed so that we can even 
//             template the block allocator or anything else on the node type.
//
//           = Add two parameters to the wavl_tree template: non-member functions 
//             for allocating and deallocating a node. By default, they should be 
//             malloc and free wrapped to return the wavl_node type.
//
//              - Alternatively, have it take a std::allocator.
//
//           = Create a singleton multi-threaded block allocator templated on 
//             cobb::wavl_node<uint32_t, FormStub*>. Create non-member functions 
//             that get the allocator instance and call allocate and free as 
//             required.
//
//           = Typedef cobb::wavl_tree<uint32_t, FormStub*, my_malloc, my_free> 
//             as map_of_form_stubs and use that for TESPluginFile.
//
//           = Threads are already designed to register themselves with the block 
//             allocator for FormStubs. They should do the same for the singleton 
//             allocator for tree nodes.
//
//           - And then test it!
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
// OLD NOTES ON LOAD ORDERS AND USE INFO BELOW:
//

//
//  - Implement the handling of multiple TESPluginFiles as part of a load order: 
//    we need a singleton that represents the full load order, with a list of 
//    TESPluginFiles. The singleton should have a method to add files for loading, 
//    and should handle the loading of the files and all dependencies once told to 
//    load files.
//
//    The singleton should maintain a map of "effective forms," i.e. a map of form 
//    IDs to the FormStubs for conflict-winning (or non-conflicted) forms. The 
//    singleton should also identify an "active file," like the Creation Kit, for 
//    use with editing later on.
//
//  - Once the singleton is ready (and not before), we will be able to track Use 
//    Info for forms. Given how I plan on designing this editor, we only *need* 
//    Use Info for conflict-winning and non-conflicted forms, so once we have a 
//    full map of just those FormStubs, it will be all the simpler to construct 
//    the Use Info.
//
//     - Alternatively, we could have a map of FormStubCollection objects, which 
//       list all of the FormStubs for a form (including overridden ones) and 
//       the Use Info for the final loaded form (the conflict-winning or non-
//       conflicted form).
//
//        - Nope! FormStubCollection would have to have a vector, which we 
//          can't block-allocate. Just have FormStubFinal which has a FormStub 
//          pointer and room for additional data e.g. Use Info; the name conveys 
//          that it's for a conflict-winning or non-conflicted form.
//
//        - TESPluginFile::_insertForm needs to be modified to insert into the 
//          TESPluginFile AND to insert into the load order singleton's map of 
//          form IDs to forms. (Well, actually, it should pass the FormStub to 
//          a member function on the singleton which normalizes the form ID and 
//          checks if it's a new form or override and blah blah blah.) This will 
//          be MUCH faster than having to collate results from all files at the 
//          end of the full load process.
//
//     - I'm thinking we can load Use Info in two passes. The first pass involves 
//       splitting the list of forms across multiple threads (this REQUIRES a 
//       custom storage class; std::map CANNOT do this efficiently) and generating 
//       their outbound connections; the second pass is single-threaded and uses 
//       the existing outbound connection info to generate inbound connections.
//
//        - Parent/child relationships between forms should also be tracked as 
//          Use Info. This will require consulting GroupMetadata in addition to 
//          reading the form's record.
//
//        - The inbound and outbound connection lists should be linked lists and 
//          should be stored on the FormStubCollection.

//
//  OLD NOTES ON USE INFO:
//

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
//                the FormStub discard the rest of the loaded form data. ALTERNATIVELY, 
//                we could add a special load method for each loaded form class that 
//                skips bytes, paying attention ONLY to form IDs.
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

void test_print_load_error() {
   auto& d = LoadOrder::get().getError();
   if (d.defined()) {
      printf("Error type:  %s\n", d.code_string());
      if (!d.parseError.empty())
         printf("Error info:  %s\n", d.parseError.c_str());
      printf("File:        %s\n", d.file.c_str());
      if (!d.dependency.empty())
         printf("Dependency:  %s\n", d.dependency.c_str());
      printf("File offset: %X\n", d.fileOffset);
      printf("Form ID:     %08X\n", d.formID);
   } else
      printf("No details available!\n");
}

void test_print_quests() {
   auto& lo = LoadOrder::get();
   lo.forEachFormOfType(FormType::Quest, [](FormStub* stub) {
      auto form = stub->load();
      if (form && form->formType == FormType::Quest) {
         auto quest = form.ptr_cast<LoadedForms::Quest>();
         const char* type = LoadedForms::Quest::QuestTypeToString(quest->questType);
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
}
void test_print_actor_bases() {
   auto& lo = LoadOrder::get();
   lo.forEachFormOfType(FormType::ActorBase, [](FormStub* stub) {
      auto editorID = stub->get_editor_id();
      if (editorID)
         printf("[NPC_:%08X]%s\n", stub->formID, editorID);
      else
         printf("[NPC_:%08X] has no editor ID\n", stub->formID);
      return false;
   });
}

void test_skyrim() {
   auto& lo = LoadOrder::get();
   lo.basePath = TEST_PLUGIN_PATH;
   lo.addFile("Skyrim.esm");
   struct timeb bench_start;
   struct timeb bench_end;
   ftime(&bench_start);
   bool result = lo.loadQueuedFiles();
   ftime(&bench_end);
   printf("Loaded Skyrim.esm.\n");
   printf("Time taken: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   if (result) {
      test_print_quests();
      test_print_actor_bases();
   } else {
      printf("...But an error was encountered during load! Details:\n");
      test_print_load_error();
   }
   lo.reset();
}
void test_hearthfire() {
   auto& lo = LoadOrder::get();
   lo.basePath = TEST_PLUGIN_PATH;
   //lo.addFile("Skyrim.esm");
   lo.addFile("Update.esm");
   lo.addFile("HearthFires.esm");
   struct timeb bench_start;
   struct timeb bench_end;
   ftime(&bench_start);
   bool result = lo.loadQueuedFiles();
   ftime(&bench_end);
   printf("Loaded Skyrim.esm, Update.esm, and HearthFires.esm.\n");
   printf("Time taken: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   if (result) {
      auto hf_form = lo.getForm(0x020008CE);
      if (hf_form)
         printf("Found HearthFires form xx0008CE.\n");
      else
         printf("Unable to find HearthFires form xx0008CE; expected final form ID 0x020008CE with local form ID 0x010008CE.\n");
      //
      auto hf_override = lo.getForm(0x0010B035);
      if (hf_override) {
         std::string fn;
         hf_override->get_source_filename(fn);
         if (fn.empty())
            printf("Unable to identify which file form 0x0010B035 was finally loaded from.\n");
         else
            printf("Form 0x0010B035, known to be defined in Skyrim.esm and overridden by HearthFires.esm, registers as being from file %s.\n", fn.c_str());
      } else
         printf("Unable to find Skyrim.esm form 0x0010B035 known to be overridden by HearthFires.esm.\n");
   } else {
      printf("...But an error was encountered during load! Details:\n");
      test_print_load_error();
   }
   lo.reset();
}

void test_errors() {
   auto& lo = LoadOrder::get();
   lo.basePath = std::filesystem::current_path().string();
   lo.basePath += "/tests/";
   //
   printf("TESTING ERROR HANDLING...\n");
   printf("Base path: %s\n", lo.basePath.c_str());
   {
      printf("\nLoading empty file...\n");
      lo.addFile("empty.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading cyclical-dependency file...\n");
      lo.addFile("Cyclical01.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading missing-dependency file...\n");
      lo.addFile("MissingDependency.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading file truncated in the header...\n");
      lo.addFile("TooShort.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading file with bad extended subrecord...\n");
      lo.addFile("BadExtendedSubrecord.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading file with a byte missing (test for suspicious signatures, etc.)...\n");
      lo.addFile("OneByteMissing.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading file with an unknown record type...\n");
      lo.addFile("UnknownRecordSignature.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   {
      printf("\nLoading file with a record that claims to be 4GB...\n");
      lo.addFile("RecordClaimsToBeHuge.esp");
      bool result = lo.loadQueuedFiles();
      test_print_load_error();
      lo.reset();
   }
   printf("\nDONE TESTING ERROR HANDLING.\n");
}

int main() {
   main_thread_id = std::this_thread::get_id();
   //
   auto& lo = LoadOrder::get();
   test_errors();
   printf("\nTEST 1:\n");
   test_skyrim();
   printf("\nTEST 2:\n");
   test_hearthfire();
   //
   return 0;
}