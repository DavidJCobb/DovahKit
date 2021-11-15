#include "DovahKit3DControls.h"

DovahKit3DControls::DovahKit3DControls() {
   {
      auto& cm = this->keyboard.camera.move;
      cm.forward = cobb::qt::key('W');
      cm.back    = cobb::qt::key('S');
      cm.left    = cobb::qt::key('A');
      cm.right   = cobb::qt::key('D');
      cm.up      = cobb::qt::key('Q');
      cm.down    = cobb::qt::key('Z');
   }
   {
      auto& ct = this->keyboard.camera.turn;
      ct.left  = cobb::qt::key('G');
      ct.right = cobb::qt::key('H');
      ct.up    = cobb::qt::key('R');
      ct.down  = cobb::qt::key('V');
   }
}