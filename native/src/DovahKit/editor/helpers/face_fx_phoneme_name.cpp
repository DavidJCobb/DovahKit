#include "./face_fx_phoneme_name.h"
#include <QCoreApplication>

namespace editor_helpers {
   extern QString face_fx_phoneme_name(dovah::face_fx::phoneme phoneme) {
      switch (phoneme) {
         case dovah::face_fx::phoneme::IY:
            return QCoreApplication::translate("all phoneme names", "IY");
         case dovah::face_fx::phoneme::IH:
            return QCoreApplication::translate("all phoneme names", "IH");
         case dovah::face_fx::phoneme::EH:
            return QCoreApplication::translate("all phoneme names", "EH");
         case dovah::face_fx::phoneme::EY:
            return QCoreApplication::translate("all phoneme names", "EY");
         case dovah::face_fx::phoneme::AE:
            return QCoreApplication::translate("all phoneme names", "AE");
         case dovah::face_fx::phoneme::AA:
            return QCoreApplication::translate("all phoneme names", "AA");
         case dovah::face_fx::phoneme::AW:
            return QCoreApplication::translate("all phoneme names", "AW");
         case dovah::face_fx::phoneme::AY:
            return QCoreApplication::translate("all phoneme names", "AY");
         case dovah::face_fx::phoneme::AH:
            return QCoreApplication::translate("all phoneme names", "AH");
         case dovah::face_fx::phoneme::AO:
            return QCoreApplication::translate("all phoneme names", "AO");
         case dovah::face_fx::phoneme::OY:
            return QCoreApplication::translate("all phoneme names", "OY");
         case dovah::face_fx::phoneme::OW:
            return QCoreApplication::translate("all phoneme names", "OW");
         case dovah::face_fx::phoneme::UH:
            return QCoreApplication::translate("all phoneme names", "UH");
         case dovah::face_fx::phoneme::UW:
            return QCoreApplication::translate("all phoneme names", "UW");
         case dovah::face_fx::phoneme::ER:
            return QCoreApplication::translate("all phoneme names", "ER");
         case dovah::face_fx::phoneme::AX:
            return QCoreApplication::translate("all phoneme names", "AX");
         case dovah::face_fx::phoneme::S:
            return QCoreApplication::translate("all phoneme names", "S");
         case dovah::face_fx::phoneme::SH:
            return QCoreApplication::translate("all phoneme names", "SH");
         case dovah::face_fx::phoneme::Z:
            return QCoreApplication::translate("all phoneme names", "Z");
         case dovah::face_fx::phoneme::ZH:
            return QCoreApplication::translate("all phoneme names", "ZH");
         case dovah::face_fx::phoneme::F:
            return QCoreApplication::translate("all phoneme names", "F");
         case dovah::face_fx::phoneme::TH:
            return QCoreApplication::translate("all phoneme names", "TH");
         case dovah::face_fx::phoneme::V:
            return QCoreApplication::translate("all phoneme names", "V");
         case dovah::face_fx::phoneme::DH:
            return QCoreApplication::translate("all phoneme names", "DH");
         case dovah::face_fx::phoneme::M:
            return QCoreApplication::translate("all phoneme names", "M");
         case dovah::face_fx::phoneme::N:
            return QCoreApplication::translate("all phoneme names", "N");
         case dovah::face_fx::phoneme::NG:
            return QCoreApplication::translate("all phoneme names", "NG");
         case dovah::face_fx::phoneme::L:
            return QCoreApplication::translate("all phoneme names", "L");
         case dovah::face_fx::phoneme::R:
            return QCoreApplication::translate("all phoneme names", "R");
         case dovah::face_fx::phoneme::W:
            return QCoreApplication::translate("all phoneme names", "W");
         case dovah::face_fx::phoneme::Y:
            return QCoreApplication::translate("all phoneme names", "Y");
         case dovah::face_fx::phoneme::HH:
            return QCoreApplication::translate("all phoneme names", "HH");
         case dovah::face_fx::phoneme::B:
            return QCoreApplication::translate("all phoneme names", "B");
         case dovah::face_fx::phoneme::D:
            return QCoreApplication::translate("all phoneme names", "D");
         case dovah::face_fx::phoneme::JH:
            return QCoreApplication::translate("all phoneme names", "JH");
         case dovah::face_fx::phoneme::G:
            return QCoreApplication::translate("all phoneme names", "G");
         case dovah::face_fx::phoneme::P:
            return QCoreApplication::translate("all phoneme names", "P");
         case dovah::face_fx::phoneme::T:
            return QCoreApplication::translate("all phoneme names", "T");
         case dovah::face_fx::phoneme::K:
            return QCoreApplication::translate("all phoneme names", "K");
         case dovah::face_fx::phoneme::CH:
            return QCoreApplication::translate("all phoneme names", "CH");
         case dovah::face_fx::phoneme::SIL:
            return QCoreApplication::translate("all phoneme names", "SIL");
         case dovah::face_fx::phoneme::SHORTSIL:
            return QCoreApplication::translate("all phoneme names", "SHORTSIL");
         case dovah::face_fx::phoneme::FLAP:
            return QCoreApplication::translate("all phoneme names", "FLAP");
      }
      return "";
   }
}