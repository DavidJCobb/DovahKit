
# Workflow

I should switch to the following workflow:

* Use Git worktrees so I can have multiple branches ready for work on my computer. That'll allow me to maintain DovahKit's alpha while working on Sustain. Work directly out of the repo directories, adding new `.gitignore` files to avoid having VS commit IntelliSense databases, etc..

* Change the build paths of all projects. The defaults are a mess and I'd prefer to have it all go someplace cleaner.

  * Build intermediate files (e.g. `.obj`) go to `$(IntDir)` i.e. the "Intermediate Directory" property under "General" project properties. Relative paths are relative to `$(ProjectDir)` and the default is `$(Platform)\$(Configuration)\`. I'd like to change this to `$(SolutionDir)_intermediate\$(ProjectName)\$(Platform)\$(Configuration)\`.
  
  * Build output files (e.g. `.exe`, `.dll`, `.pdb`) is all dumped into the "Output Directory" listed under "General" project properties. This defaults to `$(SolutionDir)$(Platform)\$(Configuration)\`. This means that when multiple projects produce executables/DLLs (e.g. `DovahKit.exe` and `DovahKitQtCustomWidgets.dll`), they all get dumped into the same subfolders of the solution directory. I'd like to change this to `$(SolutionDir)_output\$(ProjectName)\$(Platform)\$(Configuration)\`.

* Consider modifying the project to [use a modified Debug configuration that allows `__forceinline` and friends to work](https://stackoverflow.com/a/79408172). Alternatively, turn on full optimizations alongside [Dynamic Debugging](https://devblogs.microsoft.com/cppblog/cpp-dynamic-debugging-full-debuggability-for-optimized-builds/).
