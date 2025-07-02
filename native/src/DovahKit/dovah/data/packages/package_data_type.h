#pragma once

namespace dovah::packages {
   enum class package_data_type {
      invalid = -1,

      boolean,         // BGSPackageDataBool:           "Bool"
      float32,         // BGSPackageDataFloat:          "Float"
      integer,         // BGSPackageDataInt:            "Int"
      location,        // BGSPackageDataLocation:       "Location"
      object_list,     // BGSPackageDataObjectList:     "ObjectList"
      single_ref,      // BGSPackageDataRef:            "SingleRef"
      target_selector, // BGSPackageDataTargetSelector: "TargetSelector"
      topic,           // BGSPackageDataTopic:          "Topic"
   };
}
