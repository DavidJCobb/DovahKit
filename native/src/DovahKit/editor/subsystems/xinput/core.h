#pragma once
#include "./xinputDK.h"

//
// Qt's code generation system -- specifically, the MOC -- doesn't understand what 
// folders are. It'll dump the generated code files in the same folder regardless 
// of where in the folder tree the original source files were. This means that you 
// cannot have two Qt-relevant source files with the same name, even if they're in 
// different folders.
// 
// It's basically the same problem that MSVC's default linker settings have, but 
// Qt offers no configuration options that can solve it.
// 
// Ergo: each subsystem needs to be defined in a file named after the subsystem, 
// to ensure (or at least aid) unique filenames. For consistency, however, each 
// subsystem folder should have a file called `core.h` which just includes the 
// subsystem header.
//