#include "./collision_layer.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString collision_layer(dovah::collision_layer v) {
      switch (v) {
         case dovah::collision_layer::acoustic_space:
            return QCoreApplication::translate("dovah::collision_layer", "L_ACOUSTIC_SPACE");
         case dovah::collision_layer::actor_zone:
            return QCoreApplication::translate("dovah::collision_layer", "L_ACTORZONE");
         case dovah::collision_layer::anim_static:
            return QCoreApplication::translate("dovah::collision_layer", "L_ANIMSTATIC");
         case dovah::collision_layer::avoid_box:
            return QCoreApplication::translate("dovah::collision_layer", "L_AVOIDBOX");
         case dovah::collision_layer::biped:
            return QCoreApplication::translate("dovah::collision_layer", "L_BIPED");
         case dovah::collision_layer::biped_sans_char_controller:
            return QCoreApplication::translate("dovah::collision_layer", "L_BIPED_NO_CC");
         case dovah::collision_layer::camera_pick:
            return QCoreApplication::translate("dovah::collision_layer", "L_CAMERA");
         case dovah::collision_layer::camera_sphere:
            return QCoreApplication::translate("dovah::collision_layer", "L_CAMERASPHERE");
         case dovah::collision_layer::character_controller:
            return QCoreApplication::translate("dovah::collision_layer", "L_CHARCONTROLLER");
         case dovah::collision_layer::cloud_trap:
            return QCoreApplication::translate("dovah::collision_layer", "L_CLOUDTRAP");
         case dovah::collision_layer::clutter:
            return QCoreApplication::translate("dovah::collision_layer", "L_CLUTTER");
         case dovah::collision_layer::collision_box:
            return QCoreApplication::translate("dovah::collision_layer", "L_COLLISIONBOX");
         case dovah::collision_layer::cone_projectile:
            return QCoreApplication::translate("dovah::collision_layer", "L_CONEPROJECTILE");
         case dovah::collision_layer::critter:
            return QCoreApplication::translate("dovah::collision_layer", "L_CRITTER");
         case dovah::collision_layer::custom_pick_1:
            return QCoreApplication::translate("dovah::collision_layer", "L_CUSTOMPICK1");
         case dovah::collision_layer::custom_pick_2:
            return QCoreApplication::translate("dovah::collision_layer", "L_CUSTOMPICK2");
         case dovah::collision_layer::dead_biped:
            return QCoreApplication::translate("dovah::collision_layer", "L_DEADBIP");
         case dovah::collision_layer::debris_large:
            return QCoreApplication::translate("dovah::collision_layer", "L_DEBRIS_LARGE");
         case dovah::collision_layer::debris_small:
            return QCoreApplication::translate("dovah::collision_layer", "L_DEBRIS_SMALL");
         case dovah::collision_layer::detection:
            return QCoreApplication::translate("dovah::collision_layer", "L_DETECTION");
         case dovah::collision_layer::door_detection:
            return QCoreApplication::translate("dovah::collision_layer", "L_DOORDETECTION");
         case dovah::collision_layer::dropping_pick:
            return QCoreApplication::translate("dovah::collision_layer", "L_DROPPINGPICK");
         case dovah::collision_layer::gas_trap:
            return QCoreApplication::translate("dovah::collision_layer", "L_GASTRAP");
         case dovah::collision_layer::ground:
            return QCoreApplication::translate("dovah::collision_layer", "L_GROUND");
         case dovah::collision_layer::invisible_wall:
            return QCoreApplication::translate("dovah::collision_layer", "L_INVISIBLE_WALL");
         case dovah::collision_layer::item_pick:
            return QCoreApplication::translate("dovah::collision_layer", "L_ITEMPICKER");
         case dovah::collision_layer::line_of_sight:
            return QCoreApplication::translate("dovah::collision_layer", "L_LOS");
         case dovah::collision_layer::living_and_dead_actors:
            return QCoreApplication::translate("dovah::collision_layer", "L_LIVING_AND_DEAD_ACTORS");
         case dovah::collision_layer::navcut:
            return QCoreApplication::translate("dovah::collision_layer", "L_NAVCUT");
         case dovah::collision_layer::non_collidable:
            return QCoreApplication::translate("dovah::collision_layer", "L_NONCOLLIDABLE");
         case dovah::collision_layer::path_pick:
            return QCoreApplication::translate("dovah::collision_layer", "L_PATHINGPICK");
         case dovah::collision_layer::portal:
            return QCoreApplication::translate("dovah::collision_layer", "L_PORTAL");
         case dovah::collision_layer::projectile:
            return QCoreApplication::translate("dovah::collision_layer", "L_PROJECTILE");
         case dovah::collision_layer::projectile_zone:
            return QCoreApplication::translate("dovah::collision_layer", "L_PROJECTILEZONE");
         case dovah::collision_layer::props:
            return QCoreApplication::translate("dovah::collision_layer", "L_PROPS");
         case dovah::collision_layer::shell_casing:
            return QCoreApplication::translate("dovah::collision_layer", "L_SHELLCASING");
         case dovah::collision_layer::spell:
            return QCoreApplication::translate("dovah::collision_layer", "L_SPELL");
         case dovah::collision_layer::spell_explosion:
            return QCoreApplication::translate("dovah::collision_layer", "L_SPELLEXPLOSION");
         case dovah::collision_layer::spell_trigger:
            return QCoreApplication::translate("dovah::collision_layer", "L_SPELLTRIGGER");
         case dovah::collision_layer::stair_helper:
            return QCoreApplication::translate("dovah::collision_layer", "L_STAIRHELPER");
         case dovah::collision_layer::statik:
            return QCoreApplication::translate("dovah::collision_layer", "L_STATIC");
         case dovah::collision_layer::terrain:
            return QCoreApplication::translate("dovah::collision_layer", "L_TERRAIN");
         case dovah::collision_layer::transparent:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRANSPARENT");
         case dovah::collision_layer::transparent_small:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRANSPARENT_SMALL");
         case dovah::collision_layer::transparent_small_anim:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRANSPARENT_SMALL_ANIM");
         case dovah::collision_layer::trap:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRAP");
         case dovah::collision_layer::trap_trigger:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRAP_TRIGGER");
         case dovah::collision_layer::trees:
            return QCoreApplication::translate("dovah::collision_layer", "L_TREES");
         case dovah::collision_layer::trigger:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRIGGER");
         case dovah::collision_layer::trigger_falling_trap:
            return QCoreApplication::translate("dovah::collision_layer", "L_TRIGGER_FALLING_TRAP");
         case dovah::collision_layer::unidentified:
            return QCoreApplication::translate("dovah::collision_layer", "L_UNIDENTIFIED");
         case dovah::collision_layer::ward:
            return QCoreApplication::translate("dovah::collision_layer", "L_WARD");
         case dovah::collision_layer::water:
            return QCoreApplication::translate("dovah::collision_layer", "L_WATER");
         case dovah::collision_layer::weapon:
            return QCoreApplication::translate("dovah::collision_layer", "L_WEAPON");
      }
      return "";
   }
}