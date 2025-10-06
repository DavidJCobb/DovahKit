
# `cpuinfo.h`

A Meyers singleton which runs the `CPUID` assembly instruction, to retrieve information on the current processor's supported instruction sets.

## Implementation notes

I have it on good authority that it's faster to cahce the results of CPUID than to query it every time we need it; the CPUID instruction is relatively expensive, clears multiple registers, and acts as a full memory and execution barrier.

## Alternate options

On \*nix, ifuncs can be used to split a function based on its instruction set, and have the right one linked when the program loads.

On Windows 11 and *maybe* Windows 10 (definitely no older than that), a similar functionality (undocumented as of this writing) exists: the `/funcoverride` CLI option for the linker. I don't yet know how you actually mark function implementations for use with this, but I do know that each implementation of a function has its name mangled differently.
