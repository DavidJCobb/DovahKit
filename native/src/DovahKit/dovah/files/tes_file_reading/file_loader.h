#pragma once
#include <filesystem>
#include <typeinfo>
#include "../../../helpers/files.h"
#include "../common.h"
#include "file_or_file_part_loader.h"
#include "file_threaded_part_loader_base.h"
#include "threads.h"
#include "../../localization/localized_string_store.h"
#include "../file_header.h"
#include "../file_load_order.h"

namespace dovah {
   class form_stub;
   namespace tes_file_writing {
      class file_writer;
   }
}

namespace dovah::tes_file_reading {
   class file_part_loader;
   class file_threaded_part_loader_base;

   class file_loader : public file_or_file_part_loader {
      using interface_t = load_order_interfaces::file_load;
      public:
         using flag          = tes_file_flag;
         using detail_flag   = tes_file_header::detail_flag;
         using detail_flag_t = tes_file_header::detail_flag_t;
         
         file_loader(interface_t&); // TODO: threaded loaders should be created in the constructor, and so should never be nullptr during this object's lifetime
         ~file_loader();
         
         tes_file_header header;
         localized_string_store* localization_data = nullptr; // pointer is owned by the (file_loader) once received
         
         float assess_load_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet
         bool  fetch_record_header(uint32_t pos, tes_file_record_header&, uint32_t& record_decompressed_size);
         std::string get_filename() const noexcept;
         //
         constexpr bool is_light()  const noexcept { return this->header.is_light(); }
         constexpr bool is_master() const noexcept { return this->header.is_master(); }
         constexpr bool uses_string_table() const noexcept { return this->header.flags & flag::localized_string_table; }
         
         bool load(const std::filesystem::path&); // path is optional; if empty, reuses prior path (if any). calling this while a load is already in progress is undefined behavior
         void close(); // intended for use during the save process, with the file then being reopened by the caller upon a successful save
         void reopen(); // reopen the mapped file, without actually loading its contents; intended for use during the save process. can throw.
         void reopen(const std::filesystem::path&); // reopen with an alternate path (e.g. if we couldn't save to the desired filename)
         
      protected:
         std::filesystem::path path;
         cobb::mapped_file     file;
         struct {
            bool light  = false;
            bool master = false;
         } effective_flags;
         std::vector<file_threaded_part_loader_base*> threads; // array elements should never be nullptr after the instance is constructed.
         bool aborted = false; // TODO: make atomic?
         //
         void _open_mapped_file(const std::filesystem::path&);
         bool _load_header();
         //
         void _set_filename(const std::filesystem::path& desired, const std::filesystem::path& actual);
         //
         void _start_threads();
         void _wait_for_threads();
         file_threaded_part_loader_base* _get_nth_thread_of_type_impl(const std::type_info&, size_t);
         //
         template<class loader_type> loader_type* _get_nth_thread_of_type(uint32_t& s) {
            if (s >= loader_type::recommended_thread_count)
               s -= loader_type::recommended_thread_count;
            auto* t = this->_get_nth_thread_of_type_impl(typeid(loader_type), s);
            ++s;
            return (loader_type*)t;
         }
         //
      public:
         inline const cobb::mapped_file& get_raw_mapped_file() const noexcept { return this->file; };
         //
         void adopt(basic_reader&) const noexcept; // set the passed-in reader to act on this file's loaded contents
         //
         struct save_interface {
            friend file_loader;
            protected:
               file_loader& wrapped;
               save_interface(file_loader& r) : wrapped(r) {}
            public:
               inline const void* data_at(std::ptrdiff_t o) const noexcept {
                  return this->wrapped.file.data_at(o);
               }
               void update_path(const std::filesystem::path& desired, const std::filesystem::path& actual) const noexcept;
         };
         save_interface get_save_interface(const tes_file_writing::file_writer&) noexcept { return save_interface(*this); }
         interface_t    get_load_interface(const file_or_file_part_loader&) noexcept { return this->load_interface; }
   };
}