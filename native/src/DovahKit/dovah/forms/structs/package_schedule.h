#pragma once
#include <cstdint>
#include "../_common.h"

namespace dovah::loaded_forms::structs {
   struct package_schedule {
      public:
         static constexpr const uint32_t subrecord = 'PSDT';
         
         enum class schedule_month : int8_t {
            any = -1,

            january = 0,
            february,
            march,
            april,
            may,
            june,
            july,
            august,
            september,
            october,
            november,
            december,
         };
         enum class schedule_weekday : int8_t {
            any = -1,

            sunday = 0,
            monday,
            tuesday,
            wednesday,
            thursday,
            friday,
            saturday,

            all_weekdays,
            all_weekends,
            monday_wednesday_friday,
            tuesday_thursday,
         };

         static constexpr const int8_t any_hour = -1;
         static constexpr const int8_t any_minute = -1;
         static constexpr const int8_t any_day = 0;

      public:
         schedule_month   month    = schedule_month::any;
         schedule_weekday weekday  = schedule_weekday::any;
         uint8_t          day      = any_day;
         int8_t           hour     = any_hour;
         int8_t           minute   = any_minute;
         uint32_t         duration = 0;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
   };
}