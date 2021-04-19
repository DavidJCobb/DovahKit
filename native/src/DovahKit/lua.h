#pragma once

//
// Include all Lua headers. Ordinarily, we could use the standard lua.hpp file, except that 
// that file wraps the headers in an extern C block. This causes problems if we want to 
// compile Lua as C++, which in fact we do.
// 
// There's no way for DovahKit to decide how to include the headers automatically based on 
// how an entirely separate project in the same solution was compiled, so we just have to 
// manually add or remove the extern C block as appropriate. I'd rather not modify the Lua 
// library files directly (too easy to lose track of that), so we'll just do the includes 
// here.
//

#include "../Lua/lua.h"
#include "../Lua/lualib.h"
#include "../Lua/lauxlib.h"