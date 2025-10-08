#pragma once
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>
#include "helpers/performance.h"

namespace dovah {
   class bsa_archive; // a single BSA file
   class bsa_archived_file; // a single file IN a BSA
   class bsa_threaded_reader;

   //
   // NOTES:
   //
   //  - (bsa_archive) represents a single BSA file. That file need not have actually been loaded 
   //    yet and need not even have been confirmed to exist; compare to (file_reader).
   //
   //  - Skyrim uses single-byte-encoded strings for file paths and names, so we do as well.
   //
   //  - (bsa_archived_file) is the interface for a file that has been loaded from a BSA, ready 
   //    for a caller to view. That is to say: a BSA archive contains file data along with a list 
   //    of files, with each list item containing the filename or name hash, the file size, and 
   //    the offset of its data, among other metadata. The (bsa_archived_file) class DOES NOT 
   //    represent an entry in that list (i.e. metadata) but rather the fully loaded data for a 
   //    file.
   //
   //  - (bsa_archived_file) should have file content in a (cobb::generic_buffer), and should have 
   //    a bool indicating whether it "owns" the file content. If we're loading a file from an 
   //    uncompressed BSA, then the generic_buffer should be pointed directly into the mapped BSA 
   //    file and should be considered unowned. If we're loading a loose file or a file in a BSA 
   //    that has been compressed, then the generic_buffer needs to contain a copy of the file 
   //    content, and should be considered owned (meaning that the bsa_archived_file needs to 
   //    free it when destroyed).
   //
   //    This implies that a (bsa_archived_file) does not need to know about its owning archive. 
   //    It only needs to know if it's sharing its buffer with that owner, so that it knows 
   //    whether to delete its buffer during teardown.
   //
   //  - It is possible for a (file_load_order) to "adopt" a (bsa_load_order). The file load 
   //    order should contain a private pointer to a BSA load order which defaults to null. 
   //    When a BSA load order is adopted, the following things should happen:
   //
   //     - At the start of the ES[LPM] load process, the file_load_order prepends all needed 
   //       BSA files to the bsa_load_order, i.e. the BSAs specified in Skyrim.ini followed by 
   //       any extant BSAs for ES[LPM] files being loaded.
   //
   //     - At the start of the ES[LPM] load process, the file_load_order starts the BSA load 
   //       process, which will run on multiple threads.
   //
   //     - At the end of the ES[LPM] load process, the file_load_order will block until the 
   //       BSA load process is complete.
   //
   //    Why do we prepend the needed files to the BSA load order? So that the BSA load order 
   //    can potentially be configured with extra BSAs to load. That is to say: it'd be nice 
   //    if we could allow the user to select "extra BSAs" to load in addition to the BSAs 
   //    needed for their load order. This would allow the user to see things like texture 
   //    mods (if they're archived) in the editor without having to load the corresponding 
   //    ES[LPM] files (which would cause those files to become masters of the active file). 
   //    We could even add config options somewhere to let the user specify BSAs that should 
   //    always be loaded as "extra" (provided they wouldn't already be loaded by virtue of 
   //    being in the ES[LPM] load order).
   //
   //     - For this reason, as well, the file_load_order's pointer to the bsa_load_order 
   //       should be accessible to frontend code.
   //

   class bsa_load_order {
      public:
         using archive_list = std::vector<bsa_archive*>;
         struct load_options_t {
            int  thread_count                = 4;     // number of threads to use
            bool abort_all_on_any_error      = true;  // if any archive fails to load, abort all loading
            bool missing_archive_is_an_error = false; // control whether a BSA not being present counts as an error
         };
         //
      protected:
         archive_list          archives;
         std::filesystem::path base_path; // compare to file_load_order::base_path
         load_options_t        working_load_options; // copy of (load_options) created at the start of the load process, so that other threads can't screw with the options mid-load
         //
         std::vector<bsa_threaded_reader*> threads;
         std::mutex      completion_check_lock;
         bool loading = false;
         bool aborted = false; // gets set to (true) if the load process is aborted; threaded readers will need to check this periodically
         //
         bsa_archive* _make_archive(const std::filesystem::path& name);
         //
      public:
         ~bsa_load_order(); // TODO: should assert if a load is still in progress; should delete archives
         
         load_options_t load_options;
         struct {
            cobb::benchmark loading  = { 0 };
            cobb::benchmark blocking = { 0 };
         } benchmarks;
         
         bsa_archived_file* lookup_file(const std::string& path_and_name, bool check_for_loose_file = true) const;
         
         constexpr const std::filesystem::path& get_base_path() const noexcept { return this->base_path; }
         void set_base_path(const std::filesystem::path&);
         void append_archive(const std::filesystem::path& name);
         void prepend_archive(const std::filesystem::path& name);
         void insert_archive(int index, const std::filesystem::path& name); // negative indices are relative to the end of the list

         //
         // Remove an archive if it hasn't loaded yet or if (even_if_loaded) is (true). If 
         // (delete_archive) is true, then the archive is deleted and the function always 
         // returns nullptr; otherwise, it returns the archive (if any) if it is removed, 
         // or nullptr if there is no such archive or if the archive is not removed.
         //
         // If a removed archive is returned instead of deleted, then the BSA load order 
         // has relinquished ownership of that archive and all its contents.
         //
         bsa_archive* remove_archive(const std::filesystem::path& name, bool even_if_loaded, bool delete_archive);
         
         constexpr const archive_list& get_archive_list() const noexcept { return this->archives; }
         
         void load_archives(); // asynch and multi-threaded
         void wait_for_archive_load_to_finish(); // blocks on the calling thread until all archives have either loaded, failed, or have responded to the load process being aborted
         void abort_archive_load();
         bool is_archive_load_aborted() const noexcept;
         bool is_archive_load_in_progress() const noexcept; // if an abort is called, this returns true until all threads have reacted to that abort

         void on_thread_complete(bsa_threaded_reader&);
   };
}