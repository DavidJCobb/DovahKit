#pragma once
#include <cstdint>
#include <type_traits>

namespace nifDK {
   //
   // Values are probably hashes of some kind; ordering obviously doesn't 
   // correspond to chronology because DLC1 (Dawnguard) values are mixed in 
   // amongst everything else.
   //
   enum class HavokMaterial : uint32_t {
      stone_broken         =  131151687,
      wood_light           =  365420259,
      snow                 =  398949039,
      gravel               =  428587608,
      metal_chain          =  438912228,
      bottle               =  493553910,
      wood                 =  500811281,
      skin                 =  591247106,
      dlc1_deer_skin       =  617099282, // meshes\dlc01\clutter\dlc01deerskin.nif
      barrel               =  732141076,
      ceramic_medium       =  781661019,
      basket               =  790784366,
      ice                  =  873356572,
      stone_stairs         =  899511101,
      water                = 1024582599,
      draugr_unknown       = 1028101969, // actors\draugr\character assets\skeletons.nif
      blade_1_hand         = 1060167844,
      book                 = 1264672850,
      carpet               = 1286705471,
      metal_solid          = 1288358971,
      axe_1_hand           = 1305674443,
      unk_amulet           = 1440721808, // armor\draugr\draugrbootsfemale_go.nif or armor\amuletsandrings\amuletgnd.nif
      wood_stairs          = 1461712277, // SKY_HAV_MAT_STAIRS_WOOD
      mud                  = 1486385281,
      boulder_small        = 1550912982,
      snow_stairs          = 1560365355,
      stone_heavy          = 1570821952,
      dragon_unknown       = 1574477864, // actors\dragon\character assets\skeleton.nif
      varied_unknown       = 1591009235, // trap objects or clutter\displaycases\displaycaselgangled01.nif or actors\deer\character assets\skeleton.nif
      bows_and_staves      = 1607128641,
      wood_as_stairs       = 1803571212, // but what's the difference between SKY_HAV_MAT_MATERIAL_WOOD_AS_STAIRS and SKY_HAV_MAT_STAIRS_WOOD ??
      grass                = 1848600814,
      boulder_large        = 1885326971,
      stone_as_stairs      = 1886078335, // but what's the difference between SKY_HAV_MAT_MATERIAL_STONE_AS_STAIRS and SKY_HAV_MAT_STAIRS_STONE ??
      blade_2_hand         = 2022742644,
      bottle_small         = 2025794648,
      sand                 = 2168343821,
      metal_heavy          = 2229413539,
      dlc1_sabre_skin      = 2290050264, // meshes\dlc01\clutter\dlc01sabrecatpelt.nif
      dragon               = 2518321175,
      blade_1_hand_small   = 2617944780,
      skin_small           = 2632367422,
      stone_broken_stairs  = 2892392795,
      skin_large           = 2965929619,
      organic              = 2974920155,
      bone                 = 3049421844,
      wood_heavy           = 3070783559,
      chain                = 3074114406,
      dirt                 = 3106094762,
      armor_light          = 3424720541,
      shield_light         = 3448167928,
      coin                 = 3589100606,
      shield_heavy         = 3702389584,
      armor_heavy          = 3708432437,
      arrow                = 3725505938,
      glass                = 3739830338,
      stone                = 3741512247,
      cloth                = 3839073443,
      blunt_2_hand         = 3969592277,
      dlc1_swinging_bridge = 4239621792, // meshes\dlc01\prototype\dlc1protoswingingbridge.nif
      boulder_medium       = 4283869410,
   };
}