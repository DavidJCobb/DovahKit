
Worldedit tools typically have a static `invoke` function, which is responsible for executing the tool's actual function. These tools are listed in the `all_tools_by_execution_order` class list.

There may be cases in the future wherein it is more efficient to execute a set of tools in tandem, i.e. where Worldedit (or its underlying systems, e.g. the 3D renderer) offer some API and each tool supplies a single parameter to that API. A system exists to accommodate those cases.

Suppose we have two tools, `example_tool_a` and `example_tool_b`, which both supply a single parameter; these parameters can be supplied together to Worldedit or its underlying systems via a single method call, in order to perform some calculation in tandem.

Create a file `example_tandem_tool.h` in the `worldedit/tool_system/tools/invoke_in_tandem/` folder. The header should look like this:

```cpp
#pragma once
#include "./_base.h"
#include "../example_tool_a.h"
#include "../example_tool_b.h"

namespace dovahkit::subsystems::worldedit::tools::tandem {
   class example_tandem_tool : public invoke_in_tandem<example_tool_a, example_tool_b> {
      public:
         static void _invoke_impl(const example_tool_a::response*, const example_tool_b::response*);
   };
}
```

The implementation for `_invoke_impl` can be located in a `*.cpp` file and should check if either response argument is `nullptr`. The handler will be called if any or all of the tools in question are activated by the user, so there will be cases where only one of the two pointers is supplied.

The system is metaprogrammed such that an invocation-in-tandem can use any number of tools: two, three, four, or more. Each tool should be specified as a template parameter, and `_invoke_impl` should receive one argument for each tool, in the same order as the template parameters: for a given tool `T`, the argument type is `const T::response*`.

Once your invoke-in-tandem class is set up, add it to `all_tools_by_execution_order`. *Don't* list the tools it's invoking in there.

# Why was this even made?

Initially, Worldedit had two tools for camera control: `move_camera` and `turn_camera`. These tools supported only camera-relative transformations (no other reference frames). Any change to the camera's coordinates requires recomputing several transformation matrices within the 3D renderer, so these tools were built to be invoked in tandem: the 3D renderer offered an API to "adjust" the camera with movement and/or turning, and Worldedit wrapped this API for it to be invoked in tandem whenever either or both of the `move_camera` and `turn_camera` tools were activated.

Later on, `move_camera` had support for arbitrary reference frames added, and an additional camera tool (`orbit_camera`) was added as well. The former change meant that `move_camera` now had to convert its operations to the world reference frame, which in turn meant it now had to act throuh a separate API from `turn_camera`. As such, it no longer makes sense to have these two (now three) tools invoked in tandem.