//
// Convert two 128-bit registers to lowercase. SIMD equivalent of the following for 
// each character. Operations are interleaved for each input register.
// 
//    c |= (c >= 'A' && c <= 'Z') ? 0x20 : 0x00;
// 
// The following registers must have been defined in advance:
// 
//    `mb_a`      128-bit register containing all values ('A' - 1)
//    `mb_z`      128-bit register containing all values ('Z' - 1)
//
#define CASE_FOLD_REGISTERS_IN_TANDEM(ma, mb) \
   { \
      { \
         auto mask_a = _mm_cmpgt_epi8(ma, mb_a); \
         auto mask_z = _mm_cmplt_epi8(ma, mb_z); \
         mask_a = _mm_and_si128(mask_a, mask_z); \
         mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); \
         ma = _mm_or_si128(ma, mask_a); \
      } \
      { \
         auto mask_a = _mm_cmpgt_epi8(mb, mb_a); \
         auto mask_z = _mm_cmplt_epi8(mb, mb_z); \
         mask_a = _mm_and_si128(mask_a, mask_z); \
         mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); \
         mb = _mm_or_si128(ma, mask_a); \
      } \
   }