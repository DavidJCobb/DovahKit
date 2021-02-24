#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "../_common.h"

class TESPluginFile;

// Macro used to optimize the building of Use Info for VMAD subrecords. Some Papyrus fragment data 
// types don't contain any form IDs, so if we know that the fragment data is always the last thing 
// in the VMAD subrecord, then we can just early-out for these types. If Skyrim Special is ever 
// updated to put additional data after the fragment data, you'll want to set this macro to 0 to 
// re-enable the code for skipping fragment data types that lack form IDs "by hand."
#define PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD 1

namespace dovah::loaded_forms::components {
   namespace papyrus {
      enum class fragment_type : uint8_t {
         undefined,
         info,
         package,
         perk,
         quest,
         scene,
      };
      enum class property_type : uint8_t {
         object  = 1,
         string  = 2,
         integer = 3,
         float32 = 4,
         boolean = 5,
         array_of_object  = 11,
         array_of_string  = 12,
         array_of_integer = 13,
         array_of_float32 = 14,
         array_of_boolean = 15,
      };
      inline bool property_type_is_array(property_type p) { return (int)p > 10; }
      
      class script_data;
      class perk_entry_fragment;
      class quest_alias_script_data;
      class quest_log_entry_fragment;
      class scene_phase_fragment;

      //
      // The VMAD subrecord contains both the script data for a whole form, and the script data for 
      // "sub-forms," such as log entries within a quest. In order to keep these sub-forms manage-
      // able, we allow forms to claim ownership of them, physically moving them outside of the 
      // script-data structure and into whatever part of the form they actually belong to. However, 
      // this means that when saving the VMAD subrecord, we need to be able to grant the script-data 
      // structure access to this "sub-form script data" so that it can be written to the subrecord.
      //
      struct script_data_save_parameters {
         std::vector<const quest_alias_script_data*>  aliases;
         std::vector<const quest_log_entry_fragment*> log_entry_fragments;
         std::vector<const perk_entry_fragment*>      perk_entry_fragments;
         std::vector<const scene_phase_fragment*>     scene_phase_fragments;
         //
         static const script_data_save_parameters default;
      };

      class basic_fragment_data {
         public:
            using save_interface_t = load_order_interfaces::form_save;
         public:
            const fragment_type type;
            //
            basic_fragment_data(fragment_type t) : type(t) {}
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) = 0;
            virtual void save(script_data& owner, tes_subrecord_writer&, const script_data_save_parameters&) = 0;
            virtual basic_fragment_data* clone(loaded_forms::Form& owner_of_clone) const noexcept = 0;
            virtual void clear(loaded_forms::Form& owner) {}
            virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {}
      };

      class script_data {
         public:
            class script;
            class property;
            using load_interface_t = load_order_interfaces::form_load;
            using save_interface_t = load_order_interfaces::form_save;
         public:
            int16_t version       = 5;
            int16_t object_format = 2; // format of "object" property values
            std::vector<script>  scripts;
            basic_fragment_data* fragment_data = nullptr;
            //
            bool load(tes_subrecord_reader&, load_interface_t&); // assumes we're at a VMAD subrecord
            bool save(tes_subrecord_writer&, save_interface_t&, const script_data_save_parameters& = script_data_save_parameters::default);
            static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
            //
            bool save(tes_record_writer&, save_interface_t&, const script_data_save_parameters& = script_data_save_parameters::default); // opens VMAD, writes, closes; doesn't write a subrecord if there are no scripts attached
            void clone_from(const script_data& source, loaded_forms::Form& owner_of_clone) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            void clear(loaded_forms::Form& my_owner) noexcept;
            //
            void for_each_script(std::function<bool(script*)>); // return true to stop iterating early
            //
         public:
            enum class script_status : uint8_t {
               local   = 0,
               altered = 1,
               removed = 3,
            };
            enum class property_status : uint8_t {
               altered = 1,
               removed = 3,
            };
            struct property_object_value { // if object_format == 2, then the order of fields is reversed in the file
               form_reference_t form;
               uint16_t aliasID;
               uint16_t always_zero = 0;
               //
               bool load(script_data& owner, tes_subrecord_reader&);
               bool save(script_data& owner, tes_subrecord_writer&) const noexcept;
               void clone_from(const property_object_value& source, loaded_forms::Form& owner_of_clone) noexcept;
               void clear(loaded_forms::Form& owner);
               void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            };
            class property {
               friend script;
               public:
                  struct value_t {
                     union {
                        bool    boolean;
                        float   float32;
                        int32_t integer = 0;
                     };
                     std::string string;
                     property_object_value object;
                     //
                     bool load(property_type, script_data& owner, tes_subrecord_reader&);
                     bool save(property_type, script_data& owner, tes_subrecord_writer&) const noexcept;
                     void clone_from(property_type, const value_t& source, loaded_forms::Form& owner_of_clone) noexcept;
                  };
                  //
               public:
                  std::string     name;
                  property_type   type = property_type::integer;
                  property_status status;
                  std::vector<value_t> values;
                  //
                  bool load(script_data& owner, tes_subrecord_reader&);
                  bool save(script_data& owner, tes_subrecord_writer&) const noexcept;
                  void clone_from(const property& source, loaded_forms::Form& owner_of_clone) noexcept;
                  void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
                  void clear(loaded_forms::Form& my_owner) noexcept;
            };
            class script {
               friend script_data;
               public:
                  std::string   name;
                  script_status status;
                  std::vector<property> properties;
                  //
                  bool load(script_data& owner, tes_subrecord_reader&);
                  bool save(script_data& owner, tes_subrecord_writer&) const noexcept;
                  void clear_properties(loaded_forms::Form& owner);
                  void clone_from(const script& source, loaded_forms::Form& owner_of_clone) noexcept;
                  void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
                  void clear(loaded_forms::Form& my_owner) noexcept;
            };
      };

      #pragma region Quest data
      class quest_log_entry_fragment {
         public:
            bool defined = false;
            std::string filename;
            std::string function;
            //
            struct {
               uint16_t stage_id    = 0;
               uint32_t entry_index = 0;
            } ownership; // must be manually updated before save
            //
            bool load(tes_subrecord_reader&) noexcept;
            void save(tes_subrecord_writer&) const noexcept;
            void clone_from(const quest_log_entry_fragment&) noexcept;
            void clear() noexcept;
      };
      class quest_alias_script_data {
         public:
            script_data::property_object_value alias; // quest form ID and alias index
            int16_t version   = 5;
            int16_t objFormat = 2;
            std::vector<script_data::script> scripts;
            //
            void load(script_data& owner, tes_subrecord_reader&);
            void save(script_data& owner, tes_subrecord_writer&) const;
            quest_alias_script_data* clone(loaded_forms::Form& owner_of_clone) const noexcept;
            void clear(loaded_forms::Form& owner);
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
      };
      #pragma endregion
      
      #pragma region Script fragment definitions
      struct basic_fragment_entry {
         uint8_t     unknown;
         std::string script;
         std::string function;
      };
      class topic_info_fragment_data : basic_fragment_data {
         public:
            struct fragment_flag {
               fragment_flag() = delete;
               enum type : uint8_t {
                  has_begin_fragment = 0x01,
                  has_end_fragment   = 0x02,
               };
            };
            using fragment_flag_t = std::underlying_type_t<fragment_flag::type>;
            //
            topic_info_fragment_data() : basic_fragment_data(fragment_type::info) {};
            using fragment_t = basic_fragment_entry;
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) override;
            virtual void save(script_data& owner, tes_subrecord_writer&, const script_data_save_parameters&) override;
            virtual basic_fragment_data* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
            //
            uint8_t         unknown = 2;
            fragment_flag_t flags   = 0;
            std::string     filename;
            fragment_t      onBeginFragment;
            fragment_t      onEndFragment;
      };
      class package_fragment_data : basic_fragment_data {
         public:
            struct fragment_flag {
               fragment_flag() = delete;
               enum type : uint8_t {
                  has_begin_fragment  = 0x01,
                  has_end_fragment    = 0x02,
                  has_change_fragment = 0x04,
               };
            };
            using fragment_flag_t = std::underlying_type_t<fragment_flag::type>;
            //
            package_fragment_data() : basic_fragment_data(fragment_type::package) {};
            using fragment_t = basic_fragment_entry;
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) override;
            virtual void save(script_data& owner, tes_subrecord_writer&, const script_data_save_parameters&) override;
            virtual basic_fragment_data* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
            //
            uint8_t         unknown = 2;
            fragment_flag_t flags = 0;
            std::string     filename;
            fragment_t      onBeginFragment;
            fragment_t      onEndFragment;
            fragment_t      onChangeFragment;
      };
      class perk_fragment_data : basic_fragment_data {
         public:
            perk_fragment_data() : basic_fragment_data(fragment_type::perk) {};
            struct fragment_t {
               uint16_t index;
               uint16_t unknown02;
               uint8_t  unknown04;
               std::string filename;
               std::string function;
            };
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) override;
            virtual void save(script_data& owner, tes_subrecord_writer&, const script_data_save_parameters&) override;
            virtual basic_fragment_data* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
            //
            uint8_t     unknown = 2;
            std::string filename;
            std::vector<fragment_t> fragments;
      };
      class quest_fragment_data : basic_fragment_data {
         public:
            quest_fragment_data() : basic_fragment_data(fragment_type::quest) {};
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) override;
            virtual void save(script_data& owner, tes_subrecord_writer&, const script_data_save_parameters&) override;
            virtual basic_fragment_data* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
            virtual void clear(loaded_forms::Form& owner) override;
            virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept override;
            //
            uint8_t     unknown = 2;
            std::string filename;
            struct {
               //
               // These data structures are "unowned" when VMAD is first loaded, and can 
               // then be "claimed" by the containing form. Structures that remain unclaimed 
               // should not be discarded, as we can't anticipate that when building use info, 
               // and discarding them after use info was built for their contents will cause 
               // use info to desynch with the loaded form data.
               //
               std::vector<quest_log_entry_fragment> fragments;
               std::vector<quest_alias_script_data*> aliases;
            } unowned_data;
      };
      class scene_fragment_data : basic_fragment_data {
         public:
            struct fragment_flag {
               fragment_flag() = delete;
               enum type : uint8_t {
                  has_begin_fragment  = 0x01,
                  has_end_fragment    = 0x02,
               };
            };
            using fragment_flag_t = std::underlying_type_t<fragment_flag::type>;
            //
            scene_fragment_data() : basic_fragment_data(fragment_type::scene) {};
            using fragment_t = basic_fragment_entry;
            struct phase_fragment_type {
               uint8_t  unknown00;
               uint32_t phase; // zero-indexed internally; one-indexed in UI
               uint8_t  unknown05;
               std::string filename;
               std::string function;
            };
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) override;
            virtual void save(script_data& owner, tes_subrecord_writer&, const script_data_save_parameters&) override;
            virtual basic_fragment_data* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
            //
            uint8_t     unknown = 2;
            uint8_t     flags   = 0;
            std::string filename;
            fragment_t  onBeginFragment;
            fragment_t  onEndFragment;
            std::vector<phase_fragment_type> phaseFragments;
      };
      #pragma endregion
   }
   using papyrus_attachment_data = papyrus::script_data;
}