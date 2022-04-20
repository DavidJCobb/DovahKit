#pragma once
#include <QObject>
#include "helpers/enum_flags.h"
#include "helpers/singleton_ex.h"
#include "helpers/vector3.h"
#include "dovah/form_stub.h"
#include "dovah/forms/ObjectReference.h"
#include "nif/file.h"
#include "vulkan/scene_item_handle.h"

class DKVulkanView;

namespace dovahkit::subsystems {
   class worldedit;

   //
   // Subsystem for accessing game assets.
   //
   class worldedit : public QObject, public cobb::singleton_ex<worldedit> {
      Q_OBJECT;
      protected:
         worldedit();
      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         enum class camera_speed_flags {
            boost,
            precision,
         };

      protected:
         using form_stub       = dovah::form_stub;
         using loaded_refr_ptr = dovah::loaded_form_ptr<dovah::loaded_forms::ObjectReference>;

         struct refr {
            form_stub*      stub = nullptr;
            loaded_refr_ptr form;
            std::unique_ptr<nifDK::file> nif;
            struct {
               vulkanDK::rendered_light_handle light;
            } vulkan_handles;
         };

         struct cell {
            dovah::form_stub* stub = nullptr;
         };

         DKVulkanView* target_view = nullptr;
         cell loaded_cell;
         std::vector<refr> loaded_refs;
         //
         struct {
            cobb::enum_flags<camera_speed_flags, 2> camera_speed;
            struct {
               std::vector<dovah::form_stub*> refs;
            } selection;
         } state;

         void _unload_refr(dovah::form_stub&);
         void _unload_cell(dovah::form_stub*);
         bool _load_refr(dovah::form_stub&, cobb::vector3<float>& out_pos, cobb::vector3<float>& out_rot, bool& out_is_coc);
         void _load_cell(dovah::form_stub*, bool move_camera_to);
         
      public:
         void set_current_cell(dovah::form_stub*);
         void set_target_view(DKVulkanView&);

         void view_input_poll_handler(DKVulkanView&);

         bool is_ref_loaded(const dovah::form_stub*) const;

      signals:
         void refSelected(dovah::form_stub&);
         void refDeselected(dovah::form_stub&);
         void statusBarMessage(const QString& message, int display_time = 0);

      public slots:
         void setRefSelectionState(dovah::form_stub&, bool state);
         void toggleRefSelectionState(dovah::form_stub&);
   };
}