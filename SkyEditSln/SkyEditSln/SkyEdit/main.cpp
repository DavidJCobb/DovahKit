#include <iostream> // for testing
#include <sys/timeb.h> // for benchmarks
#include <thread> // for std::thread::id
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
//    I should give that a try -- map the entire Skyrim.esm file into memory 
//    and then run through it. If I just change the underlying file handle and 
//    fread stuff, then it shouldn't even be all that difficult to test.
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
//     - The std::string class does not enforce the presence or absence of a null 
//       terminator. This is preventing us from using std::string::operator== to 
//       compare certain strings. Specifically, a typical string (i.e. anything 
//       from user input or a string literal in our code) is going to have a null 
//       terminator, whereas anything from a subrecord is going to lack a null 
//       terminator. This causes the two kinds of strings to have different lengths, 
//       and since std::string::operator== checks the lengths first as a shortcut, 
//       the strings in question won't compare equal even though they are equal. 
//       The fix to this would be to have TESPluginSubrecord::to_string forcibly 
//       add a null terminator to the output std::string.
//
//        - Once we've fixed this, have LoadOrder::indexOf use operator== again.
//
//           - Actually, in our string helper file, add a std::string equivalent to 
//             stricmp that compares lengths and then runs strnicmp against the 
//             two strings' data. Use that for LoadOrder::indexOf, since I don't 
//             think the game itself enforces case-sensitivity on masters.
//
//  - Loading:
//
//     - The TESPluginFile reading code should fail if we encounter a group 
//       nested too deeply (i.e. deeper than MAX_ESP_FILE_GROUP_DEPTH).
//
//     - Add a method TESPluginFile::abort. The method should set a flag on the 
//       TESPluginFile indicating that loading needs to abort and then, if the 
//       file is doing multi-threaded loading, wait for the threads to join. The 
//       threaded reader classes, meanwhile, need to be modified to check that 
//       flag on a regular basis and abort if they see that it's been set.
//
//     - We need to account for unexpected masters. For example, if you ask the 
//       LoadOrder singleton to load ONLY Update.esm, then Skyrim.esm will be an 
//       unexpected master and we must add it to the load order.
//
//       I think we should make a class that loads file headers (do NOT use 
//       TESPluginFile for this -- you'll see why) and then, when we ask to load 
//       the queued set of files, have LoadOrder go through each queued file and 
//       work to build a "final" load order. If, when loading file headers, it 
//       finds unexpected masters, it can just add those to the final load order 
//       before it adds the current queued file. Once the final load order is 
//       built, we create TESPluginFiles for everything in it.
//
//       With that done, we'll want to modify TESPluginFile's method for loading 
//       the file header and have that fail with an error if it encounters any 
//       new unexpected masters (which would indicate that the files were 
//       tampered with during our load).
//
//        - Files with an ESM extension and files that are flagged as masters 
//          (and any dependencies of such files, even ESPs) always load first. 
//          When we're putting the load order together, we should probably 
//          first sort all files into a "master" bucket and a "non-master" 
//          bucket, and THEN construct the final load order based on that. 
//          That is:
//
//           - Read a queued file's header.
//
//           - Retrieve all masters of the queued file, and their masters, 
//             and so on, by reading their headers.
//
//           - If the original queued file was a master, then all found files 
//             go into the "master" bucket. Otherwise, any masters among the 
//             found files (and their dependencies) go into the "master" 
//             bucket and the rest go into the "non-master" bucket.
//
//              - If a file already exists in the "non-master" bucket and it 
//                needs to be in the "master" bucket, move it. If a file is 
//                about to be added to the "non-master" bucket but it already 
//                exists in the "master" bucket, then don't put it into the 
//                "non-master" bucket.
//
//           - The final load order is just the "master" bucket and the "non-
//             master" bucket taken together, end-to-end.
//
//     - We need to figure out how to get the loading code to actually signal 
//       errors to LoadOrder. In every place where we log a debug message and 
//       return false, we must instead send error details to the LoadOrder 
//       singleton.
//
//       I need to audit the relevant code -- make sure that any resources we 
//       acquire are properly released in the event of failure. After that, I 
//       need to decide on exactly how I'm going to handle load failures. We 
//       could use C++ exceptions, but then in order to release resources we 
//       have to catch and then rethrow the exceptions. Hmm...
//
//        - For multi-threaded loading, we also need to get the faulting 
//          thread to call the (abort) method on its owning TESPluginFile.
//
//        - Once we have this in place, audit all assertions and see how many 
//          should be converted to errors that we handle through LoadOrder.
//
//     - We need to be able to set an active file.
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



//
//  - Add a load order singleton.
//
//     - Instead of storing FormStubs per TESPluginFile, only store them on the 
//       load order singleton; as such, we'll keep only the last-loaded record 
//       with a given form ID. The changes involved here should be fairly minimal.
//
//        - We already intended to only build Use Info for last-loaded forms and 
//          to only use load-loaded forms in the UI, so if we have TESPluginFile 
//          keep the FormStubs, then we'll just have tons of useless stubs in 
//          memory.
//
//        - Add to FormStub two linked lists of connections, one for outbound 
//          references to other forms and the other for inbound references from 
//          other forms.
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
//        - In order to be able to see file header details in advance, we should 
//          make it so that TESPluginFile instances can be told to load just 
//          their header, i.e. we create one TESPluginFile instance for each 
//          file we MAY load and have it load its header so we can see its 
//          dependencies and so on; and then if we decide we want to actually 
//          load that file, we just reuse the same instance.
//
//           - NO NO NO. TESPluginFile maps files into memory; if we do it 
//             this way, then we're mapping all of the ESP files into memory 
//             before we even load them AND we're locking the files! Make a 
//             separate class to read just headers; remember what dependencies 
//             a file claimed to have; and then if we load that file with a 
//             TESPluginFile, fail if its dependencies have changed from 
//             what we saw earlier.
//
//  - cobb::wavl_tree
//
//     - We should add an analogue to std::map::clear.
//
//     - Can we add an analogue to std::map::swap and/or std::swap support?
//
//     - We should add an analogue to std::map::contains.
//
//     - For completeness' sake we may want an analogue to std::map::find, which 
//       looks up a node and returns an iterator to it. We don't need, but may 
//       want, analogues to std::map::lower_bound and std::map::upper_bound.
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
//             change everything from "node."
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

int main() {
   main_thread_id = std::this_thread::get_id();
   //
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
   printf("Loaded Skyrim.esm and Update.esm.\n");
   printf("Time taken: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   if (result) {
      //test_print_quests();
      //test_print_actor_bases();
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
      printf("...But an error was encountered during load!");
      //
      // TODO: display error
      //
   }
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