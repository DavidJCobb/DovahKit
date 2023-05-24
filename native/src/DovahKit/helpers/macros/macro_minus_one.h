
/*

Generated via JavaScript:

{
   let pushes = "";
   let out    = "";

   for(let i = 1; i < 128; ++i) {
      pushes += `#pragma push_macro("MACRO_${i}_MINUS_1")\n`;

      let line = `#define MACRO_${i}_MINUS_1 ${i - 1}`;
      out += line + "\n";
   }
   pushes + "\n" + out;
}

*/

// untested
#define MACRO_MINUS_1(n) MACRO_EXPAND(MACRO_##n##_MINUS_1)
#define MACRO_EXPAND(...) __VA_ARGS__

#pragma push_macro("MACRO_1_MINUS_1")
#pragma push_macro("MACRO_2_MINUS_1")
#pragma push_macro("MACRO_3_MINUS_1")
#pragma push_macro("MACRO_4_MINUS_1")
#pragma push_macro("MACRO_5_MINUS_1")
#pragma push_macro("MACRO_6_MINUS_1")
#pragma push_macro("MACRO_7_MINUS_1")
#pragma push_macro("MACRO_8_MINUS_1")
#pragma push_macro("MACRO_9_MINUS_1")
#pragma push_macro("MACRO_10_MINUS_1")
#pragma push_macro("MACRO_11_MINUS_1")
#pragma push_macro("MACRO_12_MINUS_1")
#pragma push_macro("MACRO_13_MINUS_1")
#pragma push_macro("MACRO_14_MINUS_1")
#pragma push_macro("MACRO_15_MINUS_1")
#pragma push_macro("MACRO_16_MINUS_1")
#pragma push_macro("MACRO_17_MINUS_1")
#pragma push_macro("MACRO_18_MINUS_1")
#pragma push_macro("MACRO_19_MINUS_1")
#pragma push_macro("MACRO_20_MINUS_1")
#pragma push_macro("MACRO_21_MINUS_1")
#pragma push_macro("MACRO_22_MINUS_1")
#pragma push_macro("MACRO_23_MINUS_1")
#pragma push_macro("MACRO_24_MINUS_1")
#pragma push_macro("MACRO_25_MINUS_1")
#pragma push_macro("MACRO_26_MINUS_1")
#pragma push_macro("MACRO_27_MINUS_1")
#pragma push_macro("MACRO_28_MINUS_1")
#pragma push_macro("MACRO_29_MINUS_1")
#pragma push_macro("MACRO_30_MINUS_1")
#pragma push_macro("MACRO_31_MINUS_1")
#pragma push_macro("MACRO_32_MINUS_1")
#pragma push_macro("MACRO_33_MINUS_1")
#pragma push_macro("MACRO_34_MINUS_1")
#pragma push_macro("MACRO_35_MINUS_1")
#pragma push_macro("MACRO_36_MINUS_1")
#pragma push_macro("MACRO_37_MINUS_1")
#pragma push_macro("MACRO_38_MINUS_1")
#pragma push_macro("MACRO_39_MINUS_1")
#pragma push_macro("MACRO_40_MINUS_1")
#pragma push_macro("MACRO_41_MINUS_1")
#pragma push_macro("MACRO_42_MINUS_1")
#pragma push_macro("MACRO_43_MINUS_1")
#pragma push_macro("MACRO_44_MINUS_1")
#pragma push_macro("MACRO_45_MINUS_1")
#pragma push_macro("MACRO_46_MINUS_1")
#pragma push_macro("MACRO_47_MINUS_1")
#pragma push_macro("MACRO_48_MINUS_1")
#pragma push_macro("MACRO_49_MINUS_1")
#pragma push_macro("MACRO_50_MINUS_1")
#pragma push_macro("MACRO_51_MINUS_1")
#pragma push_macro("MACRO_52_MINUS_1")
#pragma push_macro("MACRO_53_MINUS_1")
#pragma push_macro("MACRO_54_MINUS_1")
#pragma push_macro("MACRO_55_MINUS_1")
#pragma push_macro("MACRO_56_MINUS_1")
#pragma push_macro("MACRO_57_MINUS_1")
#pragma push_macro("MACRO_58_MINUS_1")
#pragma push_macro("MACRO_59_MINUS_1")
#pragma push_macro("MACRO_60_MINUS_1")
#pragma push_macro("MACRO_61_MINUS_1")
#pragma push_macro("MACRO_62_MINUS_1")
#pragma push_macro("MACRO_63_MINUS_1")
#pragma push_macro("MACRO_64_MINUS_1")
#pragma push_macro("MACRO_65_MINUS_1")
#pragma push_macro("MACRO_66_MINUS_1")
#pragma push_macro("MACRO_67_MINUS_1")
#pragma push_macro("MACRO_68_MINUS_1")
#pragma push_macro("MACRO_69_MINUS_1")
#pragma push_macro("MACRO_70_MINUS_1")
#pragma push_macro("MACRO_71_MINUS_1")
#pragma push_macro("MACRO_72_MINUS_1")
#pragma push_macro("MACRO_73_MINUS_1")
#pragma push_macro("MACRO_74_MINUS_1")
#pragma push_macro("MACRO_75_MINUS_1")
#pragma push_macro("MACRO_76_MINUS_1")
#pragma push_macro("MACRO_77_MINUS_1")
#pragma push_macro("MACRO_78_MINUS_1")
#pragma push_macro("MACRO_79_MINUS_1")
#pragma push_macro("MACRO_80_MINUS_1")
#pragma push_macro("MACRO_81_MINUS_1")
#pragma push_macro("MACRO_82_MINUS_1")
#pragma push_macro("MACRO_83_MINUS_1")
#pragma push_macro("MACRO_84_MINUS_1")
#pragma push_macro("MACRO_85_MINUS_1")
#pragma push_macro("MACRO_86_MINUS_1")
#pragma push_macro("MACRO_87_MINUS_1")
#pragma push_macro("MACRO_88_MINUS_1")
#pragma push_macro("MACRO_89_MINUS_1")
#pragma push_macro("MACRO_90_MINUS_1")
#pragma push_macro("MACRO_91_MINUS_1")
#pragma push_macro("MACRO_92_MINUS_1")
#pragma push_macro("MACRO_93_MINUS_1")
#pragma push_macro("MACRO_94_MINUS_1")
#pragma push_macro("MACRO_95_MINUS_1")
#pragma push_macro("MACRO_96_MINUS_1")
#pragma push_macro("MACRO_97_MINUS_1")
#pragma push_macro("MACRO_98_MINUS_1")
#pragma push_macro("MACRO_99_MINUS_1")
#pragma push_macro("MACRO_100_MINUS_1")
#pragma push_macro("MACRO_101_MINUS_1")
#pragma push_macro("MACRO_102_MINUS_1")
#pragma push_macro("MACRO_103_MINUS_1")
#pragma push_macro("MACRO_104_MINUS_1")
#pragma push_macro("MACRO_105_MINUS_1")
#pragma push_macro("MACRO_106_MINUS_1")
#pragma push_macro("MACRO_107_MINUS_1")
#pragma push_macro("MACRO_108_MINUS_1")
#pragma push_macro("MACRO_109_MINUS_1")
#pragma push_macro("MACRO_110_MINUS_1")
#pragma push_macro("MACRO_111_MINUS_1")
#pragma push_macro("MACRO_112_MINUS_1")
#pragma push_macro("MACRO_113_MINUS_1")
#pragma push_macro("MACRO_114_MINUS_1")
#pragma push_macro("MACRO_115_MINUS_1")
#pragma push_macro("MACRO_116_MINUS_1")
#pragma push_macro("MACRO_117_MINUS_1")
#pragma push_macro("MACRO_118_MINUS_1")
#pragma push_macro("MACRO_119_MINUS_1")
#pragma push_macro("MACRO_120_MINUS_1")
#pragma push_macro("MACRO_121_MINUS_1")
#pragma push_macro("MACRO_122_MINUS_1")
#pragma push_macro("MACRO_123_MINUS_1")
#pragma push_macro("MACRO_124_MINUS_1")
#pragma push_macro("MACRO_125_MINUS_1")
#pragma push_macro("MACRO_126_MINUS_1")
#pragma push_macro("MACRO_127_MINUS_1")

#define MACRO_1_MINUS_1 0
#define MACRO_2_MINUS_1 1
#define MACRO_3_MINUS_1 2
#define MACRO_4_MINUS_1 3
#define MACRO_5_MINUS_1 4
#define MACRO_6_MINUS_1 5
#define MACRO_7_MINUS_1 6
#define MACRO_8_MINUS_1 7
#define MACRO_9_MINUS_1 8
#define MACRO_10_MINUS_1 9
#define MACRO_11_MINUS_1 10
#define MACRO_12_MINUS_1 11
#define MACRO_13_MINUS_1 12
#define MACRO_14_MINUS_1 13
#define MACRO_15_MINUS_1 14
#define MACRO_16_MINUS_1 15
#define MACRO_17_MINUS_1 16
#define MACRO_18_MINUS_1 17
#define MACRO_19_MINUS_1 18
#define MACRO_20_MINUS_1 19
#define MACRO_21_MINUS_1 20
#define MACRO_22_MINUS_1 21
#define MACRO_23_MINUS_1 22
#define MACRO_24_MINUS_1 23
#define MACRO_25_MINUS_1 24
#define MACRO_26_MINUS_1 25
#define MACRO_27_MINUS_1 26
#define MACRO_28_MINUS_1 27
#define MACRO_29_MINUS_1 28
#define MACRO_30_MINUS_1 29
#define MACRO_31_MINUS_1 30
#define MACRO_32_MINUS_1 31
#define MACRO_33_MINUS_1 32
#define MACRO_34_MINUS_1 33
#define MACRO_35_MINUS_1 34
#define MACRO_36_MINUS_1 35
#define MACRO_37_MINUS_1 36
#define MACRO_38_MINUS_1 37
#define MACRO_39_MINUS_1 38
#define MACRO_40_MINUS_1 39
#define MACRO_41_MINUS_1 40
#define MACRO_42_MINUS_1 41
#define MACRO_43_MINUS_1 42
#define MACRO_44_MINUS_1 43
#define MACRO_45_MINUS_1 44
#define MACRO_46_MINUS_1 45
#define MACRO_47_MINUS_1 46
#define MACRO_48_MINUS_1 47
#define MACRO_49_MINUS_1 48
#define MACRO_50_MINUS_1 49
#define MACRO_51_MINUS_1 50
#define MACRO_52_MINUS_1 51
#define MACRO_53_MINUS_1 52
#define MACRO_54_MINUS_1 53
#define MACRO_55_MINUS_1 54
#define MACRO_56_MINUS_1 55
#define MACRO_57_MINUS_1 56
#define MACRO_58_MINUS_1 57
#define MACRO_59_MINUS_1 58
#define MACRO_60_MINUS_1 59
#define MACRO_61_MINUS_1 60
#define MACRO_62_MINUS_1 61
#define MACRO_63_MINUS_1 62
#define MACRO_64_MINUS_1 63
#define MACRO_65_MINUS_1 64
#define MACRO_66_MINUS_1 65
#define MACRO_67_MINUS_1 66
#define MACRO_68_MINUS_1 67
#define MACRO_69_MINUS_1 68
#define MACRO_70_MINUS_1 69
#define MACRO_71_MINUS_1 70
#define MACRO_72_MINUS_1 71
#define MACRO_73_MINUS_1 72
#define MACRO_74_MINUS_1 73
#define MACRO_75_MINUS_1 74
#define MACRO_76_MINUS_1 75
#define MACRO_77_MINUS_1 76
#define MACRO_78_MINUS_1 77
#define MACRO_79_MINUS_1 78
#define MACRO_80_MINUS_1 79
#define MACRO_81_MINUS_1 80
#define MACRO_82_MINUS_1 81
#define MACRO_83_MINUS_1 82
#define MACRO_84_MINUS_1 83
#define MACRO_85_MINUS_1 84
#define MACRO_86_MINUS_1 85
#define MACRO_87_MINUS_1 86
#define MACRO_88_MINUS_1 87
#define MACRO_89_MINUS_1 88
#define MACRO_90_MINUS_1 89
#define MACRO_91_MINUS_1 90
#define MACRO_92_MINUS_1 91
#define MACRO_93_MINUS_1 92
#define MACRO_94_MINUS_1 93
#define MACRO_95_MINUS_1 94
#define MACRO_96_MINUS_1 95
#define MACRO_97_MINUS_1 96
#define MACRO_98_MINUS_1 97
#define MACRO_99_MINUS_1 98
#define MACRO_100_MINUS_1 99
#define MACRO_101_MINUS_1 100
#define MACRO_102_MINUS_1 101
#define MACRO_103_MINUS_1 102
#define MACRO_104_MINUS_1 103
#define MACRO_105_MINUS_1 104
#define MACRO_106_MINUS_1 105
#define MACRO_107_MINUS_1 106
#define MACRO_108_MINUS_1 107
#define MACRO_109_MINUS_1 108
#define MACRO_110_MINUS_1 109
#define MACRO_111_MINUS_1 110
#define MACRO_112_MINUS_1 111
#define MACRO_113_MINUS_1 112
#define MACRO_114_MINUS_1 113
#define MACRO_115_MINUS_1 114
#define MACRO_116_MINUS_1 115
#define MACRO_117_MINUS_1 116
#define MACRO_118_MINUS_1 117
#define MACRO_119_MINUS_1 118
#define MACRO_120_MINUS_1 119
#define MACRO_121_MINUS_1 120
#define MACRO_122_MINUS_1 121
#define MACRO_123_MINUS_1 122
#define MACRO_124_MINUS_1 123
#define MACRO_125_MINUS_1 124
#define MACRO_126_MINUS_1 125
#define MACRO_127_MINUS_1 126