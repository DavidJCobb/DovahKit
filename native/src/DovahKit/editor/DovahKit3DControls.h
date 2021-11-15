#pragma once
#include <QObject>
#include "../helpers/qt/keycodes.h"

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
};