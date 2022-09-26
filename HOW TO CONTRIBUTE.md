# How to contribute

In order to contribute to DovahKit, you'll need to set up your build environment:

* Microsoft Visual Studio 2022 Community
* [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2022) plug-in
* Qt version 5.15.2, 64-bit, for MSVC 2019 x64.

  [You'll need to set that up in the Qt VS plug-in.](https://doc.qt.io/qtvstools/qtvstools-managing-projects.html#managing-qt-versions) When setting it up, you'll need to select names for the Qt versions; the names used for DovahKit are `5.15.2 MSVC2019 x64`.
* The [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows).
  
  DovahKit's project files are configured to pull this from `C:\VulkanSDK\1.2.189.2`.

Anyone wishing to contribute their changes to the main DovahKit project must also agree to [the contributor license agreement](https://gist.github.com/DavidJCobb/2023bca5e5d9c30ce1afa020e0424ae6) before contributing. This license agreement serves three purposes:

* It permits this project to use your code contributions under GPLv3.

* It permits this project to use your contributions to the content of the help manual (e.g. text and images) under CC0.

* You promise that you're legally allowed to share your contributions with this project (i.e. it's your content or you have the rights to it; your employer doesn't own it as part of your contract with them; et cetera).

GitHub has an equivalent clause in their Terms of Service (when contributing to a repo, you agree to license your contribution under the same license(s) as the repo), but I would prefer to make this explicit, rather than relying on a couple paragraphs buried in a Terms of Service agreement that a giant corporation can change at any time without warning.

The help manual (when we create one...) is licensed under CC0 so that people can share excerpts, etc., without having to bundle any raw source files or program code used to create and publish the help manual. That is: if we design the help manual so that it is initially written in a raw format, with a program used to generate human-readable HTML, users should not have to bundle that program and its source in order to share the help manual or excerpts from the manual.

Other contributions are under GPLv3, which stipulates that:

* Users can redistribute the program without having to ask for permission.
* Users can create their own forks of the program without having to ask for permission.
* People distributing the program (or a fork) must include the license information, so that users know what rights they have.
* People distributing the program (or a fork) must ensure that others can access the source code, so that users can exercise those rights.

It's my understanding that any original content (i.e. content wholly created or owned by you) that you license to this project under GPLv3 remains yours. People can take that content from this repo and use it under the terms of GPLv3, but you can still share that content with other parties, or use it in your own works, under any license you wish, without any interference from this project or its maintainers.


## Tips

In order to build a copy of DovahKit suitable for redistribution, with all Qt DLLs in place, you must:

1. Access the project settings in Visual Studio.

2. Go to the Qt Project Settings.

3. Set "Run Deployment Tool" to "Yes." This will run `windeployqt` during the build step, which will copy Qt-related dependencies into the same folder as the compiled EXE. Note that it may not copy the VC Redistributable package.


### Working with the Dovahscript engine

The Dovahscript engine allows users to run Lua scripts on a worker thread, with the engine handling cross-thread communication and allowing scripts to control UI widgets and similar. Multithreaded systems like these can be tricky to write and maintain. When making changes to the script engine, please test in both the Debug and Release configurations. Race conditions and other threading mishaps can be sensitive to execution speeds (relative and absolute), so testing in only one configuration may cause you to miss potential problems (be they of your making or mine) and contribute broken code.