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
      class basic_fragment_data {
         public:
            fragment_type type;
            //
            basic_fragment_data(fragment_type t) : type(t) {}
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) = 0;
            virtual void save(script_data& owner, tes_subrecord_writer&) = 0;
            virtual basic_fragment_data* clone(form_stub& owner_of_clone) const noexcept = 0;
            virtual void clear(form_stub& owner) {}
            virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {}
      };

      class script_data {
         public:
            class script;
            class property;
         public:
            int16_t version;
            int16_t object_format; // format of "object" property values
            std::vector<script>  scripts;
            basic_fragment_data* fragment_data = nullptr;
            //
            bool load(tes_subrecord_reader&); // assumes we're at a VMAD subrecord
            bool save(tes_subrecord_writer&);
            static void generateUseInfo(tes_subrecord_reader&, form_stub*);
            //
            bool save(tes_record_writer&); // opens VMAD, writes, closes
            void clone_from(const script_data& source, form_stub& owner_of_clone) noexcept;
            void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
            //
            void for_each_script(std::function<bool(script*)>);
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
               form_id_t formID;
               uint16_t  aliasID;
               uint16_t  always_zero = 0;
               //
               bool load(script_data& owner, tes_subrecord_reader&);
               bool save(script_data& owner, tes_subrecord_writer&);
               void clone_from(const property_object_value& source, form_stub& owner_of_clone) noexcept;
               void clear(form_stub& owner);
               void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
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
                     bool save(property_type, script_data& owner, tes_subrecord_writer&);
                     void clone_from(property_type, const value_t& source, form_stub& owner_of_clone) noexcept;
                  };
                  //
               public:
                  std::string     name;
                  property_type   type = property_type::integer;
                  property_status status;
                  std::vector<value_t> values;
                  //
                  bool load(script_data& owner, tes_subrecord_reader&);
                  bool save(script_data& owner, tes_subrecord_writer&);
                  void clone_from(const property& source, form_stub& owner_of_clone) noexcept;
                  void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
            };
            class script {
               friend script_data;
               public:
                  std::string   name;
                  script_status status;
                  std::vector<property> properties;
                  //
                  bool load(script_data& owner, tes_subrecord_reader&);
                  bool save(script_data& owner, tes_subrecord_writer&);
                  void clear_properties(form_stub& owner);
                  void clone_from(const script& source, form_stub& owner_of_clone) noexcept;
                  void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
            };
      };
      
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
            virtual void save(script_data& owner, tes_subrecord_writer&) override;
            virtual basic_fragment_data* clone(form_stub& owner_of_clone) const noexcept override;
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
            virtual void save(script_data& owner, tes_subrecord_writer&) override;
            virtual basic_fragment_data* clone(form_stub& owner_of_clone) const noexcept override;
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
            virtual void save(script_data& owner, tes_subrecord_writer&) override;
            virtual basic_fragment_data* clone(form_stub& owner_of_clone) const noexcept override;
            //
            uint8_t     unknown = 2;
            std::string filename;
            std::vector<fragment_t> fragments;
      };
      class quest_fragment_data : basic_fragment_data {
         public:
            quest_fragment_data() : basic_fragment_data(fragment_type::quest) {};
            struct fragment_t {
               uint16_t    index;
               uint16_t    unknown02;
               uint32_t    logEntry;
               uint8_t     unknown08;
               std::string filename;
               std::string function;
            };
            struct alias_type {
               script_data::property_object_value alias;
               int16_t version;
               int16_t objFormat;
               std::vector<script_data::script> scripts;
            };
            //
            virtual void load(script_data& owner, tes_subrecord_reader&) override;
            virtual void save(script_data& owner, tes_subrecord_writer&) override;
            virtual basic_fragment_data* clone(form_stub& owner_of_clone) const noexcept override;
            virtual void clear(form_stub& owner) override;
            virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept override;
            //
            uint8_t     unknown = 2;
            std::string filename;
            std::vector<fragment_t> fragments;
            std::vector<alias_type> aliasScriptData;
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
            virtual void save(script_data& owner, tes_subrecord_writer&) override;
            virtual basic_fragment_data* clone(form_stub& owner_of_clone) const noexcept override;
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