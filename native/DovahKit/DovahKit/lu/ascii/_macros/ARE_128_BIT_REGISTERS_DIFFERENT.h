// Place in the parentheses of an `if` statement.
#define ARE_128_BIT_REGISTERS_DIFFERENT(ma, mb) _mm_movemask_epi8(_mm_cmpeq_epi8(ma, mb)) != 0xFFFF