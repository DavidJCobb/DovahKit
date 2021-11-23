#pragma once
#include <QObject>
#include "../helpers/qt/keycodes.h"
#include "../dk3d/BoundInput.h"

class DovahKit3DControls : public QObject {
   Q_OBJECT;
   protected:
      DovahKit3DControls();
   public:
      static DovahKit3DControls& get() {
         static DovahKit3DControls instance;
         return instance;
      }

   signals:
      void bindingsChanged();

   public:
      struct {
         struct {
            struct {
               cobb::qt::key forward;
               cobb::qt::key back;
               cobb::qt::key left;
               cobb::qt::key right;
               cobb::qt::key up;
               cobb::qt::key down;
            } move;
            struct {
               cobb::qt::key left;
               cobb::qt::key right;
               cobb::qt::key up;
               cobb::qt::key down;
            } turn;
         } camera;
      } keyboard;
      struct {
         struct {
            struct {
               DK3D::BoundInput lateral;
               DK3D::BoundInput up;
               DK3D::BoundInput down;
            } move;
            DK3D::BoundInput turn;
         } camera;
      } gamepad;
};