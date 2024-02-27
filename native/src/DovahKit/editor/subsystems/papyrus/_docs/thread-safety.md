
The following operations are intended to support multi-threading, in order to allow us to multi-thread VMAD scanning post-load (see `DovahKitFormDataCache`):

* Looking up a script by name
* Looking up a script by name and, if it doesn't exist, creating a `known_script` and returning a wrapping `known_script_ptr`

No other operations are intended to be thread-safe.