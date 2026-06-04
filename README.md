# DovahKit

A work-in-progress tool for editing Skyrim ESM/ESP files. The name is a pun on "Creation Kit" and "*Dovahkiin*," which in Skyrim's lore means "Dragonborn," the player-character's title, in the Dragon language.

This tool was built using Visual Studio 2026. It dynamically links Qt.

## Build environment

This program was built using Microsoft Visual Studio Community 2026 with the [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2022) plug-in and Qt Designer. The [version of Qt used](https://doc.qt.io/qtvstools/qtvstools-managing-projects.html#managing-qt-versions) was 6.11.1, 64-bit, for MSVC 2022 x64.

Lua is compiled as C++; this means that Lua errors properly unwind the stack (calling destructors within any C-functions we make available to Lua) instead of using setjmp/longjmp.

I built DirectXTex using the `DirectXTex_Desktop_2019` project, suitable for programs meant to support Windows 7 SP1 or newer that are built with Visual Studio 2019. Development started with Visual Studio 2019; when I updated to 2022, I upgraded the project as well. A guide to DirectXTex project files and Visual Studio setup can be found [on its own repo](https://github.com/Microsoft/DirectXTex/wiki/DirectXTex#adding-to-a-vs-solution). General setup requirements are also on that page.

This program uses C++23 via the `/std:c++latest` compiler option. The included project files should take care of that for you.

For further information on contributing to DovahKit, refer to `HOW TO CONTRIBUTE.md`.

## License

DovahKit's code is licensed under the GNU General Public License version 3 (GPLv3) unless otherwise stated (some files are licensed under CC0 i.e. public domain as indicated by code comments, etc.).

The content of DovahKit's help manual, if one exists, is licensed under the Creative Commons 0 License (CC0). This includes the content of the documentation in both a raw form, a processed and publication-ready form, and any intermediate forms (e.g. page layout files with placeholders where content should be inserted). This also includes any assets meant to be included in the help manual (e.g. images, CSS, JavaScript). This does not include the code for any system which processes raw documentation into a publication-ready form (e.g. code for a program which might take "raw" documentation in a format such as XML, and stitch files and code together to produce "friendly" HTML-formatted documentation). This also does not include any external assets linked or embedded in the documentation (e.g. YouTube videos displayed in an embedded video player).

This program and its source code contain text content that has been data-mined from *The Elder Scrolls V: Skyrim*, including the executable-level default values of the game's INI settings and "game setting" forms. Files containing such content have been marked with an appropriate notice. The text content in question is the intellectual property of Bethesda Game Studios and is not made available under any DovahKit-specific licenses. DovahKit's author is operating under the good-faith belief that DovahKit's inclusion of this data-mined text falls under fair use &mdash; that DovahKit as a whole is sufficiently transformative.

[DirectXTex](https://github.com/microsoft/DirectXTex), Lua, the [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator), and [miniz](https://github.com/richgel999/miniz) are MIT-licensed. LZ4's license is in the `LICENSE` file in the LZ4 folder. zlib has a custom license in `zlib.h`.

The [OpenGL Mathematics Library (GLM)](https://github.com/g-truc/glm) uses a slightly modified variant of the MIT license, listed in the `copying.txt` file in its folder.

Qt and its components are licensed under LGPL v3. Per the terms of LGPL, I am required to make [the source code for Qt 5.15.2](https://download.qt.io/archive/qt/5.15/5.15.2/single/) available to you in case you're unable to acquire it on your own. Presently I have a copy saved to my machine.

The Qt Visual Studio Tools plug-in is [GPL-licensed with a special exemption](https://marketplace.visualstudio.com/items/TheQtCompany.QtVisualStudioTools2019/license) which allows its use in developing non-GPL software. Qt Designer has [a similar exception](https://opensource.stackexchange.com/questions/7709/using-qt-designer-to-create-ui-design-for-closed-source-application).