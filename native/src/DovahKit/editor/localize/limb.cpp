#include "./limb.h"
#include <QCoreApplication>

#define ENUMERATION_TYPE dovah::limb
#include "./_macros.h"

namespace editor::localize {
   extern QString limb(ENUMERATION_TYPE v) {
      using enum ENUMERATION_TYPE;
      switch (v) {
         case eye:
            return STRING("Eye");
         case fly_grab:
            return STRING("Fly Grab");
         case head:
            return STRING("Head");
         case look_at:
            return STRING("Look At");
         case saddle:
            return STRING("Saddle");
         case torso:
            return STRING("Torso");
      }
      return "";
   }
}