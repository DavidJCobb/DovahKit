
# Headless execution

DovahKit's alpha isn't planned to have any headless execution functionality. However, given that we have Lua script support, it could be useful to add headless loading and script execution during sustain.

Making a program that can run as both a GUI and a console application is, however, not easy. There are two *basic* approaches that I explored while working on ReachVariantTool, and I've since thought of a third.


## Background

Windows allows you to compile programs to run on either the Windows subsystem or the Console subsystem. This influences how the programs themselves behave, and how a console behaves when directed to execute them.

* A Windows-subsystem program is *intended* to have a GUI. If this program is run via a console window, the console won't wait for the program to finish execution; the console window will continue onward (e.g. to the next command, if in a batch file, or to prompting the user for further input). Additionally, the Windows-subsystem program won't inherit console handles (i.e. for input and output) and thus won't open the default C I/O streams; the C-era stream accessors `stdout`, `stderr`, and `stdin` won't have an underlying stream to wrap.

* A Console-subsystem program is *intended* not to have a GUI. If this program is started via a console window, the console will wait for the program to finish executing. Additionally, the Console-subsystem program will inherit console handles, which can be written to via `stdout` and friends. However, if a Console-subsystem program is started via the Windows GUI, it will spawn its own console window automatically.

Of course, a Windows-subsystem program is allowed to attach itself to a console, and a Console-subsystem program is allowed to spawn GUI objects. The distinction lies mainly in the program's relationship to a console used to spawn it. This means that if you have a GUI program with a headless mode, you have two basic options:

* Compile as a Windows-subsystem program, and use tricks to gain access to a console when invoked from one

* Compile as a Console-subsystem program, and use tricks to dismiss the automatically-created console when invoked from outside of a console

### Windows-to-Console

A Windows-subsystem program that is executed from a console window can attach itself to that console window using `AttachConsole(ATTACH_PARENT_PROCESS)`. With some trickery, it can take the empty/null C-era stream accessors and repoint them to that console window's handles. However, this still leads to a substandard experience when running the program via a console, because the following is what happens:

1. You use the console to execute the program.
2. The program begins executing.
3. The console prompts you for your next command.
4. The program begins writing output to the console, between the displayed command prompt and anything you type.

That looks like this:

```text
C:/YourStuff/>MyProgram.exe --options
C:/YourStuff/>THIS IS PROGRAM OUTPUT
THIS IS MORE PROGRAM OUTPUT

and the things you type next end up down here
```

This, frankly, sucks.

If you want to do it, though, here's how:

```c++
freopen("CONOUT$", "w", stdout);
freopen("CONOUT$", "w", stderr);
freopen("CONIN$",  "r", stdin);

// Synchronize C++ streams to C streams (i.e. std::cout to stdout and so on):
std::ios_base::sync_with_stdio(true);
```

[`freopen`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/freopen-wfreopen?view=msvc-170) is a POSIX API that takes a `FILE*` and remaps it, given a file path and mode. It's intended to work on, like, *actual files*, but evidently you *can* pass `stdout` and friends into it. The `CONOUT$` and `CONIN$` strings are filesystem paths that Windows treats as sentinel values; it maps them to the [stream handles for the current console](https://learn.microsoft.com/en-us/windows/console/console-handles).[^dup2-will-not-save-you]

[^dup2-will-not-save-you]: It's very fortunate that the `CONOUT$` and `CONIN$` sentinel paths exist, because I'm not aware of any other way to remap `stdout` and friends to the console handles when the streams don't already exist.
    
    A workaround that some people use to redirect `stdout` and friends is to use the POSIX `dup2` API. The theory is that you can use `_open_osfhandle` to convert the console stream handles[^console-output-handle] into file descriptors, use `fileno` to get the file descriptors for `stdout` and friends, and then use `dup2` to remap the latter file descriptors so that they redirect to the former file descriptors. Raymond Chen has [demonstrated](https://devblogs.microsoft.com/oldnewthing/20180221-00/?p=98065)[^chen] how to use this approach to remap the C I/O streams to a Windows file `HANDLE`.[^chen-doesnt-close] The problem is that for the specific case of a Windows-subsystem application, `stdout` and friends don't map to any stream and so have no valid file descriptors (such that [`fileno` returns -2](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fileno?view=msvc-170)) for you to redirect.

[^console-output-handle]: `GetStdHandle(STD_OUTPUT_HANDLE)`

[^chen]: Since Microsoft likes to completely break all inbound links to their devblogs every few years, here's a more durable citation: "How can I call freopen but open the file with shared access instead of exclusive access?" by Raymond Chen, via The Old New Thing (Feb. 21, 2018).

[^chen-doesnt-close]: In the sample code he gives, Chen calls `_close` on the throwaway file descriptor that he creates from an OS handle. You should avoid doing this for the purposes we discuss here, as `_close` closes both the descriptor and any underlying OS handle.

### Console-to-Windows

A Console-subsystem program that is executed from the Windows GUI (e.g. taskbar, Explorer) will have a console window spawn. It can dismiss that console window immediately by calling `FreeConsole()`; this detaches the program from that console, and since nothing else is attached to the console, the window will de-spawn immediately. However, even the fastest-starting programs will still have the console window be visible for a split-second.

Aside from that annoyance, though, everything works properly.


## A possible third option

If you type a filename in the console, without an extension, the console will search for an executable file or shell script based on a prioritized list of file extensions. The default list is `com`, `exe`, and `bat`, and can be overridden via `%PATHEXT%`. The `com` file extension is seldom used today; it originally indicated a DOS-era executable, but is now basically an antiquated synonym for `exe`.

This means that if a Console-subsystem `DovahKit.com` and a Windows-subsystem `DovahKit.exe` are in the same folder, attempting to execute `DovahKit` via the terminal will run the former executable. The naive approach would be to ship the same executable twice, compiled for different subsystems, but that feels wasteful. I had an interesting alternative idea:

* `DovahKit.exe`, the main program, should be a Windows-subsystem program that makes no attempt to attach itself to its parent process's console. It should, however, accept command-line parameters that direct it to run headlessly.

* `DovahKit.com` should be a Console-subsystem program which invokes `DovahKit.exe` with the aforementioned command-line parameters. When the two programs run in tandem like this, `DovahKit.exe` does all the work, while `DovahKit.com` is basically just a wrapper that ferries messages between `DovahKit.exe` and the console using some form of interprocess communication (not something I've worked with before; it seems like it'll be a fun challenge to learn).

We'd of course want some means for `DovahKit.exe` to know whether it's being invoked headlessly by `DovahKit.com`; we wouldn't want it to get into a situation where it's waiting for console input (e.g. because a Lua script has requested input) that will never arrive; we wouldn't want the program to get stuck running in the background, silently, forever.

