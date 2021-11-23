#pragma once
#include <array>
#include <bitset>
#include <QObject>
#include "../helpers/bitfield_array.h"
#include "chrono.h"
#include "BoundInput.h"
#include "BoundInputState.h"
#include "widgets/DKVulkanView.h"

class DK3DInputHandler : public QObject {
   Q_OBJECT;
   public:
      using Axis2D          = DK3D::Axis2D;
      using BoundInputState = DK3D::BoundInputState;
      using ScalarControl   = DK3D::ScalarControl;
      using VectorControl   = DK3D::VectorControl;
      using timestamp_t     = DK3D::timestamp_t;
   protected:
      DK3DInputHandler();
      ~DK3DInputHandler();
   public:
      static DK3DInputHandler& get() {
         static DK3DInputHandler instance;
         return instance;
      }

      float   scalarControlValue(ScalarControl, Axis2D axis = Axis2D::X) const;
      QPointF vectorControlValue(VectorControl) const;

   public slots:
      void setTargetView(DKVulkanView* target);
      DKVulkanCameraUpdate update(DKVulkanView* subject); // TODO: should return something else -- a more complete command list -- in the future

   protected slots:
      void ignoreAllHeldKeys(timestamp_t now = DK3D::zero_timestamp);
      void updateAllKeys(timestamp_t now);

   protected:
      enum class key_release_type {
         none,
         tap,
         hold,
      };
      static constexpr size_t vk_code_count = 256;

      struct {
         //
         // This is OS-specific, since Qt has no APIs for polling for input.
         //
         std::array<timestamp_t, vk_code_count> start;
         std::bitset<vk_code_count> ignore;
         cobb::bitfield_array<key_release_type, vk_code_count, 2> releases;
      } keyboard_state;

      struct {
         QPointer<DKVulkanView> target_view;
         timestamp_t last_update = DK3D::zero_timestamp;
         struct {
            int32_t x     = 0;
            int32_t y     = 0;
            int32_t wheel = 0;
         } mousemove;
      } state;
      struct {
         //
         // Bindings are still hardcoded for now.
         //
         struct {
            struct {
               struct {
                  BoundInputState forward;
                  BoundInputState back;
                  BoundInputState left;
                  BoundInputState right;
                  BoundInputState up;
                  BoundInputState down;
               } move;
               struct {
                  BoundInputState left;
                  BoundInputState right;
                  BoundInputState up;
                  BoundInputState down;
               } turn;
            } camera;
         } keyboard;
         struct {
            struct {
               struct {
                  BoundInputState lateral;
                  BoundInputState down;
                  BoundInputState up;
               } move;
               struct {
                  BoundInputState yaw;
                  BoundInputState pitch;
               } turn;
            } camera;
            BoundInputState test_tap;
            BoundInputState test_hold;
            BoundInputState test_while;
         } gamepad;
      } binds;
};