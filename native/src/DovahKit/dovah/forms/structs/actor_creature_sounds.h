#pragma once
#include "../_common.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

namespace dovah {
   enum class creature_sound_type : uint32_t {
      idle           = 0,
      aware          = 1,
      attack         = 2,
      hit            = 3,
      death          = 4,
      weapon         = 5,
      movement_loop  = 6,
      conscious_loop = 7,
   };
}

namespace dovah::loaded_forms::structs {
   //
   // An ActorBase can define a list of creature sounds, or choose to inherit the sounds 
   // from another ActorBase. (It cannot do both.)
   // 
   // Entries that specify a None form are not saved.
   //
   class actor_creature_sounds {
      public:
         static constexpr const uint32_t subrecord_signature_inherit      = 'CSCR';
         static constexpr const uint32_t subrecord_signature_sound_start  = 'CSDT';
         static constexpr const uint32_t subrecord_signature_sound_form   = 'CSDI';
         static constexpr const uint32_t subrecord_signature_sound_chance = 'CSDC';

         struct entry {
            creature_sound_type type   = creature_sound_type::idle;
            form_stub*          sound;
            uint8_t             chance = 100; // chance to play (percentage)
         };

         class use_info_builder {
            public:
               use_info_builder(form_stub_use_info_builder& owner) : owner(owner) {}

               form_stub_use_info_builder& owner;
               bare_form_id_t inherit_from = 0;
               std::vector<bare_form_id_t> sounds;
               bool pending_entry_is_valid = false;

            public:
               void done();
         };

      protected:
         struct _stored_entry {
            creature_sound_type type   = creature_sound_type::idle;
            form_reference_t    sound;
            uint8_t             chance = 100; // chance to play (percentage)
         };

         form_reference_t _inherit_from = nullptr;
         std::vector<_stored_entry> _own_sounds;

         struct {
            std::optional<entry> pending_entry;
         } _loader_state; // used to track loader state across subrecords; not safe to read from otherwise

      public:
         form_stub* inherits_from() const;
         [[nodiscard]] std::vector<entry> sounds() const;

         void set_inherits_from(loaded_forms::Form& my_containing_form, dovah::form_stub*);

         void add_sound(loaded_forms::Form& my_containing_form, creature_sound_type, dovah::form_stub*, uint8_t chance = 100);
         void replace_sounds(loaded_forms::Form& my_containing_form, const std::vector<entry>&);
         size_t sound_count() const;
         
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
         void save(tes_record_writer&, load_order_interfaces::form_save&) const;
         static void generate_use_info(tes_subrecord_reader&, use_info_builder&);
         void clone_from(const actor_creature_sounds& original, loaded_forms::Form& my_containing_form) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);
   };
}