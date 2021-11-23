#pragma once
#include <cstdint>

namespace DK3D {
   enum class InputType {
      Boolean, // button
      Scalar,  // 1D axis (e.g. gamepad trigger, joystick up/down, joystick left/right, mousemove up/down, mousemove left/right)
      Vector,  // 2D axes (e.g. joystick, mousemove)
   };

   enum class BooleanInput {
      Tap,
      Release,
      While,
      DoubleClick,
   };

   enum class VectorInput {
      X,
      Y,
   };

   // ---

   enum class EditorMode {
      Object,
      Landscape,
      Navmesh,
   };

   enum class CameraState {
      Normal,
      Precision,
      Boost,
   };

   enum class Axis3D {
      X,
      Y,
      Z,
   };

   enum class Pivot {
      Origin,
      CenterOfMass,
   };

   enum class ReferenceFrame {
      Camera,
      Selection,
      World,
      Current,
   };

   // ---

   namespace FunctionOptionBase {
      struct EditScalarValue {
         //
         // Suitable for:
         //  - Object / Adjust Light Property
         //  - Object / Scale Entities
         //
         bool increase : 1;
         struct {
            bool use_axis_x : 1;
            bool use_axis_y : 1;
            bool invert_x   : 1; // if false, X-right = increase
            bool invert_y   : 1; // if false, X-down  = increase
         } vector_input;
      };
   }
   namespace FunctionOptions {
      struct MoveCamera {
         struct {
            bool differs_if_selection : 1;
            ReferenceFrame with_selection : 2; // "Current" is the "translate" frame
            ReferenceFrame sans_selection : 2;
         } reference_frame;
         bool also_translate_selection : 1;
         struct { // boolean and scalar inputs only; multiplied by movement speed
            float x;
            float y;
            float z;
         } axes;
      };
      struct TurnCamera {
         struct { // boolean and scalar inputs only; multiplied by look sensitivity
            float x;
            float y;
            float z;
         } axes;
         bool use_selection_axes : 1;
      };
      struct SetCameraSpeed {
         bool normal    : 1;
         bool precision : 1;
         bool boost     : 1;
      };

      struct Paste {
         bool at_aim_position : 1;
      };

      struct DropEntities {
         bool individually : 1;
      };

      // Object:

      struct AdjustLightProperty : public FunctionOptionBase::EditScalarValue {
         enum class Property {
            Fade,
            Radius,
            DepthBias,
         };
         //
         Property property : 2;
      };
      struct ModifyPositionSnap : public FunctionOptionBase::EditScalarValue {};
      struct ModifyRotationSnap : public FunctionOptionBase::EditScalarValue {};
      struct ModifyScaleSnap : public FunctionOptionBase::EditScalarValue {};
      struct RotateEntities {
         struct {
            //
            // Boolean input only.
            //
            float red  = 0;
            float blue = 0;
         } magnitude;
         ReferenceFrame reference_frame : 2;
         struct {
            bool use_red_axis : 1;
         } scalar_input;
         struct {
            bool use_axis_x    : 1;
            bool use_axis_y    : 1;
            bool invert_x      : 1; // if false, X-right = increase
            bool invert_y      : 1; // if false, X-down  = increase
            bool axis_x_is_red : 1;
         } vector_input;
      };

      // Landscape:

      struct ModifyBrushSize {
         bool modify : 1;
         int  value  : 5; // -16 - 15
      };

      // Navmesh:

      struct SetNonNavmeshVisibility {
         bool visible     : 1;
         bool translucent : 1;
         bool hidden      : 1;
      };
      struct ToggleSelectionEnabled {
         bool triangles : 1;
         bool edges     : 1;
         bool vertices  : 1;
      };
   }
}