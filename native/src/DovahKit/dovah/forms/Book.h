#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Book : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::book;
         Book(const constructor_params& c) : Form(form_type, c) {};

         struct book_flag {
            enum type : uint8_t {
               teaches_skill   = 1 << 0, // we only use this during serialization; clients should [gs]et `teaches` instead
               cannot_be_taken = 1 << 1,
               teaches_spell   = 1 << 2, // we only use this during serialization; clients should [gs]et `teaches` instead
            };
         };
         using book_flags_t = std::underlying_type_t<book_flag::type>;

         enum class book_type : uint8_t {
            tome = 0,
            note = 1,
         };

      public:
         components::object_bounds bounds; // OBND
         components::keyword_list keywords; // KSIZ+KWDA
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         //
         localized_string name = localized_string(localized_string_type::common);      // FULL
         localized_string text = localized_string(localized_string_type::description); // DESC
         //
         struct {
            std::string inventory; // ICON
            std::string message;   // MICO
         } icons;
         struct {
            form_reference_t take; // YNAM
            form_reference_t drop; // ZNAM
         } sounds;
         book_flags_t     flags = 0;    // DATA+0x00
         book_type        type  = book_type::tome; // DATA+0x01
         std::variant<
            int32_t,         // actor value index
            form_reference_t // SPEL
         > teaches = -1; // DATA+0x04
         int32_t          value  = 0;   // DATA+0x08
         float            weight = 0;   // DATA+0x0C
         form_reference_t menu_display_object; // INAM -> STAT
         localized_string description = localized_string(localized_string_type::description); // CNAM // shown in the item card

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}