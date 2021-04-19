# DovahKit

A work-in-progress tool for editing Skyrim ESM/ESP files. The name is a pun on "Creation Kit" and "*Dovahkiin*," which means "Dragonborn" in the Dragon language.

This tool was built using Visual Studio 2019. It dynamically links Qt, which was licensed under LGPLv3. Zlib and LZ4 are used as well; see zlib.h for its license terms, and refer to the LICENSE file in the LZ4 directory for its license terms.

## Build environment

This program was built using Microsoft Visual Studio Community 2019 with the [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) plug-in. That plug-in is [GPL-licensed with a special exemption](https://marketplace.visualstudio.com/items/TheQtCompany.QtVisualStudioTools2019/license) which allows its use in developing non-GPL software. Qt Designer was also used to build this program, with [a similar exception](https://opensource.stackexchange.com/questions/7709/using-qt-designer-to-create-ui-design-for-closed-source-application). The [version of Qt used](https://doc.qt.io/qtvstools/qtvstools-managing-projects.html#managing-qt-versions) was 5.15.2, 64-bit, for MSVC 2019 x64.

Lua is compiled as C++; this means that Lua errors properly unwind the stack (calling destructors within any C-functions we make available to Lua) instead of using setjmp/longjmp.

This program uses C++20 via the `/std:c++latest` compiler option. The included project files should take care of that for you.

## License

Qt and its components have their own license; as do zlib, LZ4, and Lua. My code is licensed under [Creative Commons CC-BY-NC 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/) unless otherwise stated (some files are licensed under CC0 i.e. public domain, etc.).

Per the terms of LGPL, I am required to make [the source code for Qt 5.15.2](https://download.qt.io/archive/qt/5.15/5.15.2/single/) available to you in case you're unable to acquire it on your own. Presently I have a copy saved to my machine.