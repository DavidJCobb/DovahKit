#pragma once
#include <atomic>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include "lu/enums/enum_flags.h"
#include "lu/memory/threaded_slab_allocator.h"
#include "./_passkeys/build_use_info_during_load.h"
#include "./types/bare_form_id.h"
#include "./use_info/entry.h"
namespace dovah {
   namespace _passkeys {
      class add_form_stub_source_file;
   }
   namespace load_order_interfaces {
      class form_load;
   }
   namespace tes {
      class file;
      class file_view;
      class record_view;
      enum class record_flag : uint32_t;
   }
   class active_load_order;
   class base_managed_form_data;
   class form_stub_addenda;
   class form_stub_collection;
   enum class form_type : uint8_t;
}

namespace dovah {
   class form_stub {
      private:
         using use_info_list = std::map<
            bare_form_id,
            use_info::entry,
            std::less<bare_form_id>,
            lu::memory::threaded_slab_allocator<std::pair<const bare_form_id, use_info::entry>, 64000>
         >;

      public:
         enum class flag {
            is_edited,
            is_hardcoded,
            has_multiple_source_files,
         };
         using flags_type = lu::enum_flags<flag, 8>;

         struct file_data {
            tes::file* pointer       = nullptr;
            uint32_t   record_offset = 0; // for a dummy file, this is always 0
            uint32_t   record_flags  = 0;

            constexpr operator bool() const noexcept { return this->pointer != nullptr; }
         };
         struct file_data_list {
            file_data* entries = nullptr;
            uint16_t   count   = 0;
         };

      public:
         form_stub();
         ~form_stub();

      protected:
         union {
            file_data      file;
            file_data_list files;
         };
         base_managed_form_data* loaded   = nullptr;
         std::atomic<uint32_t>   refcount = 0; // of loaded form data
      public:
         form_stub_addenda* addenda   = nullptr;
         bare_form_id       form_id   = 0;
         enum form_type     form_type = {};
         flags_type         flags;
         // two bytes of padding available here to use
         std::string        editor_id;
         use_info_list      inbound;  // other forms that use this form.  entry flags describe `this`, the used form.
         use_info_list      outbound; // other forms that this form uses. entry flags descirbe `this`, the user form.

      public: // member functions
         bool can_unload_form() const noexcept;
         const active_load_order& get_load_order() const;
         active_load_order& get_load_order();
         constexpr bool is_deleted()   const noexcept; // record flag
         constexpr bool is_edited()    const noexcept;
         constexpr bool is_hardcoded() const noexcept;
         bool is_injected() const noexcept;
         bool is_none_stub() const noexcept;
         void set_edited(bool);

         form_stub_addenda& get_or_create_addenda();

         #pragma region Record flag functions
            void edit_record_flags(uint32_t mask, bool clear_or_set) noexcept; // also sets the form as edited
            constexpr uint32_t get_record_flags() const noexcept;
            constexpr bool test_record_flags(uint32_t mask) const noexcept;
            constexpr bool test_record_flags_for_file(uint32_t mask, int16_t file_index) const noexcept;
            constexpr bool test_record_flags_for_file(uint32_t mask, const tes::file&) const noexcept;
         #pragma endregion
         #pragma region Source file member functions
            constexpr bool file_list_includes(const tes::file&) const noexcept;
            constexpr tes::file* get_file_at_index(int16_t) const noexcept;
            constexpr uint32_t get_record_offset(int16_t file_index = -1) const noexcept;
            constexpr const file_data* get_source_file_info(int16_t file_index = -1) const noexcept;
            constexpr bool has_source_files() const noexcept;
            constexpr bool has_multiple_source_files() const noexcept;
            constexpr int16_t index_of_file(const tes::file&) const noexcept;
            constexpr int16_t source_file_count() const noexcept;
         #pragma endregion

         template<typename Callable>
            requires requires(Callable&& callable, tes::record_view& record, load_order_interfaces::form_load& intfc) {
               { callable(record, intfc) };
            }
         void for_each_source_record(Callable&& callable);

      protected:
         tes::file_view _open_record(const tes::file&, uint32_t record_offset) const noexcept;
         void _unload_form();

         #pragma region Source file member functions (internal)
            void _add_source_file(tes::file&, uint32_t record_offset, uint32_t record_flags = 0);
         #pragma endregion

      public: // passkeyed
         void _add_source_file(_passkeys::add_form_stub_source_file, tes::file&, uint32_t record_offset, uint32_t record_flags = 0);
         void _add_one_way_outbound_use(_passkeys::build_use_info_during_load, form_stub&, use_info::entry_flag_underlying_type flags = 0);
         void _add_one_way_outbound_use(_passkeys::build_use_info_during_load, bare_form_id, use_info::entry_flag_underlying_type flags = 0);
         void _set_parent_form_one_way(_passkeys::build_use_info_during_load, form_stub& parent);
   };
}

#include "./form_stub.inl"