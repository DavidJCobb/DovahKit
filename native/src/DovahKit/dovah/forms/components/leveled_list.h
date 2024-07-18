#pragma once
#include "../_common.h"
#include <array>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>
#include "helpers/compile_time_strings/cs.h"
#include "helpers/type_traits/is_std_array.h"
#include "dovah/data/all_carryable_form_types.h"
#include "../structs/container_object_extra_data.h"

namespace dovah::loaded_forms::components {
   class leveled_list {
      public:
         static constexpr const auto relevant_subrecords = std::array{
            'LVLD', // base data
            'LVLF', // flags
            'LVLG', // chance-none global
            'LLCT', // entry count (used to reserve memory early; not mandatory)
            'LVLO', // entry
            'COED', // entry COED
         };

         using length_type = uint8_t;
         static constexpr const size_t max_entry_count = std::numeric_limits<length_type>::max();

         struct flag {
            flag() = delete;
            enum type : uint8_t {
               calculate_from_all_levels_below_player = 0x01,
               calculate_for_each_item_in_count       = 0x02,
               use_all      = 0x04,
               special_loot = 0x08,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct entry {
            form_reference_t form  = {};
            uint16_t         count = 1;
            uint16_t         level = 1;
            std::optional<structs::container_object_extra_data> item_extra_data;
         };

         // For use by the function to preview a leveled list.
         struct generated_preview_entry {
            form_stub* form   = nullptr;
            uint32_t   count  = 0;
            float      health = 1;
            form_stub* owner  = nullptr;
         };

         // For use by the containing form's `generate_use_info` function.
         struct use_info_builder {
            bare_form_id_t global = 0;

            use_info_builder(form_stub_use_info_builder& owner) : owner(owner) {}

            form_stub_use_info_builder& owner;

            void done();
         };

      protected:
         leveled_list() {} // Require subclasses which set the allowed form types.

         struct {
            //
            // Storing the allowed form types in this manner means we can keep them as 
            // constexpr arrays shared across all leveled lists of like type, rather 
            // than each leveled list instance having to keep its own std::vector, etc..
            //
            const form_type* list  = nullptr;
            size_t           count = 0;
         } _allowed_form_types;
         const char* _level_difference_gmst_name = nullptr;
         struct {
            size_t last_loaded_entry = (size_t)-1;
         } _cross_subrecord_load_state;

         template<size_t Count>
         constexpr void _set_allowed_form_types(const std::array<form_type, Count>& src) {
            if (src.size())
               this->_allowed_form_types = decltype(_allowed_form_types){ src.data(), src.size() };
         }

      public:
         flags_t flags = 0;
         std::vector<entry> entries;
         struct {
            form_reference_t global     = {}; // value of the global should be in the range [0.0, 100.0] and will be truncated
            uint8_t          percentage = 0;
         } chance_none;

         bool allows_form_type(form_type) const;
         [[nodiscard]] std::vector<form_type> legal_form_types() const;
         constexpr const char* get_level_difference_setting_name() const {
            return this->_level_difference_gmst_name;
         }
         
      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_record_writer&, load_order_interfaces::form_save&);
         static void generate_use_info(tes_subrecord_reader&, use_info_builder&);
         void clone_from(const leveled_list& original, loaded_forms::Form& my_containing_form) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);
   };

   namespace _impl::_leveled_list {
      template<const auto& Array, const cobb::cs GMST> requires cobb::is_std_array<std::decay_t<decltype(Array)>>
      class leveled_list_template : public leveled_list {
         protected:
            static constexpr const auto _allowed = Array;

         public:
            leveled_list_template() {
               this->_set_allowed_form_types(_allowed);
               this->_level_difference_gmst_name = GMST.c_str();
            }
      };

      constexpr const auto character_types = std::array{ form_type::actor_base, form_type::leveled_character };
      constexpr const auto spell_types     = std::array{ form_type::spell,      form_type::leveled_spell };
   }
   
   using leveled_character_list = _impl::_leveled_list::leveled_list_template<_impl::_leveled_list::character_types, cobb::cs("iLevCharLevelDifferenceMax")>;
   using leveled_item_list      = _impl::_leveled_list::leveled_list_template<all_carryable_form_types,              cobb::cs("iLevItemLevelDifferenceMax")>;
   using leveled_spell_list     = _impl::_leveled_list::leveled_list_template<_impl::_leveled_list::spell_types,     cobb::cs("")>;
}