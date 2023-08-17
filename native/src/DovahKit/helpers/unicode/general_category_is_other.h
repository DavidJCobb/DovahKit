#pragma once
#include <cstdint>

namespace cobb::unicode {
   //
   // This function returns true if the given code point is in the Cc, Cf, Cs, Co, or Cn 
   // general categories. In other words, it returns `!QChar(c).isPrint()`, if you're 
   // familiar with Qt.
   // 
   // This function is generated from `_generated/unicode/general_category_is_other.js`, 
   // as storing the entire list of Unicode glyphs and their metadata in C++ as constexpr 
   // data and generating things *there* is... not viable... given the memory limitations 
   // involved.
   //
   constexpr bool general_category_is_other(std::uint32_t c) {
      if (c < 0x1000) {
         if (c <= 0x1F)
            return true;
         if (c >= 0x7F && c <= 0x9F)
            return true;
         if (c >= 0x380 && c <= 0x383)
            return true;
         if (c >= 0x5C8 && c <= 0x5CF)
            return true;
         if (c >= 0x5EB && c <= 0x5EE)
            return true;
         if (c >= 0x5F5 && c <= 0x605)
            return true;
         if (c >= 0x7B2 && c <= 0x7BF)
            return true;
         if (c >= 0x86B && c <= 0x86F)
            return true;
         if (c >= 0x88F && c <= 0x897)
            return true;
         if (c >= 0x9B3 && c <= 0x9B5)
            return true;
         if (c >= 0x9CF && c <= 0x9D6)
            return true;
         if (c >= 0x9D8 && c <= 0x9DB)
            return true;
         if (c >= 0xA0B && c <= 0xA0E)
            return true;
         if (c >= 0xA43 && c <= 0xA46)
            return true;
         if (c >= 0xA4E && c <= 0xA50)
            return true;
         if (c >= 0xA52 && c <= 0xA58)
            return true;
         if (c >= 0xA5F && c <= 0xA65)
            return true;
         if (c >= 0xA77 && c <= 0xA80)
            return true;
         if (c >= 0xAD1 && c <= 0xADF)
            return true;
         if (c >= 0xAF2 && c <= 0xAF8)
            return true;
         if (c >= 0xB4E && c <= 0xB54)
            return true;
         if (c >= 0xB58 && c <= 0xB5B)
            return true;
         if (c >= 0xB78 && c <= 0xB81)
            return true;
         if (c >= 0xB8B && c <= 0xB8D)
            return true;
         if (c >= 0xB96 && c <= 0xB98)
            return true;
         if (c >= 0xBA0 && c <= 0xBA2)
            return true;
         if (c >= 0xBA5 && c <= 0xBA7)
            return true;
         if (c >= 0xBAB && c <= 0xBAD)
            return true;
         if (c >= 0xBBA && c <= 0xBBD)
            return true;
         if (c >= 0xBC3 && c <= 0xBC5)
            return true;
         if (c >= 0xBD1 && c <= 0xBD6)
            return true;
         if (c >= 0xBD8 && c <= 0xBE5)
            return true;
         if (c >= 0xBFB && c <= 0xBFF)
            return true;
         if (c >= 0xC4E && c <= 0xC54)
            return true;
         if (c >= 0xC70 && c <= 0xC76)
            return true;
         if (c >= 0xCCE && c <= 0xCD4)
            return true;
         if (c >= 0xCD7 && c <= 0xCDC)
            return true;
         if (c >= 0xCF4 && c <= 0xCFF)
            return true;
         if (c >= 0xD50 && c <= 0xD53)
            return true;
         if (c >= 0xD97 && c <= 0xD99)
            return true;
         if (c >= 0xDC7 && c <= 0xDC9)
            return true;
         if (c >= 0xDCB && c <= 0xDCE)
            return true;
         if (c >= 0xDE0 && c <= 0xDE5)
            return true;
         if (c >= 0xDF5 && c <= 0xE00)
            return true;
         if (c >= 0xE3B && c <= 0xE3E)
            return true;
         if (c >= 0xE5C && c <= 0xE80)
            return true;
         if (c >= 0xEE0 && c <= 0xEFF)
            return true;
         if (c >= 0xF6D && c <= 0xF70)
            return true;
         if (c >= 0xFDB && c <= 0xFFF)
            return true;
         switch (c) {
            case 0xAD:
            case 0x378:
            case 0x379:
            case 0x38B:
            case 0x38D:
            case 0x3A2:
            case 0x530:
            case 0x557:
            case 0x558:
            case 0x58B:
            case 0x58C:
            case 0x590:
            case 0x61C:
            case 0x6DD:
            case 0x70E:
            case 0x70F:
            case 0x74B:
            case 0x74C:
            case 0x7FB:
            case 0x7FC:
            case 0x82E:
            case 0x82F:
            case 0x83F:
            case 0x85C:
            case 0x85D:
            case 0x85F:
            case 0x8E2:
            case 0x984:
            case 0x98D:
            case 0x98E:
            case 0x991:
            case 0x992:
            case 0x9A9:
            case 0x9B1:
            case 0x9BA:
            case 0x9BB:
            case 0x9C5:
            case 0x9C6:
            case 0x9C9:
            case 0x9CA:
            case 0x9DE:
            case 0x9E4:
            case 0x9E5:
            case 0x9FF:
            case 0xA00:
            case 0xA04:
            case 0xA11:
            case 0xA12:
            case 0xA29:
            case 0xA31:
            case 0xA34:
            case 0xA37:
            case 0xA3A:
            case 0xA3B:
            case 0xA3D:
            case 0xA49:
            case 0xA4A:
            case 0xA5D:
            case 0xA84:
            case 0xA8E:
            case 0xA92:
            case 0xAA9:
            case 0xAB1:
            case 0xAB4:
            case 0xABA:
            case 0xABB:
            case 0xAC6:
            case 0xACA:
            case 0xACE:
            case 0xACF:
            case 0xAE4:
            case 0xAE5:
            case 0xB00:
            case 0xB04:
            case 0xB0D:
            case 0xB0E:
            case 0xB11:
            case 0xB12:
            case 0xB29:
            case 0xB31:
            case 0xB34:
            case 0xB3A:
            case 0xB3B:
            case 0xB45:
            case 0xB46:
            case 0xB49:
            case 0xB4A:
            case 0xB5E:
            case 0xB64:
            case 0xB65:
            case 0xB84:
            case 0xB91:
            case 0xB9B:
            case 0xB9D:
            case 0xBC9:
            case 0xBCE:
            case 0xBCF:
            case 0xC0D:
            case 0xC11:
            case 0xC29:
            case 0xC3A:
            case 0xC3B:
            case 0xC45:
            case 0xC49:
            case 0xC57:
            case 0xC5B:
            case 0xC5C:
            case 0xC5E:
            case 0xC5F:
            case 0xC64:
            case 0xC65:
            case 0xC8D:
            case 0xC91:
            case 0xCA9:
            case 0xCB4:
            case 0xCBA:
            case 0xCBB:
            case 0xCC5:
            case 0xCC9:
            case 0xCDF:
            case 0xCE4:
            case 0xCE5:
            case 0xCF0:
            case 0xD0D:
            case 0xD11:
            case 0xD45:
            case 0xD49:
            case 0xD64:
            case 0xD65:
            case 0xD80:
            case 0xD84:
            case 0xDB2:
            case 0xDBC:
            case 0xDBE:
            case 0xDBF:
            case 0xDD5:
            case 0xDD7:
            case 0xDF0:
            case 0xDF1:
            case 0xE83:
            case 0xE85:
            case 0xE8B:
            case 0xEA4:
            case 0xEA6:
            case 0xEBE:
            case 0xEBF:
            case 0xEC5:
            case 0xEC7:
            case 0xECF:
            case 0xEDA:
            case 0xEDB:
            case 0xF48:
            case 0xF98:
            case 0xFBD:
            case 0xFCD:
               return true;
         }
      } else if (c >= 0x1000 && c < 0x2000) {
         if (c >= 0x10C8 && c <= 0x10CC)
            return true;
         if (c >= 0x137D && c <= 0x137F)
            return true;
         if (c >= 0x139A && c <= 0x139F)
            return true;
         if (c >= 0x169D && c <= 0x169F)
            return true;
         if (c >= 0x16F9 && c <= 0x16FF)
            return true;
         if (c >= 0x1716 && c <= 0x171E)
            return true;
         if (c >= 0x1737 && c <= 0x173F)
            return true;
         if (c >= 0x1754 && c <= 0x175F)
            return true;
         if (c >= 0x1774 && c <= 0x177F)
            return true;
         if (c >= 0x17EA && c <= 0x17EF)
            return true;
         if (c >= 0x17FA && c <= 0x17FF)
            return true;
         if (c >= 0x181A && c <= 0x181F)
            return true;
         if (c >= 0x1879 && c <= 0x187F)
            return true;
         if (c >= 0x18AB && c <= 0x18AF)
            return true;
         if (c >= 0x18F6 && c <= 0x18FF)
            return true;
         if (c >= 0x192C && c <= 0x192F)
            return true;
         if (c >= 0x193C && c <= 0x193F)
            return true;
         if (c >= 0x1941 && c <= 0x1943)
            return true;
         if (c >= 0x1975 && c <= 0x197F)
            return true;
         if (c >= 0x19AC && c <= 0x19AF)
            return true;
         if (c >= 0x19CA && c <= 0x19CF)
            return true;
         if (c >= 0x19DB && c <= 0x19DD)
            return true;
         if (c >= 0x1A8A && c <= 0x1A8F)
            return true;
         if (c >= 0x1A9A && c <= 0x1A9F)
            return true;
         if (c >= 0x1ACF && c <= 0x1AFF)
            return true;
         if (c >= 0x1B4D && c <= 0x1B4F)
            return true;
         if (c >= 0x1BF4 && c <= 0x1BFB)
            return true;
         if (c >= 0x1C38 && c <= 0x1C3A)
            return true;
         if (c >= 0x1C4A && c <= 0x1C4C)
            return true;
         if (c >= 0x1C89 && c <= 0x1C8F)
            return true;
         if (c >= 0x1CC8 && c <= 0x1CCF)
            return true;
         if (c >= 0x1CFB && c <= 0x1CFF)
            return true;
         switch (c) {
            case 0x10C6:
            case 0x10CE:
            case 0x10CF:
            case 0x1249:
            case 0x124E:
            case 0x124F:
            case 0x1257:
            case 0x1259:
            case 0x125E:
            case 0x125F:
            case 0x1289:
            case 0x128E:
            case 0x128F:
            case 0x12B1:
            case 0x12B6:
            case 0x12B7:
            case 0x12BF:
            case 0x12C1:
            case 0x12C6:
            case 0x12C7:
            case 0x12D7:
            case 0x1311:
            case 0x1316:
            case 0x1317:
            case 0x135B:
            case 0x135C:
            case 0x13F6:
            case 0x13F7:
            case 0x13FE:
            case 0x13FF:
            case 0x176D:
            case 0x1771:
            case 0x17DE:
            case 0x17DF:
            case 0x180E:
            case 0x191F:
            case 0x196E:
            case 0x196F:
            case 0x1A1C:
            case 0x1A1D:
            case 0x1A5F:
            case 0x1A7D:
            case 0x1A7E:
            case 0x1AAE:
            case 0x1AAF:
            case 0x1B7F:
            case 0x1CBB:
            case 0x1CBC:
            case 0x1F16:
            case 0x1F17:
            case 0x1F1E:
            case 0x1F1F:
            case 0x1F46:
            case 0x1F47:
            case 0x1F4E:
            case 0x1F4F:
            case 0x1F58:
            case 0x1F5A:
            case 0x1F5C:
            case 0x1F5E:
            case 0x1F7E:
            case 0x1F7F:
            case 0x1FB5:
            case 0x1FC5:
            case 0x1FD4:
            case 0x1FD5:
            case 0x1FDC:
            case 0x1FF0:
            case 0x1FF1:
            case 0x1FF5:
            case 0x1FFF:
               return true;
         }
      } else if (c >= 0x2000 && c < 0x3000) {
         if (c >= 0x200B && c <= 0x200F)
            return true;
         if (c >= 0x202A && c <= 0x202E)
            return true;
         if (c >= 0x2060 && c <= 0x206F)
            return true;
         if (c >= 0x209D && c <= 0x209F)
            return true;
         if (c >= 0x20C1 && c <= 0x20CF)
            return true;
         if (c >= 0x20F1 && c <= 0x20FF)
            return true;
         if (c >= 0x218C && c <= 0x218F)
            return true;
         if (c >= 0x2427 && c <= 0x243F)
            return true;
         if (c >= 0x244B && c <= 0x245F)
            return true;
         if (c >= 0x2CF4 && c <= 0x2CF8)
            return true;
         if (c >= 0x2D28 && c <= 0x2D2C)
            return true;
         if (c >= 0x2D68 && c <= 0x2D6E)
            return true;
         if (c >= 0x2D71 && c <= 0x2D7E)
            return true;
         if (c >= 0x2D97 && c <= 0x2D9F)
            return true;
         if (c >= 0x2E5E && c <= 0x2E7F)
            return true;
         if (c >= 0x2EF4 && c <= 0x2EFF)
            return true;
         if (c >= 0x2FD6 && c <= 0x2FEF)
            return true;
         if (c >= 0x2FFC && c <= 0x2FFF)
            return true;
         switch (c) {
            case 0x2072:
            case 0x2073:
            case 0x208F:
            case 0x2B74:
            case 0x2B75:
            case 0x2B96:
            case 0x2D26:
            case 0x2D2E:
            case 0x2D2F:
            case 0x2DA7:
            case 0x2DAF:
            case 0x2DB7:
            case 0x2DBF:
            case 0x2DC7:
            case 0x2DCF:
            case 0x2DD7:
            case 0x2DDF:
            case 0x2E9A:
               return true;
         }
      } else if (c >= 0x3000 && c < 0x4000) {
         if (c >= 0x3100 && c <= 0x3104)
            return true;
         if (c >= 0x31E4 && c <= 0x31EF)
            return true;
         switch (c) {
            case 0x3040:
            case 0x3097:
            case 0x3098:
            case 0x3130:
            case 0x318F:
            case 0x321F:
               return true;
         }
      } else if (c >= 0xA000 && c < 0xB000) {
         if (c >= 0xA48D && c <= 0xA48F)
            return true;
         if (c >= 0xA4C7 && c <= 0xA4CF)
            return true;
         if (c >= 0xA62C && c <= 0xA63F)
            return true;
         if (c >= 0xA6F8 && c <= 0xA6FF)
            return true;
         if (c >= 0xA7CB && c <= 0xA7CF)
            return true;
         if (c >= 0xA7DA && c <= 0xA7F1)
            return true;
         if (c >= 0xA82D && c <= 0xA82F)
            return true;
         if (c >= 0xA83A && c <= 0xA83F)
            return true;
         if (c >= 0xA878 && c <= 0xA87F)
            return true;
         if (c >= 0xA8C6 && c <= 0xA8CD)
            return true;
         if (c >= 0xA8DA && c <= 0xA8DF)
            return true;
         if (c >= 0xA954 && c <= 0xA95E)
            return true;
         if (c >= 0xA97D && c <= 0xA97F)
            return true;
         if (c >= 0xA9DA && c <= 0xA9DD)
            return true;
         if (c >= 0xAA37 && c <= 0xAA3F)
            return true;
         if (c >= 0xAAC3 && c <= 0xAADA)
            return true;
         if (c >= 0xAAF7 && c <= 0xAB00)
            return true;
         if (c >= 0xAB17 && c <= 0xAB1F)
            return true;
         if (c >= 0xAB6C && c <= 0xAB6F)
            return true;
         if (c >= 0xABFA && c <= 0xABFF)
            return true;
         switch (c) {
            case 0xA7D2:
            case 0xA7D4:
            case 0xA9CE:
            case 0xA9FF:
            case 0xAA4E:
            case 0xAA4F:
            case 0xAA5A:
            case 0xAA5B:
            case 0xAB07:
            case 0xAB08:
            case 0xAB0F:
            case 0xAB10:
            case 0xAB27:
            case 0xAB2F:
            case 0xABEE:
            case 0xABEF:
               return true;
         }
      } else if (c >= 0xD000 && c < 0xE000) {
         if (c >= 0xD000 && c <= 0xD7A3)
            return false;
         if (c >= 0xD7B0 && c <= 0xD7C6)
            return false;
         if (c >= 0xD7CB && c <= 0xD7FB)
            return false;
         return true;
      } else if (c >= 0xE000 && c < 0xF000) {
         return true;
      } else if (c >= 0xF000 && c < 0x10000) {
         if (c >= 0xF900 && c <= 0xFA6D)
            return false;
         if (c >= 0xFA70 && c <= 0xFAD9)
            return false;
         if (c >= 0xFB00 && c <= 0xFB06)
            return false;
         if (c >= 0xFB13 && c <= 0xFB17)
            return false;
         if (c >= 0xFB1D && c <= 0xFB36)
            return false;
         if (c >= 0xFB38 && c <= 0xFB3C)
            return false;
         if (c >= 0xFB46 && c <= 0xFBC2)
            return false;
         if (c >= 0xFBD3 && c <= 0xFD8F)
            return false;
         if (c >= 0xFD92 && c <= 0xFDC7)
            return false;
         if (c >= 0xFDF0 && c <= 0xFE19)
            return false;
         if (c >= 0xFE20 && c <= 0xFE52)
            return false;
         if (c >= 0xFE54 && c <= 0xFE66)
            return false;
         if (c >= 0xFE68 && c <= 0xFE6B)
            return false;
         if (c >= 0xFE70 && c <= 0xFE74)
            return false;
         if (c >= 0xFE76 && c <= 0xFEFC)
            return false;
         if (c >= 0xFF01 && c <= 0xFFBE)
            return false;
         if (c >= 0xFFC2 && c <= 0xFFC7)
            return false;
         if (c >= 0xFFCA && c <= 0xFFCF)
            return false;
         if (c >= 0xFFD2 && c <= 0xFFD7)
            return false;
         if (c >= 0xFFDA && c <= 0xFFDC)
            return false;
         if (c >= 0xFFE0 && c <= 0xFFE6)
            return false;
         if (c >= 0xFFE8 && c <= 0xFFEE)
            return false;
         switch (c) {
            case 0xFB3E:
            case 0xFB40:
            case 0xFB41:
            case 0xFB43:
            case 0xFB44:
            case 0xFDCF:
            case 0xFFFC:
            case 0xFFFD:
               return false;
         }
         return true;
      } else if (c >= 0x10000 && c < 0x11000) {
         if (c >= 0x1005E && c <= 0x1007F)
            return true;
         if (c >= 0x100FB && c <= 0x100FF)
            return true;
         if (c >= 0x10103 && c <= 0x10106)
            return true;
         if (c >= 0x10134 && c <= 0x10136)
            return true;
         if (c >= 0x1019D && c <= 0x1019F)
            return true;
         if (c >= 0x101A1 && c <= 0x101CF)
            return true;
         if (c >= 0x101FE && c <= 0x1027F)
            return true;
         if (c >= 0x1029D && c <= 0x1029F)
            return true;
         if (c >= 0x102D1 && c <= 0x102DF)
            return true;
         if (c >= 0x102FC && c <= 0x102FF)
            return true;
         if (c >= 0x10324 && c <= 0x1032C)
            return true;
         if (c >= 0x1034B && c <= 0x1034F)
            return true;
         if (c >= 0x1037B && c <= 0x1037F)
            return true;
         if (c >= 0x103C4 && c <= 0x103C7)
            return true;
         if (c >= 0x103D6 && c <= 0x103FF)
            return true;
         if (c >= 0x104AA && c <= 0x104AF)
            return true;
         if (c >= 0x104D4 && c <= 0x104D7)
            return true;
         if (c >= 0x104FC && c <= 0x104FF)
            return true;
         if (c >= 0x10528 && c <= 0x1052F)
            return true;
         if (c >= 0x10564 && c <= 0x1056E)
            return true;
         if (c >= 0x105BD && c <= 0x105FF)
            return true;
         if (c >= 0x10737 && c <= 0x1073F)
            return true;
         if (c >= 0x10756 && c <= 0x1075F)
            return true;
         if (c >= 0x10768 && c <= 0x1077F)
            return true;
         if (c >= 0x107BB && c <= 0x107FF)
            return true;
         if (c >= 0x10839 && c <= 0x1083B)
            return true;
         if (c >= 0x1089F && c <= 0x108A6)
            return true;
         if (c >= 0x108B0 && c <= 0x108DF)
            return true;
         if (c >= 0x108F6 && c <= 0x108FA)
            return true;
         if (c >= 0x1091C && c <= 0x1091E)
            return true;
         if (c >= 0x1093A && c <= 0x1093E)
            return true;
         if (c >= 0x10940 && c <= 0x1097F)
            return true;
         if (c >= 0x109B8 && c <= 0x109BB)
            return true;
         if (c >= 0x10A07 && c <= 0x10A0B)
            return true;
         if (c >= 0x10A3B && c <= 0x10A3E)
            return true;
         if (c >= 0x10A49 && c <= 0x10A4F)
            return true;
         if (c >= 0x10A59 && c <= 0x10A5F)
            return true;
         if (c >= 0x10AA0 && c <= 0x10ABF)
            return true;
         if (c >= 0x10AE7 && c <= 0x10AEA)
            return true;
         if (c >= 0x10AF7 && c <= 0x10AFF)
            return true;
         if (c >= 0x10B36 && c <= 0x10B38)
            return true;
         if (c >= 0x10B73 && c <= 0x10B77)
            return true;
         if (c >= 0x10B92 && c <= 0x10B98)
            return true;
         if (c >= 0x10B9D && c <= 0x10BA8)
            return true;
         if (c >= 0x10BB0 && c <= 0x10BFF)
            return true;
         if (c >= 0x10C49 && c <= 0x10C7F)
            return true;
         if (c >= 0x10CB3 && c <= 0x10CBF)
            return true;
         if (c >= 0x10CF3 && c <= 0x10CF9)
            return true;
         if (c >= 0x10D28 && c <= 0x10D2F)
            return true;
         if (c >= 0x10D3A && c <= 0x10E5F)
            return true;
         if (c >= 0x10EB2 && c <= 0x10EFC)
            return true;
         if (c >= 0x10F28 && c <= 0x10F2F)
            return true;
         if (c >= 0x10F5A && c <= 0x10F6F)
            return true;
         if (c >= 0x10F8A && c <= 0x10FAF)
            return true;
         if (c >= 0x10FCC && c <= 0x10FDF)
            return true;
         if (c >= 0x10FF7 && c <= 0x10FFF)
            return true;
         switch (c) {
            case 0x1000C:
            case 0x10027:
            case 0x1003B:
            case 0x1003E:
            case 0x1004E:
            case 0x1004F:
            case 0x1018F:
            case 0x1039E:
            case 0x1049E:
            case 0x1049F:
            case 0x1057B:
            case 0x1058B:
            case 0x10593:
            case 0x10596:
            case 0x105A2:
            case 0x105B2:
            case 0x105BA:
            case 0x10786:
            case 0x107B1:
            case 0x10806:
            case 0x10807:
            case 0x10809:
            case 0x10836:
            case 0x1083D:
            case 0x1083E:
            case 0x10856:
            case 0x108F3:
            case 0x109D0:
            case 0x109D1:
            case 0x10A04:
            case 0x10A14:
            case 0x10A18:
            case 0x10A36:
            case 0x10A37:
            case 0x10B56:
            case 0x10B57:
            case 0x10E7F:
            case 0x10EAA:
            case 0x10EAE:
            case 0x10EAF:
               return true;
         }
      } else if (c >= 0x11000 && c < 0x12000) {
         if (c >= 0x1104E && c <= 0x11051)
            return true;
         if (c >= 0x11076 && c <= 0x1107E)
            return true;
         if (c >= 0x110C3 && c <= 0x110CF)
            return true;
         if (c >= 0x110E9 && c <= 0x110EF)
            return true;
         if (c >= 0x110FA && c <= 0x110FF)
            return true;
         if (c >= 0x11148 && c <= 0x1114F)
            return true;
         if (c >= 0x11177 && c <= 0x1117F)
            return true;
         if (c >= 0x111F5 && c <= 0x111FF)
            return true;
         if (c >= 0x11242 && c <= 0x1127F)
            return true;
         if (c >= 0x112AA && c <= 0x112AF)
            return true;
         if (c >= 0x112EB && c <= 0x112EF)
            return true;
         if (c >= 0x112FA && c <= 0x112FF)
            return true;
         if (c >= 0x11351 && c <= 0x11356)
            return true;
         if (c >= 0x11358 && c <= 0x1135C)
            return true;
         if (c >= 0x1136D && c <= 0x1136F)
            return true;
         if (c >= 0x11375 && c <= 0x113FF)
            return true;
         if (c >= 0x11462 && c <= 0x1147F)
            return true;
         if (c >= 0x114C8 && c <= 0x114CF)
            return true;
         if (c >= 0x114DA && c <= 0x1157F)
            return true;
         if (c >= 0x115DE && c <= 0x115FF)
            return true;
         if (c >= 0x11645 && c <= 0x1164F)
            return true;
         if (c >= 0x1165A && c <= 0x1165F)
            return true;
         if (c >= 0x1166D && c <= 0x1167F)
            return true;
         if (c >= 0x116BA && c <= 0x116BF)
            return true;
         if (c >= 0x116CA && c <= 0x116FF)
            return true;
         if (c >= 0x1172C && c <= 0x1172F)
            return true;
         if (c >= 0x11747 && c <= 0x117FF)
            return true;
         if (c >= 0x1183C && c <= 0x1189F)
            return true;
         if (c >= 0x118F3 && c <= 0x118FE)
            return true;
         if (c >= 0x11947 && c <= 0x1194F)
            return true;
         if (c >= 0x1195A && c <= 0x1199F)
            return true;
         if (c >= 0x119E5 && c <= 0x119FF)
            return true;
         if (c >= 0x11A48 && c <= 0x11A4F)
            return true;
         if (c >= 0x11AA3 && c <= 0x11AAF)
            return true;
         if (c >= 0x11AF9 && c <= 0x11AFF)
            return true;
         if (c >= 0x11B0A && c <= 0x11BFF)
            return true;
         if (c >= 0x11C46 && c <= 0x11C4F)
            return true;
         if (c >= 0x11C6D && c <= 0x11C6F)
            return true;
         if (c >= 0x11CB7 && c <= 0x11CFF)
            return true;
         if (c >= 0x11D37 && c <= 0x11D39)
            return true;
         if (c >= 0x11D48 && c <= 0x11D4F)
            return true;
         if (c >= 0x11D5A && c <= 0x11D5F)
            return true;
         if (c >= 0x11D99 && c <= 0x11D9F)
            return true;
         if (c >= 0x11DAA && c <= 0x11EDF)
            return true;
         if (c >= 0x11EF9 && c <= 0x11EFF)
            return true;
         if (c >= 0x11F3B && c <= 0x11F3D)
            return true;
         if (c >= 0x11F5A && c <= 0x11FAF)
            return true;
         if (c >= 0x11FB1 && c <= 0x11FBF)
            return true;
         if (c >= 0x11FF2 && c <= 0x11FFE)
            return true;
         switch (c) {
            case 0x110BD:
            case 0x11135:
            case 0x111E0:
            case 0x11212:
            case 0x11287:
            case 0x11289:
            case 0x1128E:
            case 0x1129E:
            case 0x11304:
            case 0x1130D:
            case 0x1130E:
            case 0x11311:
            case 0x11312:
            case 0x11329:
            case 0x11331:
            case 0x11334:
            case 0x1133A:
            case 0x11345:
            case 0x11346:
            case 0x11349:
            case 0x1134A:
            case 0x1134E:
            case 0x1134F:
            case 0x11364:
            case 0x11365:
            case 0x1145C:
            case 0x115B6:
            case 0x115B7:
            case 0x1171B:
            case 0x1171C:
            case 0x11907:
            case 0x11908:
            case 0x1190A:
            case 0x1190B:
            case 0x11914:
            case 0x11917:
            case 0x11936:
            case 0x11939:
            case 0x1193A:
            case 0x119A8:
            case 0x119A9:
            case 0x119D8:
            case 0x119D9:
            case 0x11C09:
            case 0x11C37:
            case 0x11C90:
            case 0x11C91:
            case 0x11CA8:
            case 0x11D07:
            case 0x11D0A:
            case 0x11D3B:
            case 0x11D3E:
            case 0x11D66:
            case 0x11D69:
            case 0x11D8F:
            case 0x11D92:
            case 0x11F11:
               return true;
         }
      } else if (c >= 0x12000 && c < 0x13000) {
         if (c >= 0x12000 && c <= 0x12399)
            return false;
         if (c >= 0x12400 && c <= 0x1246E)
            return false;
         if (c >= 0x12470 && c <= 0x12474)
            return false;
         if (c >= 0x12480 && c <= 0x12543)
            return false;
         if (c >= 0x12F90 && c <= 0x12FF2)
            return false;
         return true;
      } else if (c >= 0x13000 && c < 0x14000) {
         if (c >= 0x13000 && c <= 0x1342F)
            return false;
         if (c >= 0x13440 && c <= 0x13455)
            return false;
         return true;
      } else if (c >= 0x14000 && c < 0x15000) {
         if (c >= 0x14400 && c <= 0x14646)
            return false;
         return true;
      } else if (c >= 0x15000 && c < 0x16000) {
         return true;
      } else if (c >= 0x16000 && c < 0x17000) {
         if (c >= 0x16800 && c <= 0x16A38)
            return false;
         if (c >= 0x16A40 && c <= 0x16A5E)
            return false;
         if (c >= 0x16A60 && c <= 0x16A69)
            return false;
         if (c >= 0x16A6E && c <= 0x16ABE)
            return false;
         if (c >= 0x16AC0 && c <= 0x16AC9)
            return false;
         if (c >= 0x16AD0 && c <= 0x16AED)
            return false;
         if (c >= 0x16AF0 && c <= 0x16AF5)
            return false;
         if (c >= 0x16B00 && c <= 0x16B45)
            return false;
         if (c >= 0x16B50 && c <= 0x16B59)
            return false;
         if (c >= 0x16B5B && c <= 0x16B61)
            return false;
         if (c >= 0x16B63 && c <= 0x16B77)
            return false;
         if (c >= 0x16B7D && c <= 0x16B8F)
            return false;
         if (c >= 0x16E40 && c <= 0x16E9A)
            return false;
         if (c >= 0x16F00 && c <= 0x16F4A)
            return false;
         if (c >= 0x16F4F && c <= 0x16F87)
            return false;
         if (c >= 0x16F8F && c <= 0x16F9F)
            return false;
         if (c >= 0x16FE0 && c <= 0x16FE4)
            return false;
         switch (c) {
            case 0x16FF0:
            case 0x16FF1:
               return false;
         }
         return true;
      } else if (c >= 0x18000 && c < 0x19000) {
         if (c >= 0x187F8 && c <= 0x187FF)
            return true;
         if (c >= 0x18CD6 && c <= 0x18CFF)
            return true;
         if (c >= 0x18D09 && c <= 0x18FFF)
            return true;
      } else if (c >= 0x19000 && c < 0x1A000) {
         return true;
      } else if (c >= 0x1A000 && c < 0x1B000) {
         if (c >= 0x1AFF0 && c <= 0x1AFF3)
            return false;
         if (c >= 0x1AFF5 && c <= 0x1AFFB)
            return false;
         switch (c) {
            case 0x1AFFD:
            case 0x1AFFE:
               return false;
         }
         return true;
      } else if (c >= 0x1B000 && c < 0x1C000) {
         if (c >= 0x1B000 && c <= 0x1B122)
            return false;
         if (c >= 0x1B150 && c <= 0x1B152)
            return false;
         if (c >= 0x1B164 && c <= 0x1B167)
            return false;
         if (c >= 0x1B170 && c <= 0x1B2FB)
            return false;
         if (c >= 0x1BC00 && c <= 0x1BC6A)
            return false;
         if (c >= 0x1BC70 && c <= 0x1BC7C)
            return false;
         if (c >= 0x1BC80 && c <= 0x1BC88)
            return false;
         if (c >= 0x1BC90 && c <= 0x1BC99)
            return false;
         if (c >= 0x1BC9C && c <= 0x1BC9F)
            return false;
         switch (c) {
            case 0x1B132:
            case 0x1B155:
               return false;
         }
         return true;
      } else if (c >= 0x1C000 && c < 0x1D000) {
         if (c >= 0x1CF00 && c <= 0x1CF2D)
            return false;
         if (c >= 0x1CF30 && c <= 0x1CF46)
            return false;
         if (c >= 0x1CF50 && c <= 0x1CFC3)
            return false;
         return true;
      } else if (c >= 0x1D000 && c < 0x1E000) {
         if (c >= 0x1D0F6 && c <= 0x1D0FF)
            return true;
         if (c >= 0x1D173 && c <= 0x1D17A)
            return true;
         if (c >= 0x1D1EB && c <= 0x1D1FF)
            return true;
         if (c >= 0x1D246 && c <= 0x1D2BF)
            return true;
         if (c >= 0x1D2D4 && c <= 0x1D2DF)
            return true;
         if (c >= 0x1D2F4 && c <= 0x1D2FF)
            return true;
         if (c >= 0x1D357 && c <= 0x1D35F)
            return true;
         if (c >= 0x1D379 && c <= 0x1D3FF)
            return true;
         if (c >= 0x1D547 && c <= 0x1D549)
            return true;
         if (c >= 0x1DA8C && c <= 0x1DA9A)
            return true;
         if (c >= 0x1DAB0 && c <= 0x1DEFF)
            return true;
         if (c >= 0x1DF1F && c <= 0x1DF24)
            return true;
         if (c >= 0x1DF2B && c <= 0x1DFFF)
            return true;
         switch (c) {
            case 0x1D127:
            case 0x1D128:
            case 0x1D455:
            case 0x1D49D:
            case 0x1D4A0:
            case 0x1D4A1:
            case 0x1D4A3:
            case 0x1D4A4:
            case 0x1D4A7:
            case 0x1D4A8:
            case 0x1D4AD:
            case 0x1D4BA:
            case 0x1D4BC:
            case 0x1D4C4:
            case 0x1D506:
            case 0x1D50B:
            case 0x1D50C:
            case 0x1D515:
            case 0x1D51D:
            case 0x1D53A:
            case 0x1D53F:
            case 0x1D545:
            case 0x1D551:
            case 0x1D6A6:
            case 0x1D6A7:
            case 0x1D7CC:
            case 0x1D7CD:
            case 0x1DAA0:
               return true;
         }
      } else if (c >= 0x1E000 && c < 0x1F000) {
         if (c >= 0x1E000 && c <= 0x1E006)
            return false;
         if (c >= 0x1E008 && c <= 0x1E018)
            return false;
         if (c >= 0x1E01B && c <= 0x1E021)
            return false;
         if (c >= 0x1E026 && c <= 0x1E02A)
            return false;
         if (c >= 0x1E030 && c <= 0x1E06D)
            return false;
         if (c >= 0x1E100 && c <= 0x1E12C)
            return false;
         if (c >= 0x1E130 && c <= 0x1E13D)
            return false;
         if (c >= 0x1E140 && c <= 0x1E149)
            return false;
         if (c >= 0x1E290 && c <= 0x1E2AE)
            return false;
         if (c >= 0x1E2C0 && c <= 0x1E2F9)
            return false;
         if (c >= 0x1E4D0 && c <= 0x1E4F9)
            return false;
         if (c >= 0x1E7E0 && c <= 0x1E7E6)
            return false;
         if (c >= 0x1E7E8 && c <= 0x1E7EB)
            return false;
         if (c >= 0x1E7F0 && c <= 0x1E7FE)
            return false;
         if (c >= 0x1E800 && c <= 0x1E8C4)
            return false;
         if (c >= 0x1E8C7 && c <= 0x1E8D6)
            return false;
         if (c >= 0x1E900 && c <= 0x1E94B)
            return false;
         if (c >= 0x1E950 && c <= 0x1E959)
            return false;
         if (c >= 0x1EC71 && c <= 0x1ECB4)
            return false;
         if (c >= 0x1ED01 && c <= 0x1ED3D)
            return false;
         if (c >= 0x1EE00 && c <= 0x1EE03)
            return false;
         if (c >= 0x1EE05 && c <= 0x1EE1F)
            return false;
         if (c >= 0x1EE29 && c <= 0x1EE32)
            return false;
         if (c >= 0x1EE34 && c <= 0x1EE37)
            return false;
         if (c >= 0x1EE4D && c <= 0x1EE4F)
            return false;
         if (c >= 0x1EE67 && c <= 0x1EE6A)
            return false;
         if (c >= 0x1EE6C && c <= 0x1EE72)
            return false;
         if (c >= 0x1EE74 && c <= 0x1EE77)
            return false;
         if (c >= 0x1EE79 && c <= 0x1EE7C)
            return false;
         if (c >= 0x1EE80 && c <= 0x1EE89)
            return false;
         if (c >= 0x1EE8B && c <= 0x1EE9B)
            return false;
         if (c >= 0x1EEA1 && c <= 0x1EEA3)
            return false;
         if (c >= 0x1EEA5 && c <= 0x1EEA9)
            return false;
         if (c >= 0x1EEAB && c <= 0x1EEBB)
            return false;
         switch (c) {
            case 0x1E023:
            case 0x1E024:
            case 0x1E08F:
            case 0x1E14E:
            case 0x1E14F:
            case 0x1E2FF:
            case 0x1E7ED:
            case 0x1E7EE:
            case 0x1E95E:
            case 0x1E95F:
            case 0x1EE21:
            case 0x1EE22:
            case 0x1EE24:
            case 0x1EE27:
            case 0x1EE39:
            case 0x1EE3B:
            case 0x1EE42:
            case 0x1EE47:
            case 0x1EE49:
            case 0x1EE4B:
            case 0x1EE51:
            case 0x1EE52:
            case 0x1EE54:
            case 0x1EE57:
            case 0x1EE59:
            case 0x1EE5B:
            case 0x1EE5D:
            case 0x1EE5F:
            case 0x1EE61:
            case 0x1EE62:
            case 0x1EE64:
            case 0x1EE7E:
            case 0x1EEF0:
            case 0x1EEF1:
               return false;
         }
         return true;
      } else if (c >= 0x1F000 && c < 0x20000) {
         if (c >= 0x1F02C && c <= 0x1F02F)
            return true;
         if (c >= 0x1F094 && c <= 0x1F09F)
            return true;
         if (c >= 0x1F0F6 && c <= 0x1F0FF)
            return true;
         if (c >= 0x1F1AE && c <= 0x1F1E5)
            return true;
         if (c >= 0x1F203 && c <= 0x1F20F)
            return true;
         if (c >= 0x1F23C && c <= 0x1F23F)
            return true;
         if (c >= 0x1F249 && c <= 0x1F24F)
            return true;
         if (c >= 0x1F252 && c <= 0x1F25F)
            return true;
         if (c >= 0x1F266 && c <= 0x1F2FF)
            return true;
         if (c >= 0x1F6D8 && c <= 0x1F6DB)
            return true;
         if (c >= 0x1F6ED && c <= 0x1F6EF)
            return true;
         if (c >= 0x1F6FD && c <= 0x1F6FF)
            return true;
         if (c >= 0x1F777 && c <= 0x1F77A)
            return true;
         if (c >= 0x1F7DA && c <= 0x1F7DF)
            return true;
         if (c >= 0x1F7EC && c <= 0x1F7EF)
            return true;
         if (c >= 0x1F7F1 && c <= 0x1F7FF)
            return true;
         if (c >= 0x1F80C && c <= 0x1F80F)
            return true;
         if (c >= 0x1F848 && c <= 0x1F84F)
            return true;
         if (c >= 0x1F85A && c <= 0x1F85F)
            return true;
         if (c >= 0x1F888 && c <= 0x1F88F)
            return true;
         if (c >= 0x1F8B2 && c <= 0x1F8FF)
            return true;
         if (c >= 0x1FA54 && c <= 0x1FA5F)
            return true;
         if (c >= 0x1FA7D && c <= 0x1FA7F)
            return true;
         if (c >= 0x1FA89 && c <= 0x1FA8F)
            return true;
         if (c >= 0x1FAC6 && c <= 0x1FACD)
            return true;
         if (c >= 0x1FADC && c <= 0x1FADF)
            return true;
         if (c >= 0x1FAE9 && c <= 0x1FAEF)
            return true;
         if (c >= 0x1FAF9 && c <= 0x1FAFF)
            return true;
         if (c >= 0x1FBCB && c <= 0x1FBEF)
            return true;
         if (c >= 0x1FBFA && c <= 0x1FFFF)
            return true;
         switch (c) {
            case 0x1F0AF:
            case 0x1F0B0:
            case 0x1F0C0:
            case 0x1F0D0:
            case 0x1F8AE:
            case 0x1F8AF:
            case 0x1FA6E:
            case 0x1FA6F:
            case 0x1FABE:
            case 0x1FB93:
               return true;
         }
      } else if (c >= 0x2A000 && c < 0x2B000) {
         if (c >= 0x2A6E0 && c <= 0x2A6FF)
            return true;
      } else if (c >= 0x2B000 && c < 0x2C000) {
         if (c >= 0x2B73A && c <= 0x2B73F)
            return true;
         switch (c) {
            case 0x2B81E:
            case 0x2B81F:
               return true;
         }
      } else if (c >= 0x2C000 && c < 0x2D000) {
         if (c >= 0x2CEA2 && c <= 0x2CEAF)
            return true;
      } else if (c >= 0x2E000 && c < 0x2F000) {
         if (c >= 0x2EBE1 && c <= 0x2EFFF)
            return true;
      } else if (c >= 0x2F000 && c < 0x30000) {
         if (c >= 0x2F800 && c <= 0x2FA1D)
            return false;
         return true;
      } else if (c >= 0x31000 && c < 0x32000) {
         if (c >= 0x3134B && c <= 0x3134F)
            return true;
      } else if (c >= 0x32000 && c < 0x33000) {
         if (c >= 0x32000 && c <= 0x323AF)
            return false;
         return true;
      } else if (c >= 0x33000 && c < 0xE0000) {
         return true;
      } else if (c >= 0xE0000 && c < 0xE1000) {
         if (c >= 0xE0100 && c <= 0xE01EF)
            return false;
         return true;
      } else if (c >= 0xE1000 && c < 0x110000) {
         return true;
      }
      return false;
   }
}
