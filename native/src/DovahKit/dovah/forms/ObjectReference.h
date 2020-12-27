#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/extra_data.h"
#include "components/papyrus.h"
#include "../../helpers/vector3.h"

namespace dovah::loaded_forms {
   class ObjectReference : public Form {
      public:
         static constexpr form_type_t form_type = form_type::reference;
         ObjectReference() : Form(form_type) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               //
               // Some flags' meanings differ depending on the base form's type. If a line comment after a 
               // value consists of a list of signatures, then it indicates the form types for which that 
               // value is relevant. If the same value is given two different names and only one has a list 
               // of signatures, then the other value applies to all form types not covered by the first.
               //
               hide_from_local_map_a   = 0x00000040, // DOOR
               doesnt_light_water      = 0x00000100, // LIGH
               inaccessible            = 0x00000100, // DOOR
               hide_from_local_map_b   = 0x00000200, // ACTI, STAT, TREE
               motion_blur             = 0x00000200, // MSTT
               starts_dead             = 0x00000200, // ACHR
               persistent              = 0x00000400,
               disabled                = 0x00000800,
               visible_when_distant    = 0x00008000, // ACTI, STAT, TREE
               is_full_lod             = 0x00010000,
               never_fades             = 0x00010000, // LIGH
               doesnt_light_landscape  = 0x00020000, // LIGH
               no_ai_acquire           = 0x02000000, // ACHR, CONT, LIGH, all items
               filter                  = 0x04000000, // related to collision geometry? navmeshing?
               bounding_box            = 0x08000000, // related to collision geometry? navmeshing?
               reflected_by_auto_water = 0x10000000,
               dont_havok_settle       = 0x20000000, // ACHR, ACTI, ADON, DOOR, LIGH, MSTT, STAT, TREE, all items
               ground                  = 0x40000000, // CONT, general
               no_respawn              = 0x40000000, // ACTI, ADON, DOOR, LIGH, MSTT, STAT, TREE, all items
               multibound              = 0x80000000,
            };
         };

         components::extra_data_list         extra_data;
         components::papyrus_attachment_data script_data; // VMAD
         base_form_reference_t base_form; // NAME
         bool is_open = false; // ONAM (empty record; acts as sentinel)
         cobb::vector3<float> position; // DATA
         cobb::vector3<float> rotation; // DATA // radians

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual components::papyrus_attachment_data* get_papyrus_data() noexcept override { return &this->script_data; }
         //
      protected:
         ObjectReference(form_type_t ft) : Form(ft) {}; // for subclasses
         //
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual bool _friendly_delete_impl(const file_load_order&) noexcept override;
   };
}