
# DovahKit XAudio2 integrations

**As of this writing, this code isn't used in DovahKit outside of a simple "laboratory test," and it requires some changes in order to make it fit for broader use.**

The code in this folder allows us to use XAudio2 to play audio, though currently, we only support 2D playback of Skyrim's FUZ files (i.e. dialogue lines). For that specific use case, I should probably switch to using `QMediaPlayer` (part of Qt's Multimedia module, which is not currently enabled for this project), but that's not worth having to rebuild the entire program right now. Additionally, if I can figure out enough about Skyrim's audio programming, using XAudio2 would allow me to try and simulate its various audio effects (e.g. `SOPM` forms) in the distant future.

For now, we also have `DKAudioWidgetSimple` which just consists of a "Play/Pause" button and a "Stop" button, and is capable of loading and playing a FUZ using the classes defined in this folder. See below for remarks on important changes that need to be made in order for these widgets to be fit for purpose.

This is all also my first foray into using XAudio2, so I can't and won't claim that any of what's here is ideal.

Classes in here are as follows:

* `core_interface` maps directly to a single instance of the `IXAudio2` interface i.e. a single audio engine with its own thread and its own voice node graph. You can theoretically create multiple audio cores, and the other classes expect to be given a `std::shared_ptr` for the audio core they should play through.

  * **However, DovahKit should be given a dedicated audio subsystem to store a single `core_interface` to be reused. We should not be creating one of these for every single UI widget that needs or wants to play audio.**

  * **And just to drive the point home -- just to make clear the implications of creating one of these things -- this class should probably be renamed to `dovahkit::xaudio2::engine_and_thread`.**

* `simple_fuz_sound_definition` maps to a loaded FUZ file and its contents.

* `simple_fuz_sound_instance` is, in essence, a sound emitter, which owns its own XAudio2 source voice. The sound instance has a shared pointer to a sound definition, because in order to play a sound, we have to stream sound data into the voice.

  * The reason we separate the "definition" and the "instance" is due to how voices work in XAudio2. You can think of voices as akin to streams which feed data to their destinations in real-time (unless they or their containing audio engine are paused). So a source voice doesn't necessarily correspond to *a sound effect*, but rather to *a place through which you can stream a sound effect*, and you have to feed that sound effect into the stream every time you want to play it.

    That's all why it's accurate to say that `simple_fuz_sound_instance` is not a source voice *per se*, but rather a "sound emitter" which can emit a single sound effect multiple times consecutively, if desired, but not multiple times concurrently (you'd need multiple emitters).

      * So perhaps I should rename these classes to `fuz_sound_definition` and `simple_fuz_sound_emitter`. The word "simple" in their names is meant to reference the fact that no functions are provided for e.g. hooking the source voice up to any submix voices for real-time audio processing.

      * It'd also be worth adding [volume accessors](https://learn.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-volume-and-pitch-control).

* `operation_set_handle` is unused, but could be used as a handle for any APIs that should be run together as a single transaction. (XAudio2 supports this kind of transaction model, with unique integer IDs used to identify transactions.) I misread some XAudio2 documentation (this is the first time I've used the library) and thought certain operations needed to be bundled into transactions, when they don't, so I've ended up not using this.