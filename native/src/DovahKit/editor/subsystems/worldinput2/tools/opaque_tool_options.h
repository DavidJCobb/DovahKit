#pragma once
#include <QPointF>

namespace dovahkit::subsystems::worldinput {
   //
   // TODO: Once we implement tools, we'll have this use a `class_array` of 
   //       forward-declarations for the tool classes. That will allow us 
   //       to know how many tools exist, which in turn will let us generate 
   //       the tag type for a tagged union; this class will store only the 
   //       tag and some templated cast operators.
   // 
   //       A subclass will include the actual classes and store the union 
   //       content. The base class will be passed exclusively by reference 
   //       (to prevent object slicing) with receivers being expected to 
   //       reference-cast it to the subclass with the actual data. This 
   //       will ensure that systems that don't actually care about the 
   //       tools (but still need to pass options around) don't have to 
   //       include all of their headers.
   // 
   //       Each bind list item would store a pointer to an instance of the 
   //       subclass, but with the pointer typed to the base class.
   // 
   //       For now, the content of this class consists of whatever we need 
   //       for testing the input system itself.
   //
   class opaque_tool_options {
      public:
         virtual ~opaque_tool_options() = default;
   };
}