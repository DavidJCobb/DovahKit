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
   class file_load_order;
   class form_stub;
}

namespace dovah::tes_file_reading {
   class file_part_loader;
   class file_threaded_part_loader_base;

   class form_stub_build_interface;

   class file_loader : file_or_file_part_loader {
      using interface_t   = load_order_interfaces::file_load;
      using flag          = tes_file_flag;
      using detail_flag   = tes_file_header::detail_flag;
      using detail_flag_t = tes_file_header::detail_flag_t;
      friend class form_stub_build_interface;
      protected:
         virtual file_loader& get_file_loader() const noexcept override final { return const_cast<file_loader&>(*this); }
      public:
         file_loader(interface_t&); // TODO: threaded loaders should be created in the constructor, and so should never be nullptr during this object's lifetime
         ~file_loader();
         //
         tes_file_header header;
         localized_string_store* localization_data = nullptr; // pointer is owned by the (file_loader) once received
         //
         void  abort() noexcept;
         float assess_load_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet
         bool  fetch_record_header(uint32_t pos, tes_file_record_header&, uint32_t& record_decompressed_size);
         std::string get_filename() const noexcept;
         //
         inline bool is_aborted() const noexcept { return this->aborted; }
         //
         inline bool is_light() const noexcept { return this->header.is_light(); }
         //
         bool load(const std::filesystem::path&); // path is optional; if empty, reuses prior path (if any). calling this while a load is already in progress is undefined behavior
         void close(); // intended for use during the save process, with the file then being reopened by the caller upon a successful save
         //
      protected:
         std::filesystem::path path;
         cobb::mapped_file     file;
         std::vector<file_threaded_part_loader_base*> threads; // array elements should never be nullptr after the instance is constructed.
         bool aborted = false; // TODO: make atomic?
         //
         bool _open_mapped_file();
         bool _load_header();
         //
         bool _start_threads();
         bool _wait_for_threads();
         file_threaded_part_loader_base* _get_nth_thread_of_type_impl(const std::type_info&, size_t);
         //
         template<class loader_type> loader_type* _get_nth_thread_of_type(uint32_t& s) {
            if (s >= threaded_loader_recommended_thread_count<loader_type>)
               s -= threaded_loader_recommended_thread_count<loader_type>;
            auto* t = this->_get_nth_thread_of_type_impl(typeid(loader_type), s);
            ++s;
            return (loader_type*)t;
         }
         //
      public:
         inline const cobb::mapped_file& get_raw_mapped_file() const noexcept { return this->file; };
   };
}